project(tiny_gpu_dec)

set(APP_SOURCES
 ${CMAKE_CURRENT_SOURCE_DIR}/tiny_gpu_dec.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/dct_c.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/sad_c.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/intra_c.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/encode_c.cpp
 ${APP_COMMON_ARGS}/sample_args.cpp
)

set(APP_CUDA_KERNELS
 ${CMAKE_CURRENT_SOURCE_DIR}/tiny_gpu_dec_kernel.cu
)

source_group( "sources" FILES ${APP_SOURCES} )

find_package(CUDA)

set(CUDA_HOST_COMPILER ${CMAKE_CXX_COMPILER})

set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode -fatbin arch=compute_50,code=\"sm_50,compute_50\")
if ( CMAKE_COMPILER_IS_GNUCC )
    if(NOT "${CUDA_NVCC_FLAGS}" MATCHES "-std=c\\+\\+11" )
        list(APPEND CUDA_NVCC_FLAGS -std=c++11)
    endif()
endif()

cuda_add_executable(${PROJECT_NAME}  ${APP_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

target_include_directories(${PROJECT_NAME} PUBLIC 
 ${CUDA_INCLUDE_DIRS}
 ${APP_COMMON_ARGS}
 ${APP_CUDA_KERNELS}
 ${tools_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} ${CUDA_CUDA_LIBRARY} ${CMAKE_DL_LIBS})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${NVCODEC_SAMPLES_INSTALL_DIR})
if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set_target_properties( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/ )
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ${CMAKE_BINARY_DIR} CONFIGURATIONS Debug)
endif()