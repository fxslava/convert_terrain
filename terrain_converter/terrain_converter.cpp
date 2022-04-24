#include <cuda.h>
#include <vector>
#include <fstream>
#include <iostream>
#include "BMP.h"
#include "NvEncoderCuda.h"
#include "sample_args.h"

inline bool check(int e, int iLine, const char* szFile) {
    if (e < 0) {
        std::cout << "General error " << e << " at line " << iLine << " in file " << szFile;
        return false;
    }
    return true;
}

#define ck(call) check(call, __LINE__, __FILE__)

bool InitCUDA(int iGpu, CUcontext& cuContext, CUdevice& cuDevice)
{
    ck(cuInit(0));
    int nGpu = 0;
    ck(cuDeviceGetCount(&nGpu));
    if (iGpu < 0 || iGpu >= nGpu)
    {
        std::cout << "GPU ordinal out of range. Should be within [" << 0 << ", " << nGpu - 1 << "]" << std::endl;
        return false;
    }

    ck(cuDeviceGet(&cuDevice, iGpu));
    char szDeviceName[80];
    ck(cuDeviceGetName(szDeviceName, sizeof(szDeviceName), cuDevice));
    std::cout << "GPU in use: " << szDeviceName << std::endl;
    ck(cuCtxCreate(&cuContext, 0, cuDevice));
    return true;
}

void CvtRGB24bitToRGBA32bit(uint32_t* dst, std::vector<uint8_t> src, const int width, const int height, const int pitch_dwords, const int bytecount)
{
#pragma omp parallel for collapse(2)
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            uint8_t r = src[(x + y * width) * bytecount];
            uint8_t g = src[(x + y * width) * bytecount + 1];
            uint8_t b = src[(x + y * width) * bytecount + 2];
            const uint32_t rgba = static_cast<uint32_t>(r) + (static_cast<uint32_t>(g) << 8U) + (static_cast<uint32_t>(b) << 16U) + (static_cast<uint32_t>(UINT8_MAX) << 24U);
            dst[x + y * pitch_dwords] = rgba;
        }
    }
}

