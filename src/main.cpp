#include <stdlib.h>
#include <iostream>
#include <memory>

#define GL_GLEXT_PROTOTYPES
#include <GLES3/gl3.h>
#include <EGL/egl.h>

GLuint loadShader(GLenum type, const GLchar* src)
{
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        return 0;
    }
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        if (len > 1) {
            auto log = std::unique_ptr<GLchar>(new GLchar[len]);
            glGetShaderInfoLog(shader, len, nullptr, log.get());
            const char* type_string = "unknown";
            if (type == GL_VERTEX_SHADER) {
                type_string = "vertex";
            }
            if (type == GL_FRAGMENT_SHADER) {
                type_string = "fragment";
            }
            std::cerr << "Error compiling " << type_string << " shader:" << std::endl << log.get() << std::endl;
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint loadProgram(const GLchar* vshSrc, const GLchar* fshSrc)
{
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vshSrc);
    if (vertexShader == 0) {
        return 0;
    }
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fshSrc);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return 0;
    }

    GLuint programObject = glCreateProgram();
    if (programObject == 0) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    // Free up no longer needed shader resources
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Link the program
    glLinkProgram(programObject);

    GLint status;
    glGetProgramiv(programObject, GL_LINK_STATUS, &status);
    if (!status) {
        GLint len = 0;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &len);
        if (len > 1) {
            auto log = std::unique_ptr<GLchar>(new GLchar[len]);
            glGetProgramInfoLog(programObject, len, nullptr, log.get());
            std::cerr << "Error linking program:" << std::endl << log.get() << std::endl;
        }
        glDeleteProgram(programObject);
        return 0;
    }

    return programObject;
}

int main(int argc, char* argv[])
{
    EGLDisplay display;
    display = eglGetDisplay((EGLNativeDisplayType)EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        printf("eglGetError(): 0x%x\n", eglGetError());
        return EGL_FALSE;
    }

    EGLint majorVersion;
    EGLint minorVersion;
    if (!eglInitialize(display, &majorVersion, &minorVersion)) {
        printf("eglGetError(): 0x%x\n", eglGetError());
        return EGL_FALSE;
    }
    printf("eglInitialize(): major = %d, minor = %d\n", majorVersion, minorVersion);

    EGLint numConfigs;
    if (!eglGetConfigs(display, NULL, 0, &numConfigs)) {
        printf("eglGetError(): 0x%x\n", eglGetError());
        return EGL_FALSE;
    }
    printf("eglGetConfigs(): num = %d\n", numConfigs);

    EGLint attribList[] =
    {
        EGL_SURFACE_TYPE,   EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT,
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        //EGL_ALPHA_SIZE,     (flags & ES_WINDOW_ALPHA) ? 8 : EGL_DONT_CARE,
        //EGL_DEPTH_SIZE,     (flags & ES_WINDOW_DEPTH) ? 8 : EGL_DONT_CARE,
        //EGL_STENCIL_SIZE,   (flags & ES_WINDOW_STENCIL) ? 8 : EGL_DONT_CARE,
        //EGL_SAMPLE_BUFFERS, (flags & ES_WINDOW_MULTISAMPLE) ? 1 : 0,
        EGL_NONE
    };
    EGLConfig config;
    if (!eglChooseConfig(display, attribList, &config, 1, &numConfigs)) {
        printf("eglGetError(): 0x%x\n", eglGetError());
        return EGL_FALSE;
    }
    {
        printf("eglChooseConfig(): config = %p, num = %d\n", config, numConfigs);
        EGLint value;
        eglGetConfigAttrib(display, config, EGL_BIND_TO_TEXTURE_RGB, &value);
        printf("  EGL_BIND_TO_TEXTURE_RGB = %d\n", value);
        eglGetConfigAttrib(display, config, EGL_BIND_TO_TEXTURE_RGBA, &value);
        printf("  EGL_BIND_TO_TEXTURE_RGBA = %d\n", value);
        eglGetConfigAttrib(display, config, EGL_CONFORMANT, &value);
        printf("  EGL_CONFORMANT = %d\n", value);
        eglGetConfigAttrib(display, config, EGL_MAX_PBUFFER_WIDTH, &value);
        printf("  EGL_MAX_PBUFFER_WIDTH = %d\n", value);
        eglGetConfigAttrib(display, config, EGL_MAX_PBUFFER_HEIGHT, &value);
        printf("  EGL_MAX_PBUFFER_HEIGHT = %d\n", value);
        eglGetConfigAttrib(display, config, EGL_MAX_PBUFFER_PIXELS, &value);
        printf("  EGL_MAX_PBUFFER_PIXELS = %d\n", value);
        eglGetConfigAttrib(display, config, EGL_NATIVE_RENDERABLE, &value);
        printf("  EGL_NATIVE_RENDERABLE = %d\n", value);
        eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &value);
        printf("  EGL_NATIVE_VISUAL_ID = %d\n", value);
        eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_TYPE, &value);
        printf("  EGL_NATIVE_VISUAL_TYPE = %d\n", value);
        eglGetConfigAttrib(display, config, EGL_RENDERABLE_TYPE, &value);
        printf("  EGL_RENDERABLE_TYPE EGL_OPENGL_ES_BIT = %d\n", value & EGL_OPENGL_ES_BIT);
        printf("  EGL_RENDERABLE_TYPE EGL_OPENGL_ES2_BIT = %d\n", value & EGL_OPENGL_ES2_BIT);
        printf("  EGL_RENDERABLE_TYPE EGL_OPENGL_ES3_BIT = %d\n", value & EGL_OPENGL_ES3_BIT);
        printf("  EGL_RENDERABLE_TYPE EGL_OPENGL_BIT = %d\n", value & EGL_OPENGL_BIT);
        printf("  EGL_RENDERABLE_TYPE EGL_OPENVG_BIT = %d\n", value & EGL_OPENVG_BIT);
        eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &value);
        printf("  EGL_SURFACE_TYPE EGL_MULTISAMPLE_RESOLVE_BOX_BIT = %d\n", value & EGL_MULTISAMPLE_RESOLVE_BOX_BIT);
        printf("  EGL_SURFACE_TYPE EGL_PBUFFER_BIT = %d\n", value & EGL_PBUFFER_BIT);
        printf("  EGL_SURFACE_TYPE EGL_PIXMAP_BIT = %d\n", value & EGL_PIXMAP_BIT);
        printf("  EGL_SURFACE_TYPE EGL_SWAP_BEHAVIOR_PRESERVED_BIT = %d\n", value & EGL_SWAP_BEHAVIOR_PRESERVED_BIT);
        printf("  EGL_SURFACE_TYPE EGL_WINDOW_BIT = %d\n", value & EGL_WINDOW_BIT);
    }

    EGLSurface surface;
