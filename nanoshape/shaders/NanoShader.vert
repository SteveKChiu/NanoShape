#version 440

layout(std140, binding = 0) uniform vert {
    mat4 qt_Matrix;
};

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec2 tcoord;
layout(location = 0) out vec2 ftcoord;
layout(location = 1) out vec2 fpos;

out gl_PerVertex { vec4 gl_Position; };

void main(void) {
    gl_Position = qt_Matrix * vertex;
    ftcoord = tcoord;
    fpos = vertex.xy;
}
