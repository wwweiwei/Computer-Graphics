#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include<math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "textfile.h"

#include "Vectors.h"
#include "Matrices.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#ifndef max
# define max(a,b) (((a)>(b))?(a):(b))
# define min(a,b) (((a)<(b))?(a):(b))
#endif

using namespace std;

// Default window size
const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 800;

bool mouse_pressed = false;
int starting_press_x = -1;
int starting_press_y = -1;

int vertex_or_perpixel = 0; // hw2

// Some constant
const float PI = 3.1415926;
int light_idx = 0;
int shininess = 64;

enum TransMode
{
	GeoTranslation = 0,
	GeoRotation = 1,
	GeoScaling = 2,
	ViewCenter = 3,
	ViewEye = 4,
	ViewUp = 5,
    // hw2
    Lighting = 6,
    Light_ED = 7,
    Shininess = 8,
};

// hw2
// GLint iLocMVP;

GLuint iLocMVP;
GLuint iLocTrans;

GLuint iLocV;
GLuint iLocP;
GLuint iLocN;
GLuint iLocR;
GLuint iLocLightIdx;
GLuint iLocKa;
GLuint iLocKd;
GLuint iLocKs;
GLuint iLocShininess;
GLuint iLocdiff_strength;
GLuint iLocview_pos;


vector<string> filenames; // .obj filename list

struct PhongMaterial
{
	Vector3 Ka;
	Vector3 Kd;
	Vector3 Ks;
};

typedef struct
{
	GLuint vao;
	GLuint vbo;
	GLuint vboTex;
	GLuint ebo;
	GLuint p_color;
	int vertex_count;
	GLuint p_normal;
	PhongMaterial material;
	int indexCount;
	GLuint m_texture;
} Shape;

// hw2
struct LightInfo
 {
     Vector3 dir_pos = Vector3(1.0f, 1.0f, 1.0f);
     Vector3 point_pos = Vector3(0.0f, 2.0f, 1.0f);
     Vector3 spot_pos = Vector3(0.0f, 0.0f, 2.0f);
     float diffstrength = 1.0;
     float cutoff = 30.0 ;
 }lightInfo;

struct model
{
	Vector3 position = Vector3(0, 0, 0);
	Vector3 scale = Vector3(1, 1, 1);
	Vector3 rotation = Vector3(0, 0, 0);	// Euler form

	vector<Shape> shapes;

    // hw2
    LightInfo lightInfos;
    // Matrix4 normal;
};
vector<model> models;

// hw2
struct iLocLightInfo
{
     GLuint dir_pos;
     GLuint point_pos;
     GLuint spot_pos;
     //GLuint diffstrength;
     GLuint cutoff;
 }iLocLightInfo;

struct camera
{
	Vector3 position;
	Vector3 center;
	Vector3 up_vector;
};
camera main_camera;

struct project_setting
{
	GLfloat nearClip, farClip;
	GLfloat fovy;
	GLfloat aspect;
	GLfloat left, right, top, bottom;
};
project_setting proj;

enum ProjMode
{
	Orthogonal = 0,
	Perspective = 1,
};
ProjMode cur_proj_mode = Orthogonal;
TransMode cur_trans_mode = GeoTranslation;

Matrix4 view_matrix;
Matrix4 project_matrix;
// hw2
double current_x;
double current_y;

Shape quad;
Shape m_shpae;
int cur_idx = 0; // represent which model should be rendered now

//Matrix4 V = Matrix4(
//    1, 0, 0, 0,
//    0, 1, 0, 0,
//    0, 0, 1, 0,
//    0, 0, 0, 1);
//
//Matrix4 P = Matrix4(
//    1, 0, 0, 0,
//    0, 1, 0, 0,
//    0, 0, -1, 0,
//    0, 0, 0, 1);
//
//Matrix4 M = Matrix4(
//    1, 0, 0, 0,
//    0, 1, 0, 0,
//    0, 0, -1, 0,
//    0, 0, 0, 1);


static GLvoid Normalize(GLfloat v[3])
{
	GLfloat l;

	l = (GLfloat)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= l;
	v[1] /= l;
	v[2] /= l;
}

static GLvoid Cross(GLfloat u[3], GLfloat v[3], GLfloat n[3])
{

	n[0] = u[1] * v[2] - u[2] * v[1];
	n[1] = u[2] * v[0] - u[0] * v[2];
	n[2] = u[0] * v[1] - u[1] * v[0];
}


