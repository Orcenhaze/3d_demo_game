
// @Note: We use this shader format for all of our different shaders. The driver returns -1
// location if the attribute isn't there. 
#define MAX_POINT_LIGHTS 7

struct Location_Point_Light
{
    GLint location_position;
    GLint location_color;
    
#if 0
    GLint location_att_linear;
    GLint location_att_exponential;
    GLint location_ambient_strength;
#endif
    
    GLint location_invert_normal;
};

struct Shader
{
    GLuint id;
    
    // Vertex shader.
    GLint location_position;
    GLint location_tangent;
    GLint location_bitangent;
    GLint location_normal;
    GLint location_uv;
    GLint location_color;
    GLint location_object_to_proj_matrix;
    GLint location_object_to_world_matrix;
    
    // Fragment shader.
    GLint location_final_alpha;
    GLint location_base_color;
    GLint location_specular;
    GLint location_roughness;
    GLint location_diffuse_map;
    GLint location_use_normal_map;
    GLint location_normal_map;
    GLint location_cube_map;
    GLint location_camera_position;
    GLint location_num_point_lights;
    Location_Point_Light location_point_lights[MAX_POINT_LIGHTS];
};
Shader  default_shader;
Shader  immediate_shader;
Shader  terrain_shader;
Shader  skybox_shader;
Shader *all_shaders[] = {&default_shader, &immediate_shader, &terrain_shader, &skybox_shader};
Shader *current_shader;

Matrix4_Inverse world_to_view_matrix;
Matrix4_Inverse view_to_proj_matrix;
Matrix4 object_to_proj_matrix;

GLuint dummy_vao;

struct Vertex_XTBNUC
{
    Vector3 position;
    Vector3 tangent;
    Vector3 bitangent;
    Vector3 normal;
    Vector2 uv;
    Vector4 color;
};

// Vertex_XNUC attribute offsets:
#define OFFSET_POSITION          0
#define OFFSET_TANGENT_XTBNUC    12
#define OFFSET_BITANGENT_XTBNUC  24
#define OFFSET_NORMAL_XTBNUC     36
#define OFFSET_UV_XTBNUC         48
#define OFFSET_COLOR_XTBNUC      56



struct Vertex_XU
{
    Vector3 position;
    Vector2 uv;
};

// Vertex_XU attribute offsets:
#define OFFSET_UV_XU           12

//
// Immediate Mode OpenGL (From https://www.youtube.com/watch?v=g2F0Yg17ZfU).
//
GLuint immediate_vbo;

struct Vertex_XC
{
    Vector3 position;
    Vector4 color;
};

// Vertex_XC attribute offsets:
#define OFFSET_COLOR_XC        12 

s32 num_immediate_vertices;
#define MAX_IMMEDIATE_VERTICES 2400
Vertex_XC immediate_vertices[MAX_IMMEDIATE_VERTICES];
//
//
//

//
// Global meshes.
//
GLfloat global_unit_cube_vertices[] = 
{
    -0.5, -0.5, -0.5,
    0.5, -0.5, -0.5,
    0.5,  0.5, -0.5,
    -0.5,  0.5, -0.5,
    
    -0.5, -0.5,  0.5,
    0.5, -0.5,  0.5,
    0.5,  0.5,  0.5,
    -0.5,  0.5,  0.5,
    
    
    // @Note:
    //
    // 3---2
    // |   | = Back.
    // 0---1
    //
    //   3--------2
	//  /|       /|
	// 7--------6 |
	// | |      | |
	// | 0------|-1
	// |/       |/
	// 4--------5
}; GLuint global_unit_cube_vbo;

GLfloat global_bilateral_cube_vertices[] = 
{
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
}; GLuint global_bilateral_cube_vbo;

GLuint global_cube_line_indices[] = 
{
    0, 1,
    1, 2,
    2, 3,
    3, 0,
    
    4, 5,
    5, 6,
    6, 7,
    7, 4,
    
    5, 1,
    6, 2,
    0, 4,
    3, 7
}; GLuint global_cube_line_ibo;

GLuint global_cube_triangle_indices[] = 
{
    // Back.
    0, 1, 2,
    2, 3, 0,
    
    // Front.
    4, 5, 6,
    6, 7, 4,
    
    // Bottom.
    0, 1, 5,
    5, 4, 0,
    
    // Top.
    7, 6, 2,
    2, 3, 7,
    
    // Left.
    0, 4, 7,
    7, 3, 0,
    
    // Right.
    5, 1, 2,
    2, 6, 5
}; GLuint global_cube_triangle_ibo;

