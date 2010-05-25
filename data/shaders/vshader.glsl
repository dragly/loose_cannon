attribute highp vec4 vertex;
attribute highp vec4 texCoord;
attribute mediump vec3 normal;
uniform mediump mat4 matrix;
varying highp vec4 texc;
varying mediump float angle;
void main(void)
{
    vec3 toLight = normalize(vec3(0.0, -0.3, 1.0));
    angle = max(dot(normal, toLight), 0.0);
    gl_Position = matrix * vertex;
    texc = texCoord;
}