int main(int argc, char** argv)
{
    int texWidth = -1;
    int texHeight = -1;
    int tileRowsNum = -1;
    int tileColsNum = -1;
    int iGpu = -1;
    bool perTile = false;
    std::string* tileName = nullptr;
    std::string* outputStream = nullptr;

    std::vector<arg_desc_t> args = {
        {"-w",      "Tile width",      ARG_TYPE_INT,  &texWidth},
        {"-h",      "Tile height",     ARG_TYPE_INT,  &texHeight},
        {"-gpu",    "GPU order",       ARG_TYPE_INT,  &iGpu},
        {"-name",   "Terrain name",    ARG_TYPE_TEXT, &tileName},
        {"-stream", "Output stream",   ARG_TYPE_TEXT, &outputStream},
        {"-tx",     "Tile rows num",   ARG_TYPE_INT,  &tileRowsNum},
        {"-ty",     "Tile cols num",   ARG_TYPE_INT,  &tileColsNum},
        {"-pertile","Per tile stream", ARG_TYPE_NONE, &perTile},
    };
    args_parser args_parser(args, argc, argv);

    const int texPitch = ((texWidth + 64 - 1) & ~63) * sizeof(uint32_t); // 256 byte aligning
    const int frameBufferSize = texHeight * texPitch + 64;

    // Allocate and make as page locked memory for tile frame
    uint8_t* frameBufferUnaligned = new uint8_t[frameBufferSize];
    uint32_t* frameBuffer = reinterpret_cast<uint32_t*>(reinterpret_cast<ptrdiff_t>(frameBufferUnaligned) & ~0xff);
    ck(cuMemHostRegister(frameBuffer, frameBufferSize, 0));

    // Convert tiles loop
    CUcontext cuContext = nullptr;
    CUdevice cuDevice = 0;
    if (InitCUDA(iGpu, cuContext, cuDevice) && tileName) {

        NvEncoderCuda enc(cuContext, texWidth, texHeight, NV_ENC_BUFFER_FORMAT_ARGB, 1);

        NV_ENC_INITIALIZE_PARAMS initializeParams = { NV_ENC_INITIALIZE_PARAMS_VER };
        NV_ENC_CONFIG encodeConfig = { NV_ENC_CONFIG_VER };
        initializeParams.encodeConfig = &encodeConfig;

        enc.CreateDefaultEncoderParams(&initializeParams, NV_ENC_CODEC_HEVC_GUID, NV_ENC_PRESET_P7_GUID, NV_ENC_TUNING_INFO_HIGH_QUALITY);

        encodeConfig.frameIntervalP = 0; // I ONLY;
        encodeConfig.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CONSTQP;
        encodeConfig.rcParams.constQP.qpIntra = 31;
        encodeConfig.rcParams.lookaheadDepth = 0;
        encodeConfig.encodeCodecConfig.hevcConfig.chromaFormatIDC = 3;
        encodeConfig.encodeCodecConfig.hevcConfig.minCUSize = NV_ENC_HEVC_CUSIZE_32x32;
        encodeConfig.encodeCodecConfig.hevcConfig.maxCUSize = NV_ENC_HEVC_CUSIZE_32x32;

        enc.CreateEncoder(&initializeParams);

        std::vector<uint8_t> compressedTiles;
        char fullTileName[256];
        char streamTileName[256];
        for (int tx = 0; tx < tileRowsNum; tx++) {
            for (int ty = 0; ty < tileColsNum; ty++) {
            	sprintf_s(fullTileName, 256, "%s_x%d_y%d.bmp", tileName->c_str(), tx, ty);
                printf("Processing: tile x%d, y%d : output %s\n", tx, ty, fullTileName);

                BMP tile(fullTileName);

                CvtRGB24bitToRGBA32bit(frameBuffer, tile.data, texWidth, texHeight, texPitch / sizeof(uint32_t), (tile.bmp_info_header.bit_count / 8));

                const NvEncInputFrame* encoderInputFrame = enc.GetNextInputFrame();

                NvEncoderCuda::CopyToDeviceFrame(cuContext, frameBuffer, texPitch, (CUdeviceptr)encoderInputFrame->inputPtr,
                    (int)encoderInputFrame->pitch, enc.GetEncodeWidth(), enc.GetEncodeHeight(), CU_MEMORYTYPE_HOST,
                    encoderInputFrame->bufferFormat,
                    encoderInputFrame->chromaOffsets,
                    encoderInputFrame->numChromaPlanes);

                std::vector<std::vector<uint8_t>> vPacket;
                if (tx == tileRowsNum - 1 && ty == tileColsNum - 1) {
                    enc.EndEncode(vPacket);
                } else {
                    enc.EncodeFrame(vPacket);
                }

                if (perTile)
                {
                    sprintf_s(streamTileName, 256, "%s_x%d_y%d.hevc", outputStream->c_str(), tx, ty);

                    std::ofstream fs(streamTileName, std::ios::out | std::ios::binary);
                    for (std::vector<uint8_t>& packet : vPacket) {
                        fs.write(reinterpret_cast<const char*>(packet.data()), packet.size());
                    }
                    fs.close();
                }
                else {
                    for (std::vector<uint8_t>& packet : vPacket) {
                        compressedTiles.insert(compressedTiles.end(), packet.data(), packet.data() + (int)packet.size());
                    }
                }
            }
        }

        if (outputStream && compressedTiles.size() != 0 && !perTile) {
            std::ofstream fs(outputStream->c_str(), std::ios::out | std::ios::binary);
            fs.write(reinterpret_cast<const char*>(&compressedTiles[0]), compressedTiles.size());
            fs.close();
        }
    }
    return 0;
}