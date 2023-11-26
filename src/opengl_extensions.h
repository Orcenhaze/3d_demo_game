#ifndef OPENGL_HEADER_H
#define OPENGL_HEADER_H

typedef size_t GLsizeiptr;
typedef char   GLchar;
typedef s16    GLshort;
typedef s8     GLbyte;
typedef u16    GLushort;
typedef void   GLvoid;
typedef u32    GLenum;
typedef f32    GLfloat;
typedef int    GLint;
typedef int    GLsizei;
typedef u32    GLbitfield;
typedef double GLdouble;
typedef u32    GLuint;
typedef u8     GLboolean;
typedef u8     GLubyte;

#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_LESS                           0x0201
#define GL_LEQUAL                         0x0203
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STREAM_DRAW                    0x88E0
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_TRIANGLES                      0x0004
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_TEXTURE_2D                     0x0DE1
#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_TEXTURE_WRAP_R                 0x8072
#define GL_CLAMP                          0x2900
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_REPEAT                         0x2901
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_RGBA                           0x1908
#define GL_RGBA8                          0x8058
#define GL_STATIC_DRAW                    0x88E4
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_TEXTURE0                       0x84C0
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE                           0x1B01
#define GL_FILL                           0x1B02
#define GL_FRONT_AND_BACK                 0x0408
#define GL_MULTISAMPLE                    0x809D
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_DEPTH_TEST                     0x0B71
#define GL_BLEND                          0x0BE2
#define GL_TEXTURE_CUBE_MAP_SEAMLESS      0x884F

struct Opengl_Extensions
{
    GLuint (*glCreateShader)(GLenum type);
    void   (*glShaderSource) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
    void   (*glAttachShader)(GLuint program, GLuint shader);
    void   (*glCompileShader)(GLuint shader);
    void   (*glGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
    GLuint (*glCreateProgram)(void);
    void   (*glLinkProgram)(GLuint program);
    void   (*glUseProgram)(GLuint program);
    void   (*glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    void   (*glValidateProgram)(GLuint program);
    void   (*glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
    void   (*glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    void   (*glDeleteShader)(GLuint shader);
    GLint  (*glGetAttribLocation)(GLuint program, const GLchar *name);
    void   (*glEnableVertexAttribArray)(GLuint index);
    void   (*glDisableVertexAttribArray)(GLuint index);
    void   (*glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    void   (*glGenVertexArrays)(GLsizei n, GLuint *arrays);
    void   (*glBindVertexArray)(GLuint array);
    void   (*glGenBuffers)(GLsizei n, GLuint *buffers);
    void   (*glBindBuffer)(GLenum target, GLuint buffer);
    void   (*glBufferData)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
    
    void   (*glDepthFunc)(GLenum func);
    void   (*glBlendFunc)(GLenum sfactor, GLenum dfactor);
    void   (*glDisable)(GLenum cap);
    void   (*glEnable)(GLenum cap);
    
    void   (*glGenTextures)(GLsizei n, GLuint *textures);
    void   (*glGenerateMipmap)(GLenum target);
    void   (*glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
    void   (*glTexParameteri)(GLenum target, GLenum pname, GLint param);
    void   (*glActiveTexture)(GLenum texture);
    void   (*glBindTexture)(GLenum target, GLuint texture);
    
    
    
    void   (*glDrawArrays)(GLenum mode, GLint first, GLsizei count);
    void   (*glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void *indices);
    void   (*glDrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
    
    
    GLint  (*glGetUniformLocation)(GLuint program, const GLchar *name);
    void   (*glUniform1f)(GLint location, GLfloat v0);
    void   (*glUniform1i)(GLint location, GLint v0);
    void   (*glUniform3fv)(GLint location, GLsizei count, const GLfloat *value);
    void   (*glUniform4fv)(GLint location, GLsizei count, const GLfloat *value);
    void   (*glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    
    
    void   (*glPolygonMode)(GLenum face, GLenum mode);
    void   (*glClear)(GLbitfield mask);
    GLuint (*glGetDebugMessageLogARB)(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
};
Opengl_Extensions *extensions;

#endif //OPENGL_HEADER_H
