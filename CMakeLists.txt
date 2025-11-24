cmake_minimum_required(VERSION 3.15)
project(RaspberryPi5_Inchrosil_RTOS VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Raspberry Pi 5 specific optimizations
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8.2-a -mtune=cortex-a76")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra")

# Enable threading
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Inchrosil/include
)

# Inchrosil source files
set(INCHROSIL_SOURCES
    Inchrosil/src/nucleotide.cpp
    Inchrosil/src/rtos_scheduler.cpp
    Inchrosil/src/rtos_memory_pool.cpp
)

# Create library for Inchrosil RTOS components
add_library(inchrosil_rtos STATIC ${INCHROSIL_SOURCES})
target_include_directories(inchrosil_rtos PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Inchrosil/include
)
target_link_libraries(inchrosil_rtos PUBLIC Threads::Threads)

# Main executable - Raspberry Pi 5 RTOS Example
add_executable(rpi5_dna_rtos rpi5_inchrosil_rtos_example.cpp)
target_link_libraries(rpi5_dna_rtos PRIVATE inchrosil_rtos Threads::Threads)

# Installation
install(TARGETS rpi5_dna_rtos DESTINATION bin)

# Display build information
message(STATUS "================================================")
message(STATUS "Raspberry Pi 5 Inchrosil RTOS Example")
message(STATUS "================================================")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "================================================")