// [TODO] given a translation vector then output a Matrix4 (Translation Matrix)
Matrix4 translate(Vector3 vec)
{
	Matrix4 mat;

	/*
	mat = Matrix4(
		...
	);
	*/

    mat = Matrix4(1, 0, 0, vec.x,
        0, 1, 0, vec.y,
        0, 0, 1, vec.z,
        0, 0, 0, 1);

	return mat;
}

// [TODO] given a scaling vector then output a Matrix4 (Scaling Matrix)
Matrix4 scaling(Vector3 vec)
{
	Matrix4 mat;

	/*
	mat = Matrix4(
		...
	);
	*/

    mat = Matrix4(vec.x, 0, 0, 0,
        0, vec.y, 0, 0,
        0, 0, vec.z, 0,
        0, 0, 0, 1);

	return mat;
}


// [TODO] given a float value then ouput a rotation matrix alone axis-X (rotate alone axis-X)
Matrix4 rotateX(GLfloat val)
{
	Matrix4 mat;

	/*
	mat = Matrix4(
		...
	);
	*/

    GLfloat c = cosf(val / 180.0 * PI);
    GLfloat s = sinf(val / 180.0 * PI);
    mat = Matrix4(1, 0, 0, 0,
                    0, c, -s, 0,
                  0, s, c, 0,
                  0, 0, 0, 1);

	return mat;
}

// [TODO] given a float value then ouput a rotation matrix alone axis-Y (rotate alone axis-Y)
Matrix4 rotateY(GLfloat val)
{
	Matrix4 mat;

	/*
	mat = Matrix4(
		...
	);
	*/

    GLfloat c = cosf(val / 180.0 * PI);
    GLfloat s = sinf(val / 180.0 * PI);
    mat = Matrix4(c, 0, s, 0,
                  0, 1, 0, 0,
                  -s, 0, c, 0,
                  0, 0, 0, 1);

	return mat;
}

// [TODO] given a float value then ouput a rotation matrix alone axis-Z (rotate alone axis-Z)
Matrix4 rotateZ(GLfloat val)
{
	Matrix4 mat;

	/*
	mat = Matrix4(
		...
	);
	*/

    GLfloat c = cosf(val / 180.0 * PI);
    GLfloat s = sinf(val / 180.0 * PI);
    mat = Matrix4(c, -s, 0, 0,
                  s, c, 0, 0,
                  0, 0, 1, 0,
                  0, 0, 0, 1);

	return mat;
}

Matrix4 rotate(Vector3 vec)
{
	return rotateX(vec.x)*rotateY(vec.y)*rotateZ(vec.z);
}

// [TODO] compute viewing matrix accroding to the setting of main_camera
void setViewingMatrix()
{
	// view_matrix[...] = ...
    GLfloat eyex = main_camera.position.x;
    GLfloat eyey = main_camera.position.y;
    GLfloat eyez = main_camera.position.z;

    GLfloat cenx = main_camera.center.x;
    GLfloat ceny = main_camera.center.y;
    GLfloat cenz = main_camera.center.z;

    GLfloat upx = main_camera.up_vector.x;
    GLfloat upy = main_camera.up_vector.y;
    GLfloat upz = main_camera.up_vector.z;
    GLfloat f[3] = { cenx - eyex,ceny - eyey,cenz - eyez };

    GLfloat S[3];
    GLfloat u[3] = {upx, upy, upz};
    GLfloat uu[3];
    Normalize(u);
    Normalize(f);
    Cross(f,u,S);
    Cross(S,f,uu);
    Normalize(uu);

    view_matrix = { S[0], S[1], S[2], -eyex * S[0] - eyey * S[1] - eyez * S[2],
                   uu[0], uu[1], uu[2], -eyex * uu[0] - eyey * uu[1] - eyez * uu[2],
                   -1 * f[0], -1 * f[1], -1 * f[2], eyex * f[0] + eyey * f[1] + eyez * f[2],
                   0,0,0,1 };

}

