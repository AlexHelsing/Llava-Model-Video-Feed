cmake_minimum_required(VERSION 3.15)
project(LlavaWebCam)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Find OpenCV4
find_package(OpenCV 4 REQUIRED)
find_package(CURL REQUIRED)


# Print status for debugging
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
message(STATUS "OpenCV_VERSION: ${OpenCV_VERSION}")

# Create executable - update path to match your structure
add_executable(LlavaWebCam 
    LlavaWebCam/LlavaWebCam.cpp
    LlavaWebCam/LlavaWebCam.h
)

# Link libraries
target_link_libraries(LlavaWebCam PRIVATE 
    ${OpenCV_LIBS}
    CURL::libcurl
)

# Include directories
target_include_directories(LlavaWebCam PRIVATE 
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/LlavaWebCam
)

# Set output directories
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)

    # Copy OpenCV DLLs
    file(GLOB OPENCV_DLLS "C:/vcpkg/installed/x64-windows/bin/opencv_*.dll")
    foreach(DLL ${OPENCV_DLLS})
        add_custom_command(TARGET LlavaWebCam POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${DLL}
            $<TARGET_FILE_DIR:LlavaWebCam>)
    endforeach()
endif()