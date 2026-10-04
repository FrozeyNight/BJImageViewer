cmake_minimum_required(VERSION 3.16)
project(BJImageViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
add_compile_options("-g")

add_subdirectory(Dependencies/glfw EXCLUDE_FROM_ALL)
add_subdirectory(Dependencies/glew/build/cmake EXCLUDE_FROM_ALL)

file(GLOB BJIMAGEVIEWER_SOURCES CONFIGURE_DEPENDS 
src/*.cpp 
Dependencies/imgui/*.cpp
Dependencies/imgui/backends/imgui_impl_glfw.cpp
Dependencies/imgui/backends/imgui_impl_opengl3.cpp
Dependencies/stb_image/*.cpp

)

find_package(OpenGL REQUIRED)

add_executable(BJImageViewer ${BJIMAGEVIEWER_SOURCES})

#target_compile_definitions(BJImageViewer PRIVATE GLEW_STATIC)

target_include_directories(BJImageViewer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/glfw/ # should be with /include
    ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/glm/
    ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/imgui/
    ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/imgui/backends/
    ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/stb_image/
    ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/glew/include
)

target_link_libraries(BJImageViewer PRIVATE
    glfw
    OpenGL::GL
    glew
)