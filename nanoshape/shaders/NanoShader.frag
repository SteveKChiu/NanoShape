#version 440

layout(std140, binding = 0) uniform frag {
    mat4 qt_Matrix;
    float qt_Opacity;
    mat3 paintMatrix;
    vec4 innerColor;
    vec4 outerColor;
    vec2 extent;
    float radius;
    float feather;
    float strokeMultiply;
    float strokeThreshold;
    float dashOffset;
    float dashUnit;
    int type;
    int edgeAA;
};

layout(binding = 1) uniform sampler2D tex;

layout(location = 0) in vec2 ftcoord;
layout(location = 1) in vec2 fpos;
layout(location = 0) out vec4 outColor;

float sdroundrect(vec2 pt, vec2 ext, float rad) {
    vec2 ext2 = ext - vec2(rad, rad);
    vec2 d = abs(pt) - ext2;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - rad;
}

// Stroke - from [0..1] to clipped pyramid, where the slope is 1px.
float strokeMask() {
    return min(1.0, (1.0 - abs(ftcoord.x * 2.0 - 1.0)) * strokeMultiply) * min(1.0, ftcoord.y);
}

void main(void) {
    vec4 color;
    float strokeAlpha;

    if (edgeAA == 1) {
        strokeAlpha = strokeMask();
        if (strokeAlpha < strokeThreshold) discard;
        strokeAlpha *= qt_Opacity;
    } else {
        strokeAlpha = qt_Opacity;
    }

    if (type == 0) {
        // Color
        color = innerColor * strokeAlpha;
    } else if (type == 1) {
        // Gradient
        vec2 pt = (paintMatrix * vec3(fpos, 1.0)).xy;
        float d = clamp((sdroundrect(pt, extent, radius) + feather * 0.5) / feather, 0.0, 1.0);
        color = mix(innerColor, outerColor, d) * strokeAlpha;
    } else if (type == 2) {
        // ImagePattern
        vec2 pt = (paintMatrix * vec3(fpos, 1.0)).xy / extent;
        color = texture(tex, pt) * innerColor * strokeAlpha;
    } else if (type == 3) {
        // DashPattern
        vec2 pt = vec2((ftcoord.y + dashOffset) / dashUnit, ftcoord.x);
        color = texture(tex, pt) * innerColor * strokeAlpha;
    } else {
        // fallback to Color
        color = innerColor * strokeAlpha;
    }

    outColor = color;
}
