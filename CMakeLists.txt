set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

cmake_minimum_required(VERSION 3.24) # Upgraded to 3.24 for better FetchContent handling

# 1. Project Setup
project(PhysicsSim
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#STATICALLY LINKED
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# For MSVC: Ensure the C/C++ Runtime is also linked statically (/MT instead of /MD)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

#Fetch libraries
include(FetchContent)

#DECLARE

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
)

FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.14.2
)

FetchContent_Declare(
    OpenXLSX
    GIT_REPOSITORY https://github.com/troldal/OpenXLSX.git
    GIT_TAG       5723411d47643ce3b5b9994064c26ca8cd841f13
)

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
#json
message(STATUS "Fetching json")
set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(json)
#cpr
message(STATUS "Fetching cpr")
set(BUILD_CPR_TESTS OFF CACHE INTERNAL "")
set(CPR_CURL_USE_LIBPSL OFF CACHE INTERNAL "")
set(CURL_USE_LIBPSL OFF CACHE INTERNAL "") # Explicitly for curl
FetchContent_MakeAvailable(cpr)
#OpenXLSX
message(STATUS "Fetching OpenXLSX")
set(OPENXLSX_BUILD_TESTS OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(OpenXLSX)
#glm
message(STATUS "Fetching glm")
set(GLM_BUILD_TESTS OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(glm)
#imgui
message(STATUS "Fetching imgui...")
set(IMGUI_USE_DARK_THEME ON CACHE BOOL "Use Dark Spectrum Theme" FORCE)
FetchContent_MakeAvailable(imgui)
#SFML
message(STATUS "Fetching SFML...")
set(SFML_BUILD_TEST_SUITE OFF CACHE INTERNAL "")
set(SFML_BUILD_NETWORK FALSE CACHE INTERNAL "")
FetchContent_MakeAvailable(SFML)
#imgui-sfml
set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_SFML_IMGUI_DEMO ON)
message(STATUS "Fetching imgui-sfml...")
FetchContent_MakeAvailable(imgui-sfml)

add_library(glad_loader STATIC external/glad/src/glad.c)
target_include_directories(glad_loader PUBLIC
    "external/glad/include")


# 2. Recursive Source Discovery
file(GLOB_RECURSE PROJECT_SOURCES
    CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.hpp"
    "src/*.c"
    "src/*.h"
)

# 3. Create Executable
if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES})
else()
    add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
endif()

# 4. Include Directories
target_include_directories(${PROJECT_NAME} PUBLIC
    include
    src
    external
    external/glad/include
)

find_package(OpenGL REQUIRED)

# --- Link
target_link_libraries(${PROJECT_NAME} PRIVATE
    nlohmann_json::nlohmann_json
    cpr::cpr
    OpenGL::GL
    SFML::Graphics
    ImGui-SFML::ImGui-SFML
    glm::glm-header-only
    OpenXLSX::OpenXLSX
    glad_loader
)

if(TARGET SFML::Main)
    target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Main)
endif()

# 5. Resource Copying Logic
set(RES_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/assets")
if(EXISTS "${RES_FOLDER}")
    message(STATUS "Resources found. Setting up sync to build directory.")
    add_custom_target(sync_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${RES_FOLDER}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMENT "Syncing resources to build directory..."
        VERBATIM
    )
    add_dependencies(${PROJECT_NAME} sync_assets)
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

# 8. Create Excel Output Directory
add_custom_target(create_excel_output ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Excel_Output"
    COMMENT "Creating Excel_Output directory..."
    VERBATIM
)
add_dependencies(${PROJECT_NAME} create_excel_output)


if(WIN32)
    target_link_libraries(PhysicsSim PRIVATE user32)
endif()