// [TODO] compute orthogonal projection matrix
void setOrthogonal()
{
	cur_proj_mode = Orthogonal;
	// project_matrix [...] = ...
    glEnable(GL_DEPTH_TEST);
    project_matrix = { 2 / (proj.right - proj.left),0,0,-1*(proj.right + proj.left) / (proj.right - proj.left),
    0,2 / (proj.top - proj.bottom),0,-1*(proj.top + proj.bottom) / (proj.top - proj.bottom),
    0,0,-2 / (proj.farClip - proj.nearClip),-1 * (proj.farClip + proj.nearClip) / (proj.farClip - proj.nearClip),
    0,0,0,1 };
}

// [TODO] compute persepective projection matrix
void setPerspective()
{
	cur_proj_mode = Perspective;
	// project_matrix [...] = ...
    GLfloat f = cos((proj.fovy/2) /180.0 * PI) / sin((proj.fovy/2) /  180.0 * PI);
    project_matrix = { f /proj.aspect,0,0,0,
    0,  f, 0,0,
    0,0,-1*(proj.farClip + proj.nearClip) / (proj.farClip - proj.nearClip),-2 * (proj.farClip * proj.nearClip) / (proj.farClip - proj.nearClip),
    0,0,-1,0 };
}


// Vertex buffers
GLuint VAO, VBO;

// Call back function for window reshape
//void ChangeSize(GLFWwindow* window, int width, int height, int i, int j)
void ChangeSize(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	// glViewport(i, j, width, height);
	// [TODO] change your aspect ratio
    proj.aspect = float(width)/float(height);;
    if (cur_proj_mode == Perspective){
        GLfloat f = cos((proj.fovy/2) /180.0 * PI) / sin((proj.fovy/2) /  180.0 * PI);
        project_matrix = { f /proj.aspect,0,0,0,
        0,  f, 0,0,
        0,0,-1*(proj.farClip + proj.nearClip) / (proj.farClip - proj.nearClip),-2 * (proj.farClip * proj.nearClip) / (proj.farClip - proj.nearClip),
        0,0,-1,0 };
    }
}

