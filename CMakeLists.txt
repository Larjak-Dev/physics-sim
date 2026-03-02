cmake_minimum_required(VERSION 3.20) # Required for modern features

# 1. Project Setup
project(MyUltimateProject
    VERSION 1.0.0
    LANGUAGES CXX
)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Recursive Source Discovery
# Note: While GLOB is often debated, it is the "ultimate" for rapid prototyping.
# CONFIGURE_DEPENDS ensures CMake reruns if you add/remove files.
file(GLOB_RECURSE PROJECT_SOURCES
    CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.hpp"
    "src/*.c"
    "src/*.h"
)

# 3. Create Executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# 4. Include Directories
# This tells CMake to look in 'include' and 'src' for headers
target_include_directories(${PROJECT_NAME} PUBLIC
    include
    src
)

# 5. Resource Copying Logic
# This function creates a symlink or copies the folder into the build directory
# so your paths like "resources/image.png" work during debugging.
set(RES_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/resources")

if(EXISTS "${RES_FOLDER}")
  message(STATUS "Resources found. Setting up sync to build directory.")

  # This command runs every time you build, ensuring assets stay updated
  add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${RES_FOLDER}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
        COMMENT "Syncing resources to build directory..."
    )
endif()

# 6. Compiler Warnings (The "Pro" Setup)
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# 7. Optimization & Output Locations
set_target_properties(${PROJECT_NAME} PROPERTIES
    DEBUG_POSTFIX "d"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
