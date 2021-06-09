#version 330

in vec2 texCoord;
in vec3 color;
in vec3 vertex_normal;
in vec3 vertex_color;
in vec3 fragment_pos;

out vec4 FragColor;

uniform sampler2D tex0;
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform vec3 light_dir_position;
uniform vec3 view_pos;
uniform float diffusestrength;
uniform int shininess;

// [TODO] passing texture from main.cpp
// Hint: sampler2D

void main() {
    FragColor = vec4(texCoord.xy, 0, 1);

    float attenuation = 1;
    float d;

    //Ambient
    float ambientstrength = 0.15f;
    vec3 La = ambientstrength * vec3(1.0,1.0,1.0);

    //Diffuse
    vec3 Ld = diffusestrength * vec3(1.0f,1.0f,1.0f);
    vec3 norm = normalize(vertex_normal);
    vec3 lightDir = normalize(light_dir_position);
    vec3 reflectDir = normalize(reflect(-1*light_dir_position,norm));
    lightDir = normalize(light_dir_position);
    attenuation = 1;
    reflectDir = normalize(reflect(-1*light_dir_position,norm));
    float diff = max(dot(norm,lightDir),0.0);

    //Specular
    vec3 Ls = vec3(1.0f,1.0f,1.0f);
    vec3 view_dir = normalize(view_pos - fragment_pos);
    float spec = pow(max(dot(view_dir, reflectDir),0.0),shininess);

    vec3 result = (La * Ka + Ld * Kd * diff + Ls * Ks * spec) * attenuation;

    // [TODO] sampleing from texture
    // Hint: texture
    
    vec4 fs_color = vec4(vertex_color, 1);
    vec4 texColor = texture(tex0,texCoord);
    
    FragColor = texColor * vec4(result, 1.0);
}
