# Specify the minimum version of CMake required to build this project.
cmake_minimum_required(VERSION 3.11)

# Set the project name.
project(OpenGLTriangle)

# Set the C++ standard to C++11.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Check if the target system is VxWorks. The VxWorks toolchain file
# (e.g., vxworks.cmake) should set CMAKE_SYSTEM_NAME to "VxWorks".
if(CMAKE_SYSTEM_NAME STREQUAL "VxWorks")

    # --- VxWorks Configuration ---
    message(STATUS "Configuring for VxWorks")
    add_executable(opengl_triangle main_vxworks.cpp)

    # Link against EGL and GLESv2, which are provided by the VxWorks platform.
    # The names might vary slightly depending on your BSP (e.g., GLESv2_static).
    target_link_libraries(opengl_triangle PRIVATE EGL GLESv2)

else()
    # --- Desktop Configuration ---
    message(STATUS "Configuring for Desktop")
    add_executable(opengl_triangle main_desktop.cpp)

    # Use FetchContent to automatically download and build GLFW.
    include(FetchContent)
    FetchContent_Declare(
      glfw
      GIT_REPOSITORY https://github.com/glfw/glfw.git
      GIT_TAG        3.4 # Use a specific version tag for reproducibility
    )
    FetchContent_MakeAvailable(glfw)

    # Set the preference for GLVND over legacy GL.
    set(OpenGL_GL_PREFERENCE "GLVND")

    # Find other required libraries.
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)

    # Link the executable with the libraries it depends on.
    target_link_libraries(opengl_triangle PRIVATE
        OpenGL::GL
        GLEW::GLEW
        glfw
    )
endif()
