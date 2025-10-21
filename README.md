# OpenGLTriangle

An application that draws a simple gradient triangle on a white background.
This is meant as a very simple test of OpenGL capabilities and color accuracy.

## Platforms

In this CMake project, there is:
* A version for desktop, that links with Desktop OpenGL, GLEW, and GLFW libs.
* A separate version for GL ES + EGL for VxWorks, using the Vivante OpenGL ES
  libraries.
