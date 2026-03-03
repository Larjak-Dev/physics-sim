cmake_minimum_required(VERSION 3.24) # Upgraded to 3.24 for better FetchContent handling

# 1. Project Setup
project(PhysicsSim
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Fetch libraries
include(FetchContent)

#DECLARE

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        3.0.0
)
set(SFML_BUILD_NETWORK FALSE)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.1-docking
)

FetchContent_Declare(
    imgui-sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG        v3.0
)

#FETCH
message(STATUS "Fetching glm")
FetchContent_MakeAvailable(glm)
message(STATUS "Fetching imgui...")
FetchContent_MakeAvailable(imgui)
message(STATUS "Fetching SFML...")
FetchContent_MakeAvailable(SFML)

set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_SFML_IMGUI_DEMO ON)

message(STATUS "Fetching imgui-sfml...")
FetchContent_MakeAvailable(imgui-sfml)


# 2. Recursive Source Discovery
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
target_include_directories(${PROJECT_NAME} PUBLIC
    include
    src
)

# --- NEW: Link SFML & ImGui ---
target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::Graphics
    ImGui-SFML::ImGui-SFML
    glm::glm
)

# 5. Resource Copying Logic
set(RES_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/resources")
if(EXISTS "${RES_FOLDER}")
    message(STATUS "Resources found. Setting up sync to build directory.")
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${RES_FOLDER}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
        COMMENT "Syncing resources to build directory..."
    )
endif()

# 6. Compiler Warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4) # Removed /WX (treat warnings as errors) temporarily to avoid ImGui headers triggering breaks
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 7. Optimization & Output Locations
set_target_properties(${PROJECT_NAME} PROPERTIES
    DEBUG_POSTFIX "d"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