// Render function for display rendering
void RenderScene(void) {
	// clear canvas
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	Matrix4 T, R, S;
	// [TODO] update translation, rotation and scaling

    T = translate(models[cur_idx].position);
    R = rotate(models[cur_idx].rotation);
    S = scaling(models[cur_idx].scale);

	Matrix4 MVP;
	GLfloat mvp[16];

    Matrix4 Trans;
    GLfloat trans[16];


	// [TODO] multiply all the matrix
    MVP = project_matrix * view_matrix * S * T * R;
    Trans = S * T * R;
    
	// [TODO] row-major ---> column-major
	mvp[0] = 1;  mvp[4] = 0;   mvp[8] = 0;    mvp[12] = 0;
	mvp[1] = 0;  mvp[5] = 1;   mvp[9] = 0;    mvp[13] = 0;
	mvp[2] = 0;  mvp[6] = 0;   mvp[10] = 1;   mvp[14] = 0;
	mvp[3] = 0; mvp[7] = 0;  mvp[11] = 0;   mvp[15] = 1;

    mvp[0] = MVP[0];  mvp[4] = MVP[1];   mvp[8] = MVP[2];    mvp[12] = MVP[3];
    mvp[1] = MVP[4];  mvp[5] = MVP[5];   mvp[9] = MVP[6];    mvp[13] = MVP[7];
    mvp[2] = MVP[8];  mvp[6] = MVP[9];   mvp[10] = MVP[10];   mvp[14] = MVP[11];
    mvp[3] = MVP[12]; mvp[7] = MVP[13];  mvp[11] = MVP[14];   mvp[15] = MVP[15];

    trans[0] = Trans[0];  trans[4] = Trans[1];   trans[8] = Trans[2];    trans[12] = Trans[3];
    trans[1] = Trans[4];  trans[5] = Trans[5];   trans[9] = Trans[6];    trans[13] = Trans[7];
    trans[2] = Trans[8];  trans[6] = Trans[9];   trans[10] = Trans[10];   trans[14] = Trans[11];
    trans[3] = Trans[12]; trans[7] = Trans[13];  trans[11] = Trans[14];   trans[15] = Trans[15];

    glViewport(0, 0, WINDOW_WIDTH/2, WINDOW_HEIGHT);
    glUniform1i(vertex_or_perpixel, 0); // per vertex
	//ChangeSize(window, WINDOW_WIDTH/2, WINDOW_HEIGHT, 0, 0);

    glUniformMatrix4fv(iLocP, 1, GL_FALSE, project_matrix.getTranspose());
    glUniformMatrix4fv(iLocV, 1, GL_FALSE, view_matrix.getTranspose());
    // glUniformMatrix4fv(iLocN, 1, GL_FALSE, N.getTranspose());
    glUniformMatrix4fv(iLocR, 1, GL_FALSE, R.getTranspose());
    
	// hw2
    glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, mvp);
    glUniformMatrix4fv(iLocTrans, 1, GL_FALSE, trans);
    glUniform1f(iLocdiff_strength, models[cur_idx].lightInfos.diffstrength);
    glUniform1i(iLocShininess, shininess);
    glUniform1i(iLocLightIdx, light_idx);

    glUniform3fv(iLocview_pos, 1, &(main_camera.position[0]));
    glUniform3fv(iLocLightInfo.dir_pos, 1, &(models[cur_idx].lightInfos.dir_pos[0]));
    glUniform3fv(iLocLightInfo.point_pos, 1, &(models[cur_idx].lightInfos.point_pos[0]));
    glUniform3fv(iLocLightInfo.spot_pos, 1, &(models[cur_idx].lightInfos.spot_pos[0]));

    for (int i = 0; i < models[cur_idx].shapes.size(); i++)
    {
        // hw2
        glUniform3fv(iLocKa, 1, &(models[cur_idx].shapes[i].material.Ka[0]));
        glUniform3fv(iLocKd, 1, &(models[cur_idx].shapes[i].material.Kd[0]));
        glUniform3fv(iLocKs, 1, &(models[cur_idx].shapes[i].material.Ks[0]));

        glBindVertexArray(models[cur_idx].shapes[i].vao);
        glDrawArrays(GL_TRIANGLES, 0, models[cur_idx].shapes[i].vertex_count);

    }

	glEnd();
    
    
	//ChangeSize(window, WINDOW_WIDTH/2, WINDOW_HEIGHT, WINDOW_WIDTH/2, 0);
	glViewport(WINDOW_WIDTH/2, 0, WINDOW_WIDTH/2, WINDOW_HEIGHT);
    glUniform1i(vertex_or_perpixel, 1); // per pixel
	// hw2
    glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, mvp);
    glUniformMatrix4fv(iLocTrans, 1, GL_FALSE, trans);
    glUniform1f(iLocdiff_strength, models[cur_idx].lightInfos.diffstrength);
    glUniform1i(iLocShininess, shininess);
    glUniform1i(iLocLightIdx, light_idx);

    glUniform3fv(iLocview_pos, 1, &(main_camera.position[0]));
    glUniform3fv(iLocLightInfo.dir_pos, 1, &(models[cur_idx].lightInfos.dir_pos[0]));
    glUniform3fv(iLocLightInfo.point_pos, 1, &(models[cur_idx].lightInfos.point_pos[0]));
    glUniform3fv(iLocLightInfo.spot_pos, 1, &(models[cur_idx].lightInfos.spot_pos[0]));

    for (int i = 0; i < models[cur_idx].shapes.size(); i++)
    {
        // hw2
        glUniform3fv(iLocKa, 1, &(models[cur_idx].shapes[i].material.Ka[0]));
        glUniform3fv(iLocKd, 1, &(models[cur_idx].shapes[i].material.Kd[0]));
        glUniform3fv(iLocKs, 1, &(models[cur_idx].shapes[i].material.Ks[0]));

        glBindVertexArray(models[cur_idx].shapes[i].vao);
        glDrawArrays(GL_TRIANGLES, 0, models[cur_idx].shapes[i].vertex_count);

    }
	glEnd();
    /*
	// use uniform to send mvp to vertex shader
	glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, mvp);
	for (int i = 0; i < models[cur_idx].shapes.size(); i++)
	{
		glBindVertexArray(models[cur_idx].shapes[i].vao);
		glDrawArrays(GL_TRIANGLES, 0, models[cur_idx].shapes[i].vertex_count);
	}
     */

}