FUNCTION void update_render_transform(Matrix4 object_to_world_matrix, Matrix4 world_to_view = world_to_view_matrix.non_inverse)
{
    object_to_proj_matrix  = view_to_proj_matrix.non_inverse * (world_to_view * object_to_world_matrix);
    
    if(current_shader)
    {        
        extensions->glUniformMatrix4fv(current_shader->location_object_to_proj_matrix, 1, GL_TRUE, &object_to_proj_matrix.I[0]);
        
        if(current_shader->location_object_to_world_matrix != -1)
            extensions->glUniformMatrix4fv(current_shader->location_object_to_world_matrix, 1, GL_TRUE, &object_to_world_matrix.I[0]);
    }
}

FUNCTION void opengl_create_program(Shader *shader, char *vertex_source, char *fragment_source)
{
    GLuint vertex_shader = extensions->glCreateShader(GL_VERTEX_SHADER);
    extensions->glShaderSource(vertex_shader, 1, &vertex_source, NULL);
    extensions->glCompileShader(vertex_shader);
    GLLogProgramErrors("vert shader compilation error", vertex_shader, GL_COMPILE_STATUS);
    
    GLuint fragment_shader = extensions->glCreateShader(GL_FRAGMENT_SHADER);
    extensions->glShaderSource(fragment_shader, 1, &fragment_source, NULL);
    extensions->glCompileShader(fragment_shader);
    GLLogProgramErrors("frag shader compilation error", fragment_shader, GL_COMPILE_STATUS);
    
    GLuint program_id = extensions->glCreateProgram();
    extensions->glAttachShader(program_id, vertex_shader);
    extensions->glAttachShader(program_id, fragment_shader);
    extensions->glLinkProgram(program_id);
    GLLogProgramErrors("program link error", program_id, GL_LINK_STATUS);
    
    extensions->glDeleteShader(vertex_shader);
    extensions->glDeleteShader(fragment_shader);
    
    shader->id                 = program_id;
    shader->location_position  = extensions->glGetAttribLocation(program_id, "position");
    shader->location_tangent   = extensions->glGetAttribLocation(program_id, "tangent");
    shader->location_bitangent = extensions->glGetAttribLocation(program_id, "bitangent");
    shader->location_normal    = extensions->glGetAttribLocation(program_id, "normal");
    shader->location_uv        = extensions->glGetAttribLocation(program_id, "uv");
    shader->location_color     = extensions->glGetAttribLocation(program_id, "color");
    shader->location_object_to_proj_matrix  = extensions->glGetUniformLocation(program_id, "object_to_proj_matrix");
    shader->location_object_to_world_matrix = extensions->glGetUniformLocation(program_id, "object_to_world_matrix");
    shader->location_final_alpha = extensions->glGetUniformLocation(program_id, "final_alpha");
    shader->location_base_color  = extensions->glGetUniformLocation(program_id, "base_color");
    shader->location_specular    = extensions->glGetUniformLocation(program_id, "specular");
    shader->location_roughness   = extensions->glGetUniformLocation(program_id, "roughness");
    shader->location_diffuse_map = extensions->glGetUniformLocation(program_id, "diffuse_map");
    shader->location_use_normal_map = extensions->glGetUniformLocation(program_id, "use_normal_map");
    shader->location_normal_map = extensions->glGetUniformLocation(program_id, "normal_map");
    shader->location_cube_map    = extensions->glGetUniformLocation(program_id, "cube_map");
    shader->location_camera_position  = extensions->glGetUniformLocation(program_id, "camera_position");
    shader->location_num_point_lights = extensions->glGetUniformLocation(program_id, "num_point_lights");
    
    for(u32 i = 0; i < ARRAY_COUNT(shader->location_point_lights); i++)
    {
        Location_Point_Light *pl = &shader->location_point_lights[i];
        
        char name[MAX_STRING];
        MEMORY_ZERO_ARRAY(name);
        
        string_format(name, sizeof(name), "point_lights[%d].position", i);
        pl->location_position = extensions->glGetUniformLocation(program_id, name);
        
        string_format(name, sizeof(name), "point_lights[%d].color", i);
        pl->location_color = extensions->glGetUniformLocation(program_id, name);
        
#if 0
        string_format(name, sizeof(name), "point_lights[%d].att_linear", i);
        pl->location_att_linear = extensions->glGetUniformLocation(program_id, name);
        
        string_format(name, sizeof(name), "point_lights[%d].att_exponential", i);
        pl->location_att_exponential = extensions->glGetUniformLocation(program_id, name);
        
        string_format(name, sizeof(name), "point_lights[%d].ambient_strength", i);
        pl->location_ambient_strength = extensions->glGetUniformLocation(program_id, name);
#endif
        
        string_format(name, sizeof(name), "point_lights[%d].invert_normal", i);
        pl->location_invert_normal = extensions->glGetUniformLocation(program_id, name);
    }
}

