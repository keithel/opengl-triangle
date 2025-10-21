cmake_minimum_required(VERSION 3.11)

project(OpenGLTriangle)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4 # Use a specific version tag for reproducibility
)
FetchContent_MakeAvailable(glfw)

set(OpenGL_GL_PREFERENCE "GLVND")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
#find_package(GLFW REQUIRED)

add_executable(opengl_app main.cpp)
target_link_libraries(opengl_app PRIVATE OpenGL::GL GLEW::GLEW glfw )

find_package(Threads REQUIRED)
target_link_libraries(opengl_app PRIVATE Threads::Threads)
