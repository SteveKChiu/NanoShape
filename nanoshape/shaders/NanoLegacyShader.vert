uniform highp mat4 qt_Matrix;

attribute highp vec4 vertex;
attribute highp vec2 tcoord;
varying highp vec2 ftcoord;
varying highp vec2 fpos;

void main(void) {
    gl_Position = qt_Matrix * vertex;
    ftcoord = tcoord;
    fpos = vertex.xy;
}
