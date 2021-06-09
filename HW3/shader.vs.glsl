#version 330

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

out vec2 texCoord;
out vec3 vertex_color;
out vec3 fragment_pos;
out vec3 vertex_normal;

uniform mat4 um4p;    // projection matrix
uniform mat4 um4v;    // camera viewing transformation matrix
uniform mat4 um4m;    // rotation matrix
uniform mat4 mvp;
uniform mat4 trans;
uniform int iLocisEye; // whether is eye texture
uniform int eye_idx; // current eye idx

void main()
{
    if (iLocisEye == 1){
        if (eye_idx == 0)
            texCoord = vec2(0, 0.75);
        else if (eye_idx == 1)
            texCoord = vec2(0, 0.5);
        else if (eye_idx == 2)
            texCoord = vec2(0, 0.25);
        else if (eye_idx == 3)
            texCoord = vec2(0, 0);
        else if (eye_idx == 4)
            texCoord = vec2(0.5, 0.75);
        else if (eye_idx == 5)
            texCoord = vec2(0.5, 0.5);
        else if (eye_idx == 6)
            texCoord = vec2(0.5, 0.25);
        else if (eye_idx == 7)
            texCoord = vec2(0.5, 0);
    } else {
        texCoord = aTexCoord;
    }
    vertex_color = aColor;
    
    vertex_normal = mat3(transpose(inverse(trans)))*aNormal;
    gl_Position = um4p * um4v * um4m * vec4(aPos, 1.0);
    fragment_pos = vec3(trans * vec4(aPos, 1.0));
}
