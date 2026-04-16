cmake_minimum_required(VERSION 3.5)
project(physics_engine LANGUAGES CXX)


file(GLOB_RECURSE PROJECT_SOURCES
    CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.hpp"
    "src/*.c"
    "src/*.h"
)

add_library(physics_engine STATIC
    ${PROJECT_SOURCES}
)

target_include_directories(physics_engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/physics_engine>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        src
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glm::glm-header-only
    #OpenXLSX::OpenXLSX
)

set_target_properties(physics_engine PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
