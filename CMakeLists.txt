cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME mic_3)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/Source/TransformFunctions/arm_rfft_fast_f32.c"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/Source/TransformFunctions/arm_rfft_fast_init_f32.c"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/Source/TransformFunctions/arm_cfft_f32.c"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/Source/TransformFunctions/arm_cfft_radix8_f32.c"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/Source/ComplexMathFunctions/arm_cmplx_mag_f32.c"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/Source/StatisticsFunctions/arm_max_f32.c"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/Source/CommonTables/arm_common_tables.c"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/Source/CommonTables/arm_const_structs.c"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/Source/TransformFunctions/arm_bitreversal.c"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/Source/TransformFunctions/arm_bitreversal2.S"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/Source/TransformFunctions/arm_cfft_init_f32.c"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/Source/BasicMathFunctions/arm_dot_prod_f32.c"
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/USB_DEVICE/App    # if headers live here
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/PrivateInclude
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS_DSP/Source
    
    
)

# Add linker flags to enable float printing
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -u _printf_float
)