FUNCTION void opengl_create_default_shader(Shader *shader)
{
    char *vertex_source = R"XX(
#version 330 core
layout (location = 0) in vec3 position;
        layout (location = 1) in vec3 tangent;
        layout (location = 2) in vec3 bitangent;
        layout (location = 3) in vec3 normal;
        layout (location = 4) in vec2 uv;
        layout (location = 5) in vec4 color;
        
        uniform mat4 object_to_proj_matrix;
uniform mat4 object_to_world_matrix;
        
out vec3 v_position;
        out vec3 v_normal;
        out vec2 v_uv;
        out vec4 v_color;
        out mat3 v_tbn;

void main()
        {
                       vec3 t      = normalize((object_to_world_matrix * vec4(tangent,   0.0f)).xyz);
                       vec3 b      = normalize((object_to_world_matrix * vec4(bitangent, 0.0f)).xyz);
                       vec3 n      = normalize((object_to_world_matrix * vec4(normal,    0.0f)).xyz);

v_position  = vec4(object_to_world_matrix * vec4(position, 1.0f)).xyz;
           v_normal    = n;
           v_uv        = uv;
           v_color     = color;
v_tbn       = mat3(t, b, n);

               gl_Position = object_to_proj_matrix * vec4(position, 1.0);
        }
)XX";
    char *fragment_source = R"XX(
#version 330 core
out vec4 color_out;
                    
const int MAX_POINT_LIGHTS = 7;
struct Point_Light
{
vec3 position;
vec3 color;

//float att_linear;
 //float att_exponential;
//float ambient_strength;

bool invert_normal;
};

in  vec3 v_position;
                    in  vec3 v_normal;
                    in  vec2 v_uv;
                    in  vec4 v_color;
                    in  mat3 v_tbn;

uniform float       final_alpha = 1.0f;
uniform vec4        base_color;
uniform float       specular;
uniform float       roughness;
                    uniform sampler2D   diffuse_map;
uniform bool        use_normal_map;
uniform sampler2D   normal_map;
uniform vec3        camera_position;
uniform int         num_point_lights;
                    uniform Point_Light point_lights[MAX_POINT_LIGHTS];

vec3 specular_light(vec3 normal, vec3 pos_to_light_dir, vec3 light_color)
{
float specular_strength = specular;
float k                 = max((1.0f - roughness) * 64, 1.0f);

vec3 pos_to_cam_dir     = normalize(camera_position - v_position);
vec3 reflect_dir        = reflect(-pos_to_light_dir, normal);
float specular_factor   = pow(max(dot(pos_to_cam_dir, reflect_dir), 0.0f), k);

return specular_strength * specular_factor * light_color;
}

                    vec3 diffuse_light(vec3 normal, vec3 pos_to_light_dir, vec3 light_color)
                    {
                    float diff = max(dot(pos_to_light_dir, normal), 0.0f);
                    return diff * light_color;
                    }
                    
