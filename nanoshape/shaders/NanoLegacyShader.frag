uniform highp float qt_Opacity;
uniform highp mat3 paintMatrix;
uniform highp vec4 innerColor;
uniform highp vec4 outerColor;
uniform highp vec2 extent;
uniform highp float radius;
uniform highp float feather;
uniform highp float strokeMultiply;
uniform highp float strokeThreshold;
uniform highp float dashOffset;
uniform highp float dashUnit;
uniform int type;
uniform int edgeAA;

uniform sampler2D tex;

varying highp vec2 ftcoord;
varying highp vec2 fpos;

highp float sdroundrect(highp vec2 pt, highp vec2 ext, highp float rad) {
    highp vec2 ext2 = ext - vec2(rad, rad);
    highp vec2 d = abs(pt) - ext2;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - rad;
}

// Stroke - from [0..1] to clipped pyramid, where the slope is 1px.
highp float strokeMask() {
    return min(1.0, (1.0 - abs(ftcoord.x * 2.0 - 1.0)) * strokeMultiply) * min(1.0, ftcoord.y);
}

void main(void) {
    highp vec4 color;
    highp float strokeAlpha;

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
        highp vec2 pt = (paintMatrix * vec3(fpos, 1.0)).xy;
        highp float d = clamp((sdroundrect(pt, extent, radius) + feather * 0.5) / feather, 0.0, 1.0);
        color = mix(innerColor, outerColor, d) * strokeAlpha;
    } else if (type == 2) {
        // ImagePattern
        highp vec2 pt = (paintMatrix * vec3(fpos, 1.0)).xy / extent;
        color = texture2D(tex, pt) * innerColor * strokeAlpha;
    } else if (type == 3) {
        // DashPattern
        highp vec2 pt = vec2((ftcoord.y + dashOffset) / dashUnit, ftcoord.x);
        color = texture2D(tex, pt) * innerColor * strokeAlpha;
    } else {
        // fallback to Color
        color = innerColor * strokeAlpha;
    }

    gl_FragColor = color;
}