void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// [TODO] Call back function for keyboard
    // switch the model
    if (key == GLFW_KEY_X && action == GLFW_PRESS){
        if(cur_idx == 0)
            cur_idx = 4;
        else
            cur_idx -=1;
    }
    else if(key == GLFW_KEY_Z && action == GLFW_PRESS){
        if(cur_idx == 4)
            cur_idx = 0;
        else
            cur_idx +=1;
    }
    // switch to translation mode
    else if(key == GLFW_KEY_T && action == GLFW_PRESS){
        cur_trans_mode = GeoTranslation;
    }
    // switch to scale mode
    else if(key == GLFW_KEY_S && action == GLFW_PRESS){
        cur_trans_mode = GeoScaling;
    }
    // switch to rotation mode
    else if(key == GLFW_KEY_R && action == GLFW_PRESS){
        cur_trans_mode = GeoRotation;
    }
    // switch between directional/point/spot light
    else if(key == GLFW_KEY_L && action == GLFW_PRESS){
        // hw2
        if(light_idx ==2){
            light_idx = 0;
        } else {
            light_idx ++;
        }
    }
    // switch to light editing mode
    else if(key == GLFW_KEY_K && action == GLFW_PRESS){
        cur_trans_mode = Light_ED;
    }
    // switch to shininess editing mode
    else if(key == GLFW_KEY_J && action == GLFW_PRESS){
        cur_trans_mode = Shininess;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// [TODO] scroll up positive, otherwise it would be negtive
    if (yoffset > 0) {
        switch (cur_trans_mode) {
            case ViewEye:
                main_camera.position.z += 0.5;
                setViewingMatrix();
                break;
            case ViewCenter:
                main_camera.center.z += 0.5;
                setViewingMatrix();
                break;
            case ViewUp:
                main_camera.up_vector.z += 0.5;
                setViewingMatrix();
                break;
            case GeoTranslation:
                models[cur_idx].position.z += 0.01;
                break;
            case GeoScaling:
                models[cur_idx].scale.z += 0.01;
                break;
            case GeoRotation:
                models[cur_idx].rotation.z += 1;
                break;

            case Light_ED:
                if (light_idx ==0 || light_idx ==1){
                    models[cur_idx].lightInfos.diffstrength += 0.01;
                } else if (light_idx ==2){
                    models[cur_idx].lightInfos.cutoff += 0.005;
                }
                break;
            case Shininess:
                shininess += 1;
                break;

        }
    } else {
        switch (cur_trans_mode)
        {
            case ViewEye:
                main_camera.position.z -= 0.5;
                setViewingMatrix();
                break;
            case ViewCenter:
                main_camera.center.z -= 0.5;
                setViewingMatrix();
                break;
            case ViewUp:
                main_camera.up_vector.z -= 0.5;
                setViewingMatrix();
                break;
            case GeoTranslation:
                models[cur_idx].position.z -= 0.01;
                break;
            case GeoScaling:
                models[cur_idx].scale.z -= 0.01;
                break;
            case GeoRotation:
                models[cur_idx].rotation.z -= 1;
                break;
            case Light_ED:
                if (light_idx ==0 || light_idx ==1){
                    models[cur_idx].lightInfos.diffstrength -= 0.01;
                } else if (light_idx ==2){
                    models[cur_idx].lightInfos.cutoff -= 0.005;
                }
                break;
            case Shininess:
                if(shininess > 10){
                    shininess -= 0.01;//
                }
                break;
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// [TODO] mouse press callback function
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        mouse_pressed = true;
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        current_x = xpos;
        current_y = ypos;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
        mouse_pressed = false;
    }
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	// [TODO] cursor position callback function
    if (mouse_pressed){
        double diff_x = xpos - current_x;
        double diff_y = ypos - current_y;
        current_x = xpos;
        current_y = ypos;
        switch (cur_trans_mode) {
        case ViewEye:
            main_camera.position.x += diff_x/50.0;
            main_camera.position.y += diff_y/50.0;
            setViewingMatrix();
            break;
        case ViewCenter:
            main_camera.center.x += diff_x / 50.0;
            main_camera.center.y += diff_y / 50.0;
            setViewingMatrix();
            break;
        case ViewUp:
            main_camera.up_vector.x += diff_x / 50.0;
            main_camera.up_vector.y += diff_y / 50.0;
            setViewingMatrix();
            break;
        case GeoTranslation:
            models[cur_idx].position.x += diff_x / 200.0;
            models[cur_idx].position.y -= diff_y / 200.0;
            break;
        case GeoScaling:
            models[cur_idx].scale.x += diff_x / 200.0;
            models[cur_idx].scale.y += diff_y / 200.0;
            break;
        case GeoRotation:
            models[cur_idx].rotation.x += diff_y ;
            models[cur_idx].rotation.y += diff_x ;
            break;
        case Light_ED:
            if(light_idx == 0){
                models[cur_idx].lightInfos.dir_pos.x += diff_x/50.0;
                models[cur_idx].lightInfos.dir_pos.y -= diff_y/50.0;
            } else if(light_idx ==1){
                models[cur_idx].lightInfos.point_pos.x += diff_x/50.0;
                models[cur_idx].lightInfos.point_pos.y -= diff_y/50.0;
            } else if(light_idx ==2 ){
                models[cur_idx].lightInfos.spot_pos.x += diff_x/50.0;
                models[cur_idx].lightInfos.spot_pos.y -= diff_y/50.0;
            }
            break;
        }
    }
}

void setShaders()
{
	GLuint v, f, p;
	char *vs = NULL;
	char *fs = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead("shader.vs");
	fs = textFileRead("shader.fs");

	glShaderSource(v, 1, (const GLchar**)&vs, NULL);
	glShaderSource(f, 1, (const GLchar**)&fs, NULL);

	free(vs);
	free(fs);

	GLint success;
	char infoLog[1000];
	// compile vertex shader
	glCompileShader(v);
	// check for shader compile errors
	glGetShaderiv(v, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(v, 1000, NULL, infoLog);
		std::cout << "ERROR: VERTEX SHADER COMPILATION FAILED\n" << infoLog << std::endl;
	}

	// compile fragment shader
	glCompileShader(f);
	// check for shader compile errors
	glGetShaderiv(f, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(f, 1000, NULL, infoLog);
		std::cout << "ERROR: FRAGMENT SHADER COMPILATION FAILED\n" << infoLog << std::endl;
	}

	// create program object
	p = glCreateProgram();

	// attach shaders to program object
	glAttachShader(p,f);
	glAttachShader(p,v);

	// link program
	glLinkProgram(p);
	// check for linking errors
	glGetProgramiv(p, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(p, 1000, NULL, infoLog);
		std::cout << "ERROR: SHADER PROGRAM LINKING FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(v);
	glDeleteShader(f);

	iLocMVP = glGetUniformLocation(p, "mvp");
    
    vertex_or_perpixel = glGetUniformLocation(p, "vertex_or_perpixel");
    
    //hw2
    iLocTrans = glGetUniformLocation(p,"trans");
    iLocP = glGetUniformLocation(p, "um4p");
    iLocV = glGetUniformLocation(p, "um4v");
    
    // iLocN = glGetUniformLocation(p, "um4n");
    iLocR = glGetUniformLocation(p, "um4r");
    
    iLocLightIdx = glGetUniformLocation(p, "lightIdx");
    iLocKa = glGetUniformLocation(p, "Ka");
    iLocKd = glGetUniformLocation(p, "Kd");
    iLocKs = glGetUniformLocation(p, "Ks");
    iLocview_pos = glGetUniformLocation(p, "view_pos");
    iLocShininess =glGetUniformLocation(p, "shininess");

    iLocLightInfo.dir_pos = glGetUniformLocation(p, "light_dir_position");
    iLocLightInfo.point_pos = glGetUniformLocation(p, "light_point_position");
    iLocLightInfo.spot_pos = glGetUniformLocation(p, "light_spot_position");
    iLocLightInfo.cutoff = glGetUniformLocation(p, "spotCutoff");
    iLocdiff_strength =glGetUniformLocation(p, "diffusestrength");

	if (success)
		glUseProgram(p);
    else {
        system("pause");
        exit(123);
    }
}

void normalization(tinyobj::attrib_t* attrib, vector<GLfloat>& vertices, vector<GLfloat>& colors, vector<GLfloat>& normals, tinyobj::shape_t* shape)
{
	vector<float> xVector, yVector, zVector;
	float minX = 10000, maxX = -10000, minY = 10000, maxY = -10000, minZ = 10000, maxZ = -10000;

	// find out min and max value of X, Y and Z axis
	for (int i = 0; i < attrib->vertices.size(); i++)
	{
		//maxs = max(maxs, attrib->vertices.at(i));
		if (i % 3 == 0)
		{

			xVector.push_back(attrib->vertices.at(i));

			if (attrib->vertices.at(i) < minX)
			{
				minX = attrib->vertices.at(i);
			}

			if (attrib->vertices.at(i) > maxX)
			{
				maxX = attrib->vertices.at(i);
			}
		}
		else if (i % 3 == 1)
		{
			yVector.push_back(attrib->vertices.at(i));

			if (attrib->vertices.at(i) < minY)
			{
				minY = attrib->vertices.at(i);
			}

			if (attrib->vertices.at(i) > maxY)
			{
				maxY = attrib->vertices.at(i);
			}
		}
		else if (i % 3 == 2)
		{
			zVector.push_back(attrib->vertices.at(i));

			if (attrib->vertices.at(i) < minZ)
			{
				minZ = attrib->vertices.at(i);
			}

			if (attrib->vertices.at(i) > maxZ)
			{
				maxZ = attrib->vertices.at(i);
			}
		}
	}

	float offsetX = (maxX + minX) / 2;
	float offsetY = (maxY + minY) / 2;
	float offsetZ = (maxZ + minZ) / 2;

	for (int i = 0; i < attrib->vertices.size(); i++)
	{
		if (offsetX != 0 && i % 3 == 0)
		{
			attrib->vertices.at(i) = attrib->vertices.at(i) - offsetX;
		}
		else if (offsetY != 0 && i % 3 == 1)
		{
			attrib->vertices.at(i) = attrib->vertices.at(i) - offsetY;
		}
		else if (offsetZ != 0 && i % 3 == 2)
		{
			attrib->vertices.at(i) = attrib->vertices.at(i) - offsetZ;
		}
	}

	float greatestAxis = maxX - minX;
	float distanceOfYAxis = maxY - minY;
	float distanceOfZAxis = maxZ - minZ;

	if (distanceOfYAxis > greatestAxis)
	{
		greatestAxis = distanceOfYAxis;
	}

	if (distanceOfZAxis > greatestAxis)
	{
		greatestAxis = distanceOfZAxis;
	}

	float scale = greatestAxis / 2;

	for (int i = 0; i < attrib->vertices.size(); i++)
	{
		//std::cout << i << " = " << (double)(attrib.vertices.at(i) / greatestAxis) << std::endl;
		attrib->vertices.at(i) = attrib->vertices.at(i) / scale;
	}
	size_t index_offset = 0;
	for (size_t f = 0; f < shape->mesh.num_face_vertices.size(); f++) {
		int fv = shape->mesh.num_face_vertices[f];

		// Loop over vertices in the face.
		for (size_t v = 0; v < fv; v++) {
			// access to vertex
			tinyobj::index_t idx = shape->mesh.indices[index_offset + v];
			vertices.push_back(attrib->vertices[3 * idx.vertex_index + 0]);
			vertices.push_back(attrib->vertices[3 * idx.vertex_index + 1]);
			vertices.push_back(attrib->vertices[3 * idx.vertex_index + 2]);
			// Optional: vertex colors
			colors.push_back(attrib->colors[3 * idx.vertex_index + 0]);
			colors.push_back(attrib->colors[3 * idx.vertex_index + 1]);
			colors.push_back(attrib->colors[3 * idx.vertex_index + 2]);
			// Optional: vertex normals
			if (idx.normal_index >= 0) {
				normals.push_back(attrib->normals[3 * idx.normal_index + 0]);
				normals.push_back(attrib->normals[3 * idx.normal_index + 1]);
				normals.push_back(attrib->normals[3 * idx.normal_index + 2]);
			}
		}
		index_offset += fv;
	}
}

string GetBaseDir(const string& filepath) {
	if (filepath.find_last_of("/\\") != std::string::npos)
		return filepath.substr(0, filepath.find_last_of("/\\"));
	return "";
}

void LoadModels(string model_path)
{
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	tinyobj::attrib_t attrib;
	vector<GLfloat> vertices;
	vector<GLfloat> colors;
	vector<GLfloat> normals;

	string err;
	string warn;

	string base_dir = GetBaseDir(model_path); // handle .mtl with relative path

#ifdef _WIN32
	base_dir += "\\";
#else
	base_dir += "/";
#endif

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path.c_str(), base_dir.c_str());

	if (!warn.empty()) {
		cout << warn << std::endl;
	}

	if (!err.empty()) {
		cerr << err << std::endl;
	}

	if (!ret) {
		exit(1);
	}

	printf("Load Models Success ! Shapes size %d Material size %d\n", shapes.size(), materials.size());
	model tmp_model;

	vector<PhongMaterial> allMaterial;
	for (int i = 0; i < materials.size(); i++)
	{
		PhongMaterial material;
		material.Ka = Vector3(materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]);
		material.Kd = Vector3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
		material.Ks = Vector3(materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
		allMaterial.push_back(material);
	}

	for (int i = 0; i < shapes.size(); i++)
	{

		vertices.clear();
		colors.clear();
		normals.clear();
		normalization(&attrib, vertices, colors, normals, &shapes[i]);
		// printf("Vertices size: %d", vertices.size() / 3);

		Shape tmp_shape;
		glGenVertexArrays(1, &tmp_shape.vao);
		glBindVertexArray(tmp_shape.vao);

		glGenBuffers(1, &tmp_shape.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GL_FLOAT), &vertices.at(0), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		tmp_shape.vertex_count = vertices.size() / 3;

		glGenBuffers(1, &tmp_shape.p_color);
		glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.p_color);
		glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GL_FLOAT), &colors.at(0), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &tmp_shape.p_normal);
		glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.p_normal);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GL_FLOAT), &normals.at(0), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		// not support per face material, use material of first face
		if (allMaterial.size() > 0)
			tmp_shape.material = allMaterial[shapes[i].mesh.material_ids[0]];
		tmp_model.shapes.push_back(tmp_shape);
	}
	shapes.clear();
	materials.clear();
	models.push_back(tmp_model);
}