vec4 get_phong(int index, vec3 normal)
{

if(point_lights[index].invert_normal) normal = -normal;

vec3 light_color       = point_lights[index].color;
vec3 pos_to_light_dir  = point_lights[index].position - v_position;
float distance         = length(pos_to_light_dir);
pos_to_light_dir       = normalize(pos_to_light_dir);

float att              = 1.0f + 0.01f*distance + (0.002f * distance * distance);

float ambient_strength = ((1 / num_point_lights) * 0.2f) + 0.1f;
vec3 ambient_          = (ambient_strength * light_color);
ambient_              += (ambient_strength * light_color)                      / att;
vec3 diffuse_          = diffuse_light(normal, pos_to_light_dir, light_color)  / att;
vec3 specular_         = specular_light(normal, pos_to_light_dir, light_color) / att;

return vec4(ambient_ + diffuse_ + specular_, 1.0f);
}

                    void main()
                    {

vec3 normal = normalize(v_normal);
if(use_normal_map)
{
vec3 nm = texture(normal_map, v_uv).xyz;
nm      = 2.0f*nm - vec3(1.0f);
normal  = normalize(v_tbn * nm);
}

vec4 total_light = vec4(0.0f);
for(int i = 0; i < num_point_lights; i++)
{
total_light += get_phong(i, normal);
}

                           vec4 tex_color = texture(diffuse_map, v_uv);
                           color_out      = base_color * tex_color * total_light;
color_out.w    = final_alpha;
}
                
)XX";
    
    opengl_create_program(shader, vertex_source, fragment_source);
}

FUNCTION void opengl_create_immediate_shader(Shader *shader)
{
    char *vertex_source = R"XX(
#version 330 core
layout (location = 0) in vec3 position;
        layout (location = 1) in vec4 color;
        
        uniform mat4 object_to_proj_matrix;
        
out vec4 v_color;
        
void main()
        {
               gl_Position = object_to_proj_matrix * vec4(position, 1.0);
           v_color     = color;
        }
)XX";
    char *fragment_source = R"XX(
#version 330 core
out vec4 color_out;
                    
                    in  vec4 v_color;
                    
                    void main()
                    {
                           color_out = v_color;
}
)XX";
    
    opengl_create_program(shader, vertex_source, fragment_source);
}

FUNCTION void opengl_create_terrain_shader(Shader *shader)
{
    char *vertex_source = R"XX(
#version 330 core
layout (location = 0) in vec3 position;
        layout (location = 1) in vec2 uv;
        
        uniform mat4 object_to_proj_matrix;
uniform mat4 object_to_world_matrix;

out vec3 v_position;
  out vec2 v_uv;

void main()
        {
v_position  = vec4(object_to_world_matrix * vec4(position, 1.0f)).xyz;
v_uv        = uv;
               gl_Position = object_to_proj_matrix * vec4(position, 1.0);
        }
)XX";
    char *fragment_source = R"XX(
#version 330 core
out vec4 color_out;
                    
const int MAX_POINT_LIGHTS = 7;
struct Point_Light
{
vec3 position;
vec3 color;

//float att_linear;
 //float att_exponential;
//float ambient_strength;
};

in vec3 v_position;
in vec2 v_uv;

uniform sampler2D diffuse_map;
uniform int         num_point_lights;
                    uniform Point_Light point_lights[MAX_POINT_LIGHTS];
                    
                    vec3 diffuse_light(vec3 normal, vec3 pos_to_light_dir, vec3 light_color)
                    {
                    float diff = max(dot(pos_to_light_dir, normal), 0.0f);
                    return diff * light_color;
                    }
                    
vec4 get_phong(Point_Light light, vec3 normal)
{
vec3 light_color       = light.color;
vec3 pos_to_light_dir  = light.position - v_position;
float distance         = length(pos_to_light_dir);
pos_to_light_dir       = normalize(pos_to_light_dir);

float att              = 1.0f + 0.01f*distance + (0.002f * distance * distance);

float ambient_strength = ((1 / num_point_lights) * 0.2f) + 0.1f;
vec3 ambient_          = (ambient_strength * light_color);
ambient_              += (ambient_strength * light_color)                     / att;
vec3 diffuse_          = diffuse_light(normal, pos_to_light_dir, light_color) / att;
vec3 specular_         = vec3(0.0f);

return vec4(ambient_ + diffuse_ + specular_, 1.0f);
}

                    void main()
                    {
vec3 dx        = dFdx(v_position);
		vec3 dy        = dFdy(v_position);
		vec3 normal    = normalize(cross(dx, dy));

vec4 total_light = vec4(0.0f);

for(int i = 0; i < num_point_lights; i++)
{
total_light += get_phong(point_lights[i], normal);
}

vec4 tex_color = texture(diffuse_map, v_uv);
                           color_out      = vec4(1.0f) * tex_color * total_light;
}
)XX";
    
    opengl_create_program(shader, vertex_source, fragment_source);
}

