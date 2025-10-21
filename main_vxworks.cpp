#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <iostream>
#include <cstring>
// For VxWorks, you may need taskLib for taskDelay
#include <taskLib.h> 

// Vertex shader with color attribute
const char* vertexShaderSrc = R"(
attribute vec4 a_position;
attribute vec3 a_color;
varying vec3 v_color;
void main() {
    gl_Position = a_position;
    v_color = a_color;
}
)";

// Fragment shader with color gradient
const char* fragmentShaderSrc = R"(
precision mediump float;
varying vec3 v_color;
void main() {
    gl_FragColor = vec4(v_color, 1.0);
}
)";

GLuint loadShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = new char[infoLen];
            glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
            std::cerr << "Error compiling shader:\n" << infoLog << std::endl;
            delete[] infoLog;
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint createProgram(const char* vtxSrc, const char* fragSrc) {
    GLuint vtxShader = loadShader(GL_VERTEX_SHADER, vtxSrc);
    GLuint fragShader = loadShader(GL_FRAGMENT_SHADER, fragSrc);
    
    if (!vtxShader || !fragShader) {
        return 0;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vtxShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint infoLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = new char[infoLen];
            glGetProgramInfoLog(program, infoLen, nullptr, infoLog);
            std::cerr << "Error linking program:\n" << infoLog << std::endl;
            delete[] infoLog;
        }
        glDeleteProgram(program);
        return 0;
    }
    
    glDeleteShader(vtxShader);
    glDeleteShader(fragShader);
    
    return program;
}

// Entry point for VxWorks is often not 'main', but a function with a specific signature.
// Renaming to 'vx_main' for clarity, but you should adjust to your RTP's entry point.
int vx_main() {
    // EGL initialization
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        std::cerr << "Failed to get EGL display" << std::endl;
        return -1;
    }
    
    EGLint major, minor;
    if (!eglInitialize(display, &major, &minor)) {
        std::cerr << "Failed to initialize EGL" << std::endl;
        return -1;
    }
    
    std::cout << "EGL version: " << major << "." << minor << std::endl;
    
    // Choose config
    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_NONE
    };
    
    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs)) {
        std::cerr << "Failed to choose EGL config" << std::endl;
        return -1;
    }
    
    // Create window surface (uses Vivante framebuffer on i.MX6)
    EGLNativeWindowType nativeWindow = 0; // VxWorks/Vivante uses NULL for default FB
    EGLSurface surface = eglCreateWindowSurface(display, config, nativeWindow, nullptr);
    if (surface == EGL_NO_SURFACE) {
        std::cerr << "Failed to create EGL surface" << std::endl;
        return -1;
    }
    
    // Create OpenGL ES 2.0 context
    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT) {
        std::cerr << "Failed to create EGL context" << std::endl;
        return -1;
    }
    
    if (!eglMakeCurrent(display, surface, surface, context)) {
        std::cerr << "Failed to make EGL context current" << std::endl;
        return -1;
    }
    
    // Get surface dimensions
    EGLint width, height;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);
    std::cout << "Surface size: " << width << "x" << height << std::endl;
    
    // Create shader program
    GLuint program = createProgram(vertexShaderSrc, fragmentShaderSrc);
    if (!program) {
        std::cerr << "Failed to create shader program" << std::endl;
        return -1;
    }
    
    // Get attribute locations
    GLint positionLoc = glGetAttribLocation(program, "a_position");
    GLint colorLoc = glGetAttribLocation(program, "a_color");
    
    // Triangle vertices (centered, normalized device coordinates)
    GLfloat vertices[] = {
         0.0f,  0.5f, 0.0f,  // Top vertex
        -0.5f, -0.5f, 0.0f,  // Bottom left
         0.5f, -0.5f, 0.0f   // Bottom right
    };
    
    // Color gradient (RGB per vertex)
    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,  // Red
        0.0f, 1.0f, 0.0f,  // Green
        0.0f, 0.0f, 1.0f   // Blue
    };
    
    // Set viewport
    glViewport(0, 0, width, height);
    
    // Rendering loop (render once for this example)
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // White background
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(program);
    
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLoc);
    
    glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, colors);
    glEnableVertexAttribArray(colorLoc);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    glDisableVertexAttribArray(positionLoc);
    glDisableVertexAttribArray(colorLoc);
    
    eglSwapBuffers(display, surface);
    
    std::cout << "Triangle rendered. Press Ctrl+C in host shell to exit..." << std::endl;
    
    // Keep running (in real application, you'd have a proper event loop)
    while (true) {
        taskDelay(60);  // VxWorks sleep for ~1 second (assuming 60 ticks/sec)
    }
    
    // Cleanup (won't reach here without proper signal handling)
    glDeleteProgram(program);
    eglDestroySurface(display, surface);
    eglDestroyContext(display, context);
    eglTerminate(display);
    
    return 0;
}

// In many VxWorks systems, 'main' is not the entry point for a Real-Time Process (RTP).
// You might need a wrapper or to name your entry point according to your project's convention.
// If your system does use 'main', you can just rename 'vx_main' to 'main'.
int main(int argc, char *argv[])
{
    return vx_main();
}