void initParameter()
{
	proj.left = -1;
	proj.right = 1;
	proj.top = 1;
	proj.bottom = -1;
	proj.nearClip = 0.001;
	proj.farClip = 100.0;
	proj.fovy = 80;
	proj.aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

	main_camera.position = Vector3(0.0f, 0.0f, 2.0f);
	main_camera.center = Vector3(0.0f, 0.0f, 0.0f);
	main_camera.up_vector = Vector3(0.0f, 1.0f, 0.0f);

	setViewingMatrix();
	setPerspective();	//set default projection matrix as perspective matrix
}

void setupRC()
{
	// setup shaders
	setShaders();
	initParameter();

	// OpenGL States and Values
	glClearColor(0.2, 0.2, 0.2, 1.0);
	vector<string> model_list{ "../NormalModels/bunny5KN.obj", "../NormalModels/dragon10KN.obj", "../NormalModels/lucy25KN.obj", "../NormalModels/teapot4KN.obj", "../NormalModels/dolphinN.obj"};
	
    // [TODO] Load five model at here
	LoadModels(model_list[cur_idx]);
    LoadModels(model_list[cur_idx+1]);
    LoadModels(model_list[cur_idx+2]);
    LoadModels(model_list[cur_idx+3]);
    LoadModels(model_list[cur_idx+4]);
}