FUNCTION void opengl_create_skybox_shader(Shader *shader)
{
    char *vertex_source = R"XX(
#version 330 core
layout (location = 0) in vec3 position;
        
        uniform mat4 object_to_proj_matrix;

  out vec3 v_texcoords;

void main()
        {
v_texcoords = position;
                   
vec4 pos    = object_to_proj_matrix * vec4(position, 1.0);
gl_Position = pos.xyww; // (z = w) to result in depth value of 1.0f.
        }
)XX";
    char *fragment_source = R"XX(
#version 330 core
out vec4 color_out;
                    
in vec3 v_texcoords;

uniform samplerCube cube_map;

                    void main()
                    {
                           color_out = texture(cube_map, v_texcoords);
}
)XX";
    
    opengl_create_program(shader, vertex_source, fragment_source);
}

FUNCTION void generate_buffers_for_global_meshes()
{
    // Unit cube vbo.
    extensions->glGenBuffers(1, &global_unit_cube_vbo);
    extensions->glBindBuffer(GL_ARRAY_BUFFER, global_unit_cube_vbo);
    extensions->glBufferData(GL_ARRAY_BUFFER, sizeof(global_unit_cube_vertices), global_unit_cube_vertices, GL_STATIC_DRAW);
    
    // Bilateral cube vbo.
    extensions->glGenBuffers(1, &global_bilateral_cube_vbo);
    extensions->glBindBuffer(GL_ARRAY_BUFFER, global_bilateral_cube_vbo);
    extensions->glBufferData(GL_ARRAY_BUFFER, sizeof(global_bilateral_cube_vertices), global_bilateral_cube_vertices, GL_STATIC_DRAW);
    
    // Unit cube line ibo.
    extensions->glGenBuffers(1, &global_cube_line_ibo);
    extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, global_cube_line_ibo);
    extensions->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(global_cube_line_indices), global_cube_line_indices, GL_STATIC_DRAW);
    
    // Unit cube triangle ibo.
    extensions->glGenBuffers(1, &global_cube_triangle_ibo);
    extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, global_cube_triangle_ibo);
    extensions->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(global_cube_triangle_indices), global_cube_triangle_indices, GL_STATIC_DRAW);
}

FUNCTION void opengl_init(Game_Memory *memory)
{
    //
    // Create all shaders at startup and refer to them before rendering.
    //
    opengl_create_default_shader(&default_shader);
    opengl_create_terrain_shader(&terrain_shader);
    opengl_create_skybox_shader(&skybox_shader);
    opengl_create_immediate_shader(&immediate_shader);
    
    //
    // We have to create a VAO if we're using core OpenGL.
    //
    extensions->glGenVertexArrays(1, &dummy_vao);
    extensions->glBindVertexArray(dummy_vao);
    
    extensions->glGenBuffers(1, &immediate_vbo);
    
    //
    // Generate buffers for our global meshes.
    //
    generate_buffers_for_global_meshes();
    
    //
    // Projection transform.
    //
    f32 w = (f32)memory->settings.render_dimensions.width;
    f32 h = (f32)memory->settings.render_dimensions.height;
    f32 fov       = 100.0f;
    f32 near_clip = 0.1f;
    f32 far_clip  = 1000.0f;
    f32 ar        = w/h;
    f32 scale     = 10.0f;
    view_to_proj_matrix = perspective(to_radians(fov), ar, near_clip, far_clip);
    //view_to_proj_matrix = orthographic(-ar*scale, ar*scale, -scale, scale);
}

FUNCTION void set_vertex_format_to_X()
{
    Shader *s = current_shader;
    
    if(s->location_position != -1)
    {
        extensions->glEnableVertexAttribArray(s->location_position);
        extensions->glVertexAttribPointer(s->location_position, 3, GL_FLOAT, GL_FALSE, 
                                          0, OFFSET_POSITION);
    }
}

FUNCTION void set_vertex_format_to_XC()
{
    GLsizei stride = sizeof(Vertex_XC);
    Shader *s = current_shader;
    
    if(s->location_position != -1)
    {
        extensions->glEnableVertexAttribArray(s->location_position);
        extensions->glVertexAttribPointer(s->location_position, 3, GL_FLOAT, GL_FALSE, 
                                          stride, OFFSET_POSITION);
    }
    
    if(s->location_color != -1)
    {
        extensions->glEnableVertexAttribArray(s->location_color);
        extensions->glVertexAttribPointer(s->location_color, 4, GL_FLOAT, GL_FALSE, 
                                          stride, (const void *) OFFSET_COLOR_XC);
    }
}

