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
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;

bool isDrawWireframe = false;
bool mouse_pressed = false;
int starting_press_x = -1;
int starting_press_y = -1;

// Ass1
const float PI = 3.1415926;
void glPrintContextInfo(bool printExtension);

enum TransMode
{
	GeoTranslation = 0,
	GeoRotation = 1,
	GeoScaling = 2,
	ViewCenter = 3,
	ViewEye = 4,
	ViewUp = 5,
};

GLint iLocMVP;

vector<string> filenames; // .obj filename list

struct model
{
	Vector3 position = Vector3(0, 0, 0);
	Vector3 scale = Vector3(1, 1, 1);
	Vector3 rotation = Vector3(0, 0, 0);	// Euler form
};
vector<model> models;

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
// Ass1
double current_x;
double current_y;

typedef struct
{
	GLuint vao;
	GLuint vbo;
	GLuint vboTex;
	GLuint ebo;
	GLuint p_color;
	int vertex_count;
	GLuint p_normal;
	int materialId;
	int indexCount;
	GLuint m_texture;
} Shape;
Shape quad;
Shape m_shpae;
vector<Shape> m_shape_list;
int cur_idx = 0; // represent which model should be rendered now


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
	//cout << cenx << endl;
	GLfloat upx = main_camera.up_vector.x;
	GLfloat upy = main_camera.up_vector.y;
	GLfloat upz = main_camera.up_vector.z;
	GLfloat f[3] = { cenx - eyex,ceny - eyey,cenz - eyez };

	GLfloat S[3];
	GLfloat u[3] = { upx, upy, upz };
	GLfloat uu[3];
	Normalize(u); // u'/|u|

	Normalize(f);//f'/|f|
	Cross(f, u, S);
	Cross(S, f, uu);
	Normalize(uu);

	view_matrix = { S[0],S[1],S[2],-eyex * S[0] - eyey * S[1] - eyez * S[2],
				   uu[0],uu[1],uu[2],-eyex * uu[0] - eyey * uu[1] - eyez * uu[2],
				   -1 * f[0],-1 * f[1],-1 * f[2],eyex * f[0] + eyey * f[1] + eyez * f[2],
				   0,0,0,1 };

}

// [TODO] compute orthogonal projection matrix
void setOrthogonal()
{
	cur_proj_mode = Orthogonal;
	glEnable(GL_DEPTH_TEST);
	// project_matrix [...] = ...
	project_matrix = { 2 / (proj.right - proj.left),0,0,-1 * (proj.right + proj.left) / (proj.right - proj.left),
	0,2 / (proj.top - proj.bottom),0,-1 * (proj.top + proj.bottom) / (proj.top - proj.bottom),
	0,0,-2 / (proj.farClip - proj.nearClip),-1 * (proj.farClip + proj.nearClip) / (proj.farClip - proj.nearClip),
	0,0,0,1 };

}

// [TODO] compute persepective projection matrix
void setPerspective()
{
	cur_proj_mode = Perspective;
	// project_matrix [...] = ...
	GLfloat f = cos((proj.fovy / 2) / 180.0 * PI) / sin((proj.fovy / 2) / 180.0 * PI);
	project_matrix = { f / proj.aspect,0,0,0,
	0,  f, 0,0,
	0,0,-1 * (proj.farClip + proj.nearClip) / (proj.farClip - proj.nearClip),-2 * (proj.farClip * proj.nearClip) / (proj.farClip - proj.nearClip),
	0,0,-1,0 };
}

// Vertex buffers
GLuint VAO, VBO;

// Call back function for window reshape
void ChangeSize(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	// [TODO] change your aspect ratio in both perspective and orthogonal view
	// cout << "width / height: " << width << "," << height << endl;
	proj.aspect = width / height;
	float ratio =  width / height;

	if (cur_proj_mode == Perspective) {
		GLfloat f = cos((proj.fovy / 2) / 180.0 * PI) / sin((proj.fovy / 2) / 180.0 * PI);
		project_matrix = { f / proj.aspect,0,0,0,
			0,f,0,0,
			0,0,-1 * (proj.farClip + proj.nearClip) / (proj.farClip - proj.nearClip),-2 * (proj.farClip * proj.nearClip) / (proj.farClip - proj.nearClip),
			0,0,-1,0 };
	} else if (cur_proj_mode == Orthogonal) {
		project_matrix = { 2 / (proj.right - proj.left) *1 , 0, 0, -1 * (proj.right + proj.left) / (proj.right - proj.left),
			0,2 / (proj.top - proj.bottom) * 1 ,0,-1 * (proj.top + proj.bottom) / (proj.top - proj.bottom),
			0,0,-2 / (proj.farClip - proj.nearClip),-1 * (proj.farClip + proj.nearClip) / (proj.farClip - proj.nearClip),
			0,0,0,1 };
	}
	
}

