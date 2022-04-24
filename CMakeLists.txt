cmake_minimum_required(VERSION 3.7)

project(terrain_tools)

include(FetchContent)
FetchContent_Declare(
    tools
    GIT_REPOSITORY        https://github.com/sol-prog/cpp-bmp-images.git
    GIT_TAG               origin/master
)

FetchContent_GetProperties(tools)
if (NOT tools_POPULATED)
    FetchContent_Populate(tools)
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set C++11 for all projects and disable non-standard extensions
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_PREFIX .)

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        Set(CUVID_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/nvcuvid.lib)
        set(NVENCODEAPI_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/nvencodeapi.lib)
    else()
        Set(CUVID_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/Win32/nvcuvid.lib)
        set(NVENCODEAPI_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/Win32/nvencodeapi.lib)
    endif()
else ()
    find_library(CUVID_LIB nvcuvid)
    find_library(NVENCODEAPI_LIB nvidia-encode)
endif()

set(NV_ENC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nvenc)
set(NVCODEC_PUBLIC_INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/interface)
set(APP_COMMON_ARGS ${CMAKE_CURRENT_SOURCE_DIR}/common_args)

add_subdirectory(terrain_converter)
add_subdirectory(common_args)