FUNCTION void set_vertex_format_to_XU()
{
    GLsizei stride = sizeof(Vertex_XU);
    Shader *s = current_shader;
    
    if(s->location_position != -1)
    {
        extensions->glEnableVertexAttribArray(s->location_position);
        extensions->glVertexAttribPointer(s->location_position, 3, GL_FLOAT, GL_FALSE, 
                                          stride, OFFSET_POSITION);
    }
    
    if(s->location_uv != -1)
    {
        extensions->glEnableVertexAttribArray(s->location_uv);
        extensions->glVertexAttribPointer(s->location_uv, 2, GL_FLOAT, GL_FALSE, 
                                          stride, (const void *) OFFSET_UV_XU);
    }
}

FUNCTION void set_vertex_format_to_XTBNUC()
{
    GLsizei stride = sizeof(Vertex_XTBNUC);
    Shader *s      = current_shader;
    
    if(s->location_position != -1)
    {
        extensions->glEnableVertexAttribArray(s->location_position);
        extensions->glVertexAttribPointer(s->location_position, 3, GL_FLOAT, GL_FALSE, 
                                          stride, OFFSET_POSITION);
    }
    
    if(s->location_tangent != -1)
    {
        extensions->glEnableVertexAttribArray(s->location_tangent);
        extensions->glVertexAttribPointer(s->location_tangent, 3, GL_FLOAT, GL_FALSE, 
                                          stride, (const void *) OFFSET_TANGENT_XTBNUC);
    }
    
    if(s->location_bitangent != -1)
    {
        extensions->glEnableVertexAttribArray(s->location_bitangent);
        extensions->glVertexAttribPointer(s->location_bitangent, 3, GL_FLOAT, GL_FALSE, 
                                          stride, (const void *) OFFSET_BITANGENT_XTBNUC);
    }
    
    if(s->location_normal != -1)
    {
        extensions->glEnableVertexAttribArray(s->location_normal);
        extensions->glVertexAttribPointer(s->location_normal, 3, GL_FLOAT, GL_FALSE, 
                                          stride, (const void *) OFFSET_NORMAL_XTBNUC);
    }
    
    if(s->location_uv != -1)
    {
        extensions->glEnableVertexAttribArray(s->location_uv);
        extensions->glVertexAttribPointer(s->location_uv, 2, GL_FLOAT, GL_FALSE, 
                                          stride, (const void *) OFFSET_UV_XTBNUC);
    }
    
    if(s->location_color != -1)
    {
        extensions->glEnableVertexAttribArray(s->location_color);
        extensions->glVertexAttribPointer(s->location_color, 4, GL_FLOAT, GL_FALSE, 
                                          stride, (const void *) OFFSET_COLOR_XTBNUC);
    }
}





//
// Immediate Mode OpenGL (Inspired by https://www.youtube.com/watch?v=g2F0Yg17ZfU).  
//
FUNCTION void immediate_flush()
{
    if(!num_immediate_vertices) return;
    
    // Bind shader.
    current_shader = &immediate_shader;
    if(!current_shader)
    {
        if(num_immediate_vertices)
        {
            PRINT("No shader is set.\n");
            ASSERT(false);
        }
        
        num_immediate_vertices = 0;
        return;
    }
    extensions->glUseProgram(current_shader->id);
    
    // Update transforms.
    update_render_transform(matrix4_identity());
    
    // Bind buffers and set vertex format.
    s32 num_vertices = num_immediate_vertices;
    extensions->glBindBuffer(GL_ARRAY_BUFFER, immediate_vbo);
    extensions->glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_XC) * num_vertices, &immediate_vertices[0], GL_STREAM_DRAW);
    set_vertex_format_to_XC();
    
    //extensions->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    extensions->glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    extensions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    num_immediate_vertices = 0;
}

FUNCTION void immediate_begin(GLenum mode = GL_FILL)
{
    extensions->glPolygonMode(GL_FRONT_AND_BACK, mode);
    immediate_flush();
}

FUNCTION Vertex_XC* immediate_vertex_ptr(s32 index)
{
    if(index == MAX_IMMEDIATE_VERTICES) 
    {
        PRINT("Maximum allowed vertices reached.\n");
        ASSERT(false);
    }
    
    Vertex_XC *result = immediate_vertices + index;
    return result;
}

