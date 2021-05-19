#version 330 core

in vec3 vertex_color;
in vec3 vertex_normal;
in vec3 fragment_pos; // from vertex shader

out vec4 FragColor;

// hw2
uniform vec3 light_dir_position;
uniform vec3 light_point_position;
uniform vec3 light_spot_position;
uniform vec3 view_pos;
uniform int lightIdx;
uniform int shininess;
uniform float diffusestrength;
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform int vertex_or_perpixel;

void main() {
	// [TODO]
	// FragColor = vec4(vertex_normal, 1.0f);
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

    if (lightIdx == 0) { // directional
        lightDir = normalize(light_dir_position);
        attenuation = 1;
        reflectDir = normalize(reflect(-1 * light_dir_position,norm));
    } else if (lightIdx == 1) { // point
        lightDir = normalize(light_point_position - fragment_pos);
        d = length(light_point_position - fragment_pos);
        attenuation = min(1 / (0.01 + 0.8 * d + 0.1 * d * d), 1);
        reflectDir = normalize(reflect( -1 * light_point_position, norm));
    } else { // spot
        lightDir = normalize(light_spot_position - fragment_pos);
        d = length(light_spot_position - fragment_pos);
        attenuation = min(1 / (0.05 + 0.3 * d + 0.6 * d * d), 1);
        reflectDir = normalize(reflect(-1 * light_spot_position, norm));
    }
    float diffuse = max(dot(norm, lightDir), 0.0);

    //Specular
    vec3 Ls = vec3(1.0f,1.0f,1.0f);
    vec3 view_dir = normalize(view_pos - fragment_pos);
    float spec = pow(max(dot(view_dir, reflectDir),0.0),shininess);

    vec3 result = (La * Ka + Ld * Kd * diffuse + Ls * Ks * spec) * attenuation;
    vec4 fragColor = vec4(result, 1.0);

    // vertex or perpixel
    if (vertex_or_perpixel == 0) {
        FragColor = fragColor; // vec4(vertex_color, 1.0);
    } else {
        FragColor = fragColor;
    }
    
}