void glPrintContextInfo(bool printExtension)
{
	cout << "GL_VENDOR = " << (const char*)glGetString(GL_VENDOR) << endl;
	cout << "GL_RENDERER = " << (const char*)glGetString(GL_RENDERER) << endl;
	cout << "GL_VERSION = " << (const char*)glGetString(GL_VERSION) << endl;
	cout << "GL_SHADING_LANGUAGE_VERSION = " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	if (printExtension)
	{
		GLint numExt;
		glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
		cout << "GL_EXTENSIONS =" << endl;
		for (GLint i = 0; i < numExt; i++)
		{
			cout << "\t" << (const char*)glGetStringi(GL_EXTENSIONS, i) << endl;
		}
	}
}


int main(int argc, char **argv)
{
    // initial glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // fix compilation on OS X
#endif

    // create window
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "106070038 HW2", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);


    // load OpenGL function pointer
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

	// register glfw callback functions
    glfwSetKeyCallback(window, KeyCallback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);

    glfwSetFramebufferSizeCallback(window, ChangeSize);
	glEnable(GL_DEPTH_TEST);
	// Setup render context
	setupRC();

	// main loop
    while (!glfwWindowShouldClose(window))
    {
        // render
        RenderScene();

        // swap buffer from back to front
        glfwSwapBuffers(window);

        // Poll input event
        glfwPollEvents();
    }

	// just for compatibiliy purposes
	return 0;
}