FUNCTION void immediate_vertex(Vector3 position, Vector4 color)
{
    if(num_immediate_vertices == MAX_IMMEDIATE_VERTICES) immediate_flush();
    
    Vertex_XC *v = immediate_vertex_ptr(num_immediate_vertices);
    v->position  = position;
    v->color     = color;
    
    num_immediate_vertices += 1;
}

FUNCTION void immediate_triangle(Vector3 p0, Vector3 p1, Vector3 p2, Vector4 color)
{
    if((num_immediate_vertices + 3) > MAX_IMMEDIATE_VERTICES) immediate_flush();
    
    immediate_vertex(p0, color);
    immediate_vertex(p1, color);
    immediate_vertex(p2, color);
}

FUNCTION void immediate_quad(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, Vector4 color)
{
    // @Note: CCW.
    
    if((num_immediate_vertices + 6) > MAX_IMMEDIATE_VERTICES) immediate_flush();
    
    immediate_triangle(p0, p1, p2, color);
    immediate_triangle(p2, p3, p0, color);
}

FUNCTION void immediate_quad(Plane plane, f32 scale, Vector4 color)
{
    Vector3 c = plane.center; Vector3 normal = plane.normal;
    
    // Calculate quad tangent and bitangent.
    Vector3 tangent, bitangent;
    calculate_tangents(normal, &tangent, &bitangent);
    
    scale /= 2.0f;
    
    Vector3 p0 = c - tangent*scale - bitangent*scale;
    Vector3 p1 = c + tangent*scale - bitangent*scale;
    Vector3 p2 = c + tangent*scale + bitangent*scale;
    Vector3 p3 = c - tangent*scale + bitangent*scale;
    
    immediate_quad(p0, p1, p2, p3, color);
}

FUNCTION void immediate_hexahedron(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3,
                                   Vector3 p4, Vector3 p5, Vector3 p6, Vector3 p7,
                                   Vector4 color = make_vector4(1.0f))
{
    // @Note: p0 to p3 --> back // p4 to p7 --> front // CCW.
    
    immediate_quad(p0, p1, p2, p3, color); // Back
    immediate_quad(p4, p5, p6, p7, color); // Front
    immediate_quad(p0, p1, p5, p4, color); // Bottom
    immediate_quad(p7, p6, p2, p3, color); // Top
    immediate_quad(p0, p4, p7, p3, color); // Left
    immediate_quad(p5, p1, p2, p6, color); // Right
}

FUNCTION void immediate_cuboid(Vector3 center, Vector3 half_size, Vector4 color = make_vector4(1.0f))
{
    Vector3 p0 = {center.x - half_size.x, center.y - half_size.y, center.z - half_size.z};
    Vector3 p1 = {center.x + half_size.x, center.y - half_size.y, center.z - half_size.z};
    Vector3 p2 = {center.x + half_size.x, center.y + half_size.y, center.z - half_size.z};
    Vector3 p3 = {center.x - half_size.x, center.y + half_size.y, center.z - half_size.z};
    Vector3 p4 = {center.x - half_size.x, center.y - half_size.y, center.z + half_size.z};
    Vector3 p5 = {center.x + half_size.x, center.y - half_size.y, center.z + half_size.z};
    Vector3 p6 = {center.x + half_size.x, center.y + half_size.y, center.z + half_size.z};
    Vector3 p7 = {center.x - half_size.x, center.y + half_size.y, center.z + half_size.z};
    
    immediate_hexahedron(p0, p1, p2, p3, p4, p5, p6, p7, color);
}

FUNCTION void immediate_cuboid(Rect3 box, Vector4 color = make_vector4(1.0f))
{
    Vector3 center    = rect3_get_center(box);
    Vector3 half_size = rect3_get_scale(box) / 2.0f;
    immediate_cuboid(center, half_size, color);
}

FUNCTION void immediate_cube(Vector3 center, f32 half_size, Vector4 color = make_vector4(1.0f))
{
    immediate_cuboid(center, make_vector3(half_size), color);
}