#if 1
    EGLint pbufferAttribs[] =
    {
        EGL_WIDTH, 32,
        EGL_HEIGHT, 32,
        EGL_NONE,
    };
    surface = eglCreatePbufferSurface(display, config, pbufferAttribs);
    if (surface == EGL_NO_SURFACE) {
        printf("eglGetError(): 0x%x\n", eglGetError());
        return EGL_FALSE;
    }
#else
    // surfaceless
    surface = EGL_NO_SURFACE;
#endif

    EGLint contextAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE,
    };
    EGLContext context;
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT) {
        printf("eglGetError(): 0x%x\n", eglGetError());
        return EGL_FALSE;
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        printf("eglGetError(): 0x%x\n", eglGetError());
        return EGL_FALSE;
    }
    {
        std::cout << "EGL_VENDOR: " << eglQueryString(display, EGL_VENDOR) << std::endl;
        std::cout << "EGL_VERSION: " << eglQueryString(display, EGL_VERSION) << std::endl;
        std::cout << "EGL_EXTENSIONS: " << std::endl;
        auto exts = std::string {eglQueryString(display, EGL_EXTENSIONS)};
        std::string::size_type start = 0;
        while (1) {
            auto pos = exts.find(" ", start);
            if (pos == std::string::npos) {
                std::cout << "  " << exts.substr(start) << std::endl;
                break;
            } else {
                std::cout << "  " << exts.substr(start, pos - start) << std::endl;
                start = pos + 1;
            }
        }
    }


    // GLES
    {
        std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GL_SHADING_LANGUAGE_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        GLint value;
        glGetIntegerv(GL_MAJOR_VERSION, &value);
        std::cout << "GL_MAJOR_VERSION: " << value << std::endl;
        glGetIntegerv(GL_MINOR_VERSION, &value);
        std::cout << "GL_MINOR_VERSION: " << value << std::endl;
        glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &value);
        std::cout << "GL_NUM_PROGRAM_BINARY_FORMATS: " << value << std::endl;
        glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &value);
        std::cout << "GL_NUM_SHADER_BINARY_FORMATS: " << value << std::endl;
        glGetIntegerv(GL_SHADER_COMPILER, &value);
        std::cout << "GL_SHADER_COMPILER: " << value << std::endl;
        glGetIntegerv(GL_NUM_EXTENSIONS, &value);
        std::cout << "GL_NUM_EXTENSIONS: " << value << std::endl;
        for (GLint i = 0; i < value; ++i) {
            std::cout << "  " << glGetStringi(GL_EXTENSIONS, i) << std::endl;
        }
    }
    GLuint programObject = loadProgram(
        "#version 300 es\n" "void main() {gl_Position = vec4(0.0);}",
        "#version 300 es\n" "precision mediump float;" "out vec4 fragColor; void main() {fragColor = vec4(1.0);}");
    if (programObject != 0) {
        GLint binary_len = 0;
        glGetProgramiv(programObject, GL_PROGRAM_BINARY_LENGTH, &binary_len);
        std::cout << "program binary length: " << binary_len << std::endl;

        glDeleteProgram(programObject);
    }

    GLenum glerr = glGetError();
    if (glerr == GL_NO_ERROR) {
        std::cout << "glGetError(): " << "GL_NO_ERROR" << std::endl;
    } else {
        std::cout << "glGetError(): 0x" << std::hex << glerr << std::dec << std::endl;
    }


    // terminate
    if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        printf("eglGetError(): 0x%x\n", eglGetError());
        return EGL_FALSE;
    }
    if (!eglDestroyContext(display, context)) {
        printf("eglGetError(): 0x%x\n", eglGetError());
        return EGL_FALSE;
    }
    if (!eglDestroySurface(display, surface)) {
        printf("eglGetError(): 0x%x\n", eglGetError());
        return EGL_FALSE;
    }
    if (!eglTerminate(display)) {
        printf("eglGetError(): 0x%x\n", eglGetError());
        return EGL_FALSE;
    }

    EGLint err = eglGetError();
    if (err == EGL_SUCCESS) {
        printf("eglGetError(): EGL_SUCCESS\n");
    } else {
        printf("eglGetError(): 0x%x\n", err);
    }
    return EGL_TRUE;
} 
