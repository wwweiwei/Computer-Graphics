#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

out vec3 vertex_color;
out vec3 vertex_normal;
out vec3 fragment_pos; // to fragment shader

uniform mat4 mvp;
uniform mat4 trans;
uniform vec3 view_pos;
uniform mat4 um4p; // projection matrix
uniform mat4 um4v; // camera viewing transformation matrix
uniform mat4 um4r; // rotation matrix

uniform vec3 light_dir_position;
uniform vec3 light_point_position;
uniform vec3 light_spot_position;
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float diffusestrength;
uniform int lightIdx;

vec3 directionalLight(vec3 N, vec3 V){
    float ambientstrength = 0.15f;
    vec3 La = ambientstrength * vec3(1.0,1.0,1.0);
    vec3 Ld = diffusestrength * vec3(1.0f,1.0f,1.0f);
    vec3 Ls = vec3(1.0f,1.0f,1.0f);
    
    vec4 lightInView = um4v * vec4(light_dir_position, 1.0);
    vec3 S = normalize(lightInView.xyz + V);
    vec3 H = normalize(S + V);
    float dc = dot(N,S);
    float sc = pow(max(dot(N, H), 0.0), 64);

    return La * Ka + dc * Ld * Kd + sc * Ls * Ks;
}

vec3 pointLight(vec3 N, vec3 V){
    float ambientstrength = 0.15f;
    vec3 La = ambientstrength * vec3(1.0,1.0,1.0);
    vec3 Ld = diffusestrength * vec3(1.0f,1.0f,1.0f);
    vec3 Ls = vec3(1.0f,1.0f,1.0f);
    
    vec4 lightInView = um4v * vec4(light_point_position, 1.0);
    vec3 S = normalize(lightInView.xyz + V);
    vec3 H = normalize(S + V);
    float dis = length(lightInView.xyz + V);
    float atten = 1.0/(0.01 + (0.8 * dis) + (0.1 * dis * dis));
    float dc = dot(N,S);
    float sc = pow(max(dot(N, H), 0), 64);
    
    return La * Ka + ( dc * Ld * Kd + sc * Ls * Ks) * atten;
}

vec3 spotLight(vec3 N, vec3 V){
    vec3 light_spot_pos = vec3(0, 0, 2);
    vec3 light_spot_dir = vec3(0, 0, -1);
    float exponent = 50;
    float cutoff = 0.524; // 30 degree

    float ambientstrength = 0.15f;
    vec3 La = ambientstrength * vec3(1.0,1.0,1.0);
    vec3 Ld = diffusestrength * vec3(1.0f,1.0f,1.0f);
    vec3 Ls = vec3(1.0f,1.0f,1.0f);
    
    vec4 lightInView = um4v * vec4(light_spot_position, 1.0);
    vec3 S = normalize(lightInView.xyz + V);
    vec3 H = normalize(S + V);
    float dis = length(lightInView.xyz + V);
    float spot = dot(S, -light_spot_dir);
    float coscutoff = cos(cutoff);
    float tmp = 0.0;
    if (spot < coscutoff) tmp=0.0;
    else tmp = pow(spot,exponent);
    float atten = tmp/(0.05 + (0.3 * dis) + (0.6 * dis * dis));
    float dc = dot(N, S);
    float sc = pow(max(dot(N, H), 0), 64);
    return La * Ka + ( dc * Ld * Kd + sc * Ls * Ks) * atten;
}

void main()
{
    // [TODO]
    // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    fragment_pos = vec3(trans * vec4(aPos, 1.0f));
    
    //vertex_normal = aNormal;
    vertex_normal = mat3(transpose(inverse(trans))) * aNormal;
    
    vec3 N = normalize(aNormal);
    vec3 V = view_pos;

    if (lightIdx == 0) { // directional
        vertex_color = directionalLight(N, V);
    } else if (lightIdx == 1) { // point
        vertex_color = pointLight(N, V);
    } else if (lightIdx == 2) { // spot
        vertex_color = spotLight(N ,V);
    }
    
}