FUNCTION void immediate_line(Vector3 p0, Vector3 p1, Vector4 color = make_vector4(1.0), f32 thickness = 0.1f)
{
    Vector3 a[2];
    Vector3 b[2];
    
    Vector3 line_d  = p1 - p0;
    f32 line_length = length(line_d);
    line_d          = normalize(line_d);
    
    // Half-thickness.
    f32 half_thickness = thickness * 0.5f;
    
    Vector3 tangent   = {};
    Vector3 bitangent = {};
    calculate_tangents(line_d, &tangent, &bitangent);
    
    // Make a `+` sign to construct the points. 
    Vector3 p = p0;
    for(s32 sindex = 0; sindex < 2; sindex++)
    {
        a[sindex] = p + tangent*half_thickness;
        b[sindex] = p - tangent*half_thickness;
        
        p = p + line_length*line_d;
    }
    
    // Back CCW.
    Vector3 v0 = a[1] - bitangent*half_thickness;
    Vector3 v1 = b[1] - bitangent*half_thickness;
    Vector3 v2 = b[1] + bitangent*half_thickness;
    Vector3 v3 = a[1] + bitangent*half_thickness;
    
    // Front CCW.
    Vector3 v4 = a[0] - bitangent*half_thickness;
    Vector3 v5 = b[0] - bitangent*half_thickness;
    Vector3 v6 = b[0] + bitangent*half_thickness;
    Vector3 v7 = a[0] + bitangent*half_thickness;
    
    immediate_hexahedron(v0, v1, v2, v3, v4, v5, v6, v7, color);
}

FUNCTION void immediate_torus(Vector3 center, f32 radius, Vector3 normal, Vector4 color = make_vector4(1.0f), f32 thickness = 0.1f)
{
    f32 half_thickness = thickness * 0.5f;
    
#define NUM_SEGMENTS 50
    Vector3 inner_points_back [NUM_SEGMENTS];
    Vector3 outer_points_back [NUM_SEGMENTS];
    Vector3 inner_points_front[NUM_SEGMENTS];
    Vector3 outer_points_front[NUM_SEGMENTS];
    f32 inner_radius = radius - half_thickness;
    f32 outer_radius = radius + half_thickness;
    f32 step         = 1.0f / NUM_SEGMENTS;
    
    Vector3 tangent = {};
    Vector3 unused  = {};
    calculate_tangents(normal, &tangent, &unused);
    
    // Initial points.
    inner_points_back [0] = inner_radius*tangent - half_thickness*normal;
    outer_points_back [0] = outer_radius*tangent - half_thickness*normal;
    inner_points_front[0] = inner_radius*tangent + half_thickness*normal;
    outer_points_front[0] = outer_radius*tangent + half_thickness*normal;
    
    for(s32 i = 1; i < NUM_SEGMENTS; i++)
    {
        f32 theta    = TAU * (step * i);
        Quaternion q = make_quaternion_from_axis_and_angle(normal, theta);
        
        // Rotate the points.
        inner_points_back [i] = q * inner_points_back [0];
        outer_points_back [i] = q * outer_points_back [0];
        inner_points_front[i] = q * inner_points_front[0];
        outer_points_front[i] = q * outer_points_front[0];
        
        // Back CCW.
        Vector3 p0 = center + inner_points_back[i];
        Vector3 p1 = center + inner_points_back[i - 1];
        Vector3 p2 = center + outer_points_back[i - 1];
        Vector3 p3 = center + outer_points_back[i];
        
        // Front CCW.
        Vector3 p4 = center + inner_points_front[i];
        Vector3 p5 = center + inner_points_front[i - 1];
        Vector3 p6 = center + outer_points_front[i - 1];
        Vector3 p7 = center + outer_points_front[i];
        
        immediate_hexahedron(p0, p1, p2, p3, p4, p5, p6, p7, color);
    }
    
    // Draw last segment.
    Vector3 p0 = center + inner_points_back[0];
    Vector3 p1 = center + inner_points_back[NUM_SEGMENTS - 1];
    Vector3 p2 = center + outer_points_back[NUM_SEGMENTS - 1];
    Vector3 p3 = center + outer_points_back[0];
    
    Vector3 p4 = center + inner_points_front[0];
    Vector3 p5 = center + inner_points_front[NUM_SEGMENTS - 1];
    Vector3 p6 = center + outer_points_front[NUM_SEGMENTS - 1];
    Vector3 p7 = center + outer_points_front[0];
    
    immediate_hexahedron(p0, p1, p2, p3, p4, p5, p6, p7, color);
}