void drawPlane()
{
	// solid
	GLint polygonMode[2];
	glGetIntegerv(GL_POLYGON_MODE, polygonMode);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//

	GLfloat vertices[18]{ 1.0, -0.9, -1.0,
		1.0, -0.9,  1.0,
		-1.0, -0.9, -1.0,
		1.0, -0.9,  1.0,
		-1.0, -0.9,  1.0,
		-1.0, -0.9, -1.0 };

	GLfloat colors[18]{ 0.0,1.0,0.0,
		0.0,0.5,0.8,
		0.0,1.0,0.0,
		0.0,0.5,0.8,
		0.0,0.5,0.8,
		0.0,1.0,0.0 };


	// [TODO] draw the plane with above vertices and color
	Matrix4 MVP;
	MVP = project_matrix * view_matrix;
	GLfloat mvp[16];
	// [TODO] multiply all the matrix
	// [TODO] row-major ---> column-major
	mvp[0] = MVP[0];  mvp[4] = MVP[1];   mvp[8] = MVP[2];    mvp[12] = MVP[3];
	mvp[1] = MVP[4];  mvp[5] = MVP[5];   mvp[9] = MVP[6];    mvp[13] = MVP[7];
	mvp[2] = MVP[8];  mvp[6] = MVP[9];   mvp[10] = MVP[10];   mvp[14] = MVP[11];
	mvp[3] = MVP[12]; mvp[7] = MVP[13];  mvp[11] = MVP[14];   mvp[15] = MVP[15];

	GLuint VAO;
	GLuint VBO;
	GLuint Color;
	glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, mvp);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glGenBuffers(1, &Color);
	glBindBuffer(GL_ARRAY_BUFFER, Color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / 3);


	if (polygonMode[0] == GL_FILL) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	//please refer to LoadModels function
	//glGenVertexArrays..., glBindVertexArray...
	//glGenBuffers..., glBindBuffer..., glBufferData...

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

	// [TODO] multiply all the matrix
	MVP = project_matrix * view_matrix * S * T * R;
	// [TODO] row-major ---> column-major

	mvp[0] = 1;  mvp[4] = 0;   mvp[8] = 0;    mvp[12] = 0;
	mvp[1] = 0;  mvp[5] = 1;   mvp[9] = 0;    mvp[13] = 0;
	mvp[2] = 0;  mvp[6] = 0;   mvp[10] = 1;   mvp[14] = 0;
	mvp[3] = 0; mvp[7] = 0;  mvp[11] = 0;   mvp[15] = 1;

	mvp[0] = MVP[0];  mvp[4] = MVP[1];   mvp[8] = MVP[2];    mvp[12] = MVP[3];
	mvp[1] = MVP[4];  mvp[5] = MVP[5];   mvp[9] = MVP[6];    mvp[13] = MVP[7];
	mvp[2] = MVP[8];  mvp[6] = MVP[9];   mvp[10] = MVP[10];   mvp[14] = MVP[11];
	mvp[3] = MVP[12]; mvp[7] = MVP[13];  mvp[11] = MVP[14];   mvp[15] = MVP[15];

	// use uniform to send mvp to vertex shader
	// [TODO] draw 3D model in solid or in wireframe mode here, and draw plane
	glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, mvp);
	glBindVertexArray(m_shape_list[cur_idx].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[cur_idx].vertex_count);
	glBindVertexArray(0);
	drawPlane();

}


void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// [TODO] Call back function for keyboard
	if (key == GLFW_KEY_X && action == GLFW_PRESS) {
		if (cur_idx == 0)
			cur_idx = 4;
		else
			cur_idx -= 1;
	}
	else if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
		if (cur_idx == 4)
			cur_idx = 0;
		else
			cur_idx += 1;
	}
	else if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		setOrthogonal();
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		setPerspective();
	}

	else if (key == GLFW_KEY_T && action == GLFW_PRESS) {
		cur_trans_mode = GeoTranslation;
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		cur_trans_mode = GeoScaling;
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		cur_trans_mode = GeoRotation;
	}
	else if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		cur_trans_mode = ViewEye;
	}
	else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		cur_trans_mode = ViewCenter;
	}
	else if (key == GLFW_KEY_U && action == GLFW_PRESS) {
		cur_trans_mode = ViewUp;
	}
	else if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		GLint polygonMode[2];
		glGetIntegerv(GL_POLYGON_MODE, polygonMode);
		if (polygonMode[0] == GL_FILL) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		} else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
	else if (key == GLFW_KEY_I && action == GLFW_PRESS) {
		cout  << "******************************" << endl;
		cout << "Translation Matrix: " << models[cur_idx].position.x << "," << models[cur_idx].position.y << "," << models[cur_idx].position.z << endl;
		cout << "Scaling Matrix: " << models[cur_idx].scale.x << "," << models[cur_idx].scale.y << "," << models[cur_idx].scale.z << endl;
		cout << "Rotation Matrix: " << models[cur_idx].rotation.x << "," << models[cur_idx].rotation.y << "," << models[cur_idx].rotation.z << endl;
		cout << "Viewing Matrix: " << endl;
		cout << view_matrix << endl;
		cout << "Projection Matrix: " << endl;
		cout << project_matrix << endl;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// [TODO] scroll up positive, otherwise it would be negtive
	if (yoffset > 0) {
		//cout << "scroll up" << endl;
		switch (cur_trans_mode)
		{
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
		}
	}
	else {
		// cout << "scroll down" << endl;
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
		}
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// [TODO] Call back function for mouse
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		mouse_pressed = true;
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		current_x = xpos;
		current_y = ypos;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		mouse_pressed = false;
	}
	// cout << "mouse_button" << endl;
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	// [TODO] Call back function for cursor position
	if (mouse_pressed) {
		double diff_x = xpos - current_x;
		double diff_y = ypos - current_y;
		current_x = xpos;
		current_y = ypos;
		switch (cur_trans_mode)
		{
		case ViewEye:
			main_camera.position.x += diff_x / 50.0;
			main_camera.position.y += diff_y / 50.0;
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
			models[cur_idx].rotation.x += diff_y;
			models[cur_idx].rotation.y += diff_x;
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
	if (!success)
	{
		glGetShaderInfoLog(v, 1000, NULL, infoLog);
		std::cout << "ERROR: VERTEX SHADER COMPILATION FAILED\n" << infoLog << std::endl;
	}

	// compile fragment shader
	glCompileShader(f);
	// check for shader compile errors
	glGetShaderiv(f, GL_COMPILE_STATUS, &success);
	if (!success)
	{
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

	if (success)
		glUseProgram(p);
	else {
		system("pause");
		exit(123);
	}
}

void normalization(tinyobj::attrib_t* attrib, vector<GLfloat>& vertices, vector<GLfloat>& colors, tinyobj::shape_t* shape)
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
		attrib->vertices.at(i) = attrib->vertices.at(i)/ scale;
	}
	size_t index_offset = 0;
	vertices.reserve(shape->mesh.num_face_vertices.size() * 3);
	colors.reserve(shape->mesh.num_face_vertices.size() * 3);
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
		}
		index_offset += fv;
	}
}

void LoadModels(string model_path)
{
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	tinyobj::attrib_t attrib;
	vector<GLfloat> vertices;
	vector<GLfloat> colors;

	string err;
	string warn;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path.c_str());

	if (!warn.empty()) {
		cout << warn << std::endl;
	}

	if (!err.empty()) {
		cerr << err << std::endl;
	}

	if (!ret) {
		exit(1);
	}

	printf("Load Models Success ! Shapes size %d Maerial size %d\n", shapes.size(), materials.size());

	normalization(&attrib, vertices, colors, &shapes[0]);

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

	m_shape_list.push_back(tmp_shape);
	model tmp_model;
	models.push_back(tmp_model);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	shapes.clear();
	materials.clear();
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
	vector<string> model_list{ "../ColorModels/bunny5KC.obj", "../ColorModels/dragon10KC.obj", "../ColorModels/lucy25KC.obj", "../ColorModels/teapot4KC.obj", "../ColorModels/dolphinC.obj"};
	// [TODO] Load five model at here
	LoadModels(model_list[cur_idx]);
	LoadModels(model_list[cur_idx + 1]);
	LoadModels(model_list[cur_idx + 2]);
	LoadModels(model_list[cur_idx + 3]);
	LoadModels(model_list[cur_idx + 4]);
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
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "106070038 HW1", NULL, NULL);
	
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


	//  ass1 initial
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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