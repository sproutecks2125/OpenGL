/*
*	Title:	Final Project / Source.cpp
*	Author:	Brandon Rickman <brandon.rickman@snhu.edu>
*	Date:	August 19, 2019
*/

/*
*	NOTES: Resources
*	TableTop.jpg Photo by Mockup Photos on Unsplash
*	TableLeg.jpg Photo by Nicole Wilcox on Unsplash
*	Editing conducted by Brandon Rickman in Paint.net
*/


// Header inclusions
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GL/freeglut.h>

//GLM OpenMath Header
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Soil2 header file
#include "SOIL2/SOIL2.h"

using namespace std; // standard namespace

#define WINDOW_TITLE "Final Project - Brandon Rickman" // page title macro

// Shader Program Macro
#ifndef GLSL
#define GLSL(Version, Source) "#version" #Version "\n" #Source
#endif

// Global Variable Declarations
// Window Size
GLint windowWidth = 800;
GLint windowHeight = 600;

// Shader Program ID
GLint objectShaderProgram;
GLint keyLightShaderProgram;
GLint fillLightShaderProgram;

// Vertex Array & Buffer Objects
GLuint legVAO;
GLuint legVBO;
GLuint legEBO;
GLuint topVAO;
GLuint topVBO;
GLuint topEBO;
GLuint keyLightVAO;
GLuint fillLightVAO;
GLuint lightVBO;
GLuint topTex;
GLuint legTex;

// Subject position and scale
glm::vec3 objectPosition(0.0f, 0.0f, 0.0f);
glm::vec3 objectScale(2.0f);

// Pyramid and Light Color
glm::vec3 objectColor(1.0f, 1.0f, 1.0f);
glm::vec3 keyLightColor(1.0f, 1.0f, 1.0f);	// Green Light
glm::vec3 fillLightColor(1.0f, 1.0f, 1.0f);	// White Light

// Light position and Scale
glm::vec3 keyLightPosition(-1.0f, 5.5f, -6.0f);
glm::vec3 fillLightPosition(3.0f, 0.0f, 0.0f);
glm::vec3 lightScale(0.3f);

// Camera Position
glm::vec3 cameraPosition(0.0f, -1.5f, -6.0f);
GLfloat cameraSpeed = 0.05f; // movement speed per frame
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); // temporary y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, 0.0f); // temporary z unit vector
glm::vec3 front; // temporary z unit vector for mouse
int prevY = 0; // var to hold previous value of Y for zoom feature
float cameraRotation = glm::radians(-25.0f); // Camera Rotation

// Global mouse movements
GLfloat lastMouseX = 400, lastMouseY = 300; // Locks the mouse cursor at the center of the screen
GLfloat mouseXOffset, mouseYOffset, yaw = 0.0f, pitch = 0.0f; // mouse offset, yaw, and pitch var.
GLfloat sensitivity = 0.005f; // Used for mouse / camera rotation sensitivity
bool mouseDetected = true; // Initially true when mouse movement is detected
bool leftButton = false; // init left Mouse button not pressed
bool rightButton = false; // init right Mouse button not pressed

// function prototypes
void CheckStatus(GLuint, bool);
void AttachShader(GLuint, GLenum, const char*);
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UGenerateTexture(void);
void UMouseClick(int button, int state, int x, int y);
void UMouseMove(int x, int y);
void UKeyboard(unsigned char key, int x, int y);


// Modified Shader Code from Mod 4
void CheckStatus(GLuint obj, bool isShader)
{
	GLint status = GL_FALSE, log[1 << 11] = { 0 };
	(isShader ? glGetShaderiv : glGetProgramiv)(obj, isShader ? GL_COMPILE_STATUS : GL_LINK_STATUS, &status);
	(isShader ? glGetShaderInfoLog : glGetProgramInfoLog)(obj, sizeof(log), NULL, (GLchar*)log);
	if (status == GL_TRUE) return;
	std::cerr << (GLchar*)log << "\n";
	std::exit(EXIT_FAILURE);
}


void AttachShader(GLuint program, GLenum type, const char* src)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	CheckStatus(shader, true);
	glAttachShader(program, shader);
	glDeleteShader(shader);
}

/*
*	Object VERTEX SHADER SOURCE CODE
*/
const char* objectVertexShaderSource = 1 + R"GLSL(
	#version 330 core
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec3 normal;
	layout(location = 2) in vec2 textureCoordinate;

	out vec3 Normal;
	out vec3 FragmentPos;
	out vec2 mobileTextureCoordinate;

	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main() {
		gl_Position = projection * view * model * vec4(position, 1.0f);
		FragmentPos = vec3(model * vec4(position, 1.0f));
		Normal = mat3(transpose(inverse(model))) * normal;
		mobileTextureCoordinate = vec2(textureCoordinate.x, 1.0f - textureCoordinate.y);
	}
)GLSL";

// Object FRAGMENT SHADER SOURCE CODE
const char* objectFragmentShaderSource = 1 + R"GLSL(
	#version 330 core
	in vec3 Normal;
	in vec3 FragmentPos;
	in vec2 mobileTextureCoordinate;

	out vec4 pyramidColor;

	uniform vec3 keyLightColor;
	uniform vec3 fillLightColor;
	uniform vec3 keyLightPos;
	uniform vec3 fillLightPos;
	uniform vec3 viewPosition;
	uniform sampler2D uTexture;

	void main() {

		float ambientStrength = 0.1f;
		vec3 keyAmbient = ambientStrength * keyLightColor;
		vec3 fillAmbient = ambientStrength * fillLightColor;

		vec3 norm = normalize(Normal);
		vec3 keyLightDirection = normalize(keyLightPos - FragmentPos);
		vec3 fillLightDirection = normalize(fillLightPos - FragmentPos);
		float keyImpact = max(dot(norm, keyLightDirection), 0.0);
		float fillImpact = max(dot(norm, fillLightDirection), 0.0);
		vec3 keyDiffuse = keyImpact * keyLightColor;
		vec3 fillDiffuse = fillImpact * fillLightColor;

		float keySpecularIntensity = 1.0f;
		float fillSpecularIntensity = 0.1f;
		float highlightSize = 16.0f;
		vec3 viewDir = normalize(viewPosition - FragmentPos);
		vec3 keyReflectDir = reflect(-keyLightDirection, norm);
		vec3 fillReflectDir = reflect(-fillLightDirection, norm);
		float keySpecularComponent = pow(max(dot(viewDir, keyReflectDir), 0.0), highlightSize);
		float fillSpecularComponent = pow(max(dot(viewDir, fillReflectDir), 0.0), highlightSize);
		vec3 keySpecular = keySpecularIntensity * keySpecularComponent * keyLightColor;
		vec3 fillSpecular = fillSpecularIntensity * fillSpecularComponent * fillLightColor;
		
		vec3 objectColor = texture(uTexture, mobileTextureCoordinate).xyz;
		vec3 keyPhong = (keyAmbient + keyDiffuse + keySpecular) * objectColor;
		vec3 fillPhong = (fillAmbient + fillDiffuse + fillSpecular) * objectColor;
		vec3 phong = keyPhong + fillPhong;
		pyramidColor = vec4(phong, 1.0f);
	}
)GLSL";

// KEY LIGHT VERTEX SHADER SOURCE CODE
const char* keyLightVertexShaderSource = 1 + R"GLSL(
	#version 330 core
	layout(location = 0) in vec3 position;

	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main() {
		gl_Position = projection * view * model * vec4(position, 1.0f);
	}
)GLSL";


// KEY LIGHT FRAGMENT SHADER SOURCE CODE
const char* keyLightFragmentShaderSource = 1 + R"GLSL(
	#version 330 core
	out vec4 color;

	void main() {
		color = vec4(0.8f, 1.0f, 0.8f, 1.0f);
	}
)GLSL";

// FILL LIGHT VERTEX SHADER SOURCE CODE
const char* fillLightVertexShaderSource = 1 + R"GLSL(
	#version 330 core
	layout(location = 0) in vec3 position;
	
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main() {
		gl_Position = projection * view * model * vec4(position, 1.0f);
	}
)GLSL";


// FILL LIGHT FRAGMENT SHADER SOURCE CODE
const char* fillLightFragmentShaderSource = 1 + R"GLSL(
	#version 330 core

	out vec4 color;

	void main() {
		color = vec4(1.0f);
	}
)GLSL";


// Main Program
int main(int argc, char* argv[]) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	
	UCreateShader();
	UCreateBuffers();
	UGenerateTexture();

	glClearColor(0.82f, 0.7f, 0.554f, 1.0f); // sets background color to BLACK

	glutDisplayFunc(URenderGraphics);

	// view projections
	glutKeyboardFunc(UKeyboard); // detects key press
	glutKeyboardUpFunc(UKeyboard); // detects key release

	// mouse operations
	glutMouseFunc(UMouseClick); // detects mouse click
	glutPassiveMotionFunc(UMouseMove); // detects mouse movement
	glutMotionFunc(UMouseMove); // detects mouse press and movement

	glutMainLoop();

	// Destroy Buffer Objects once used
	glDeleteVertexArrays(1, &legVAO);
	glDeleteVertexArrays(1, &topVAO);
	glDeleteVertexArrays(1, &keyLightVAO);
	glDeleteVertexArrays(1, &fillLightVAO);
	glDeleteBuffers(1, &legVBO);
	glDeleteBuffers(1, &topVBO);
	glDeleteBuffers(1, &lightVBO);

	// Successfully exit the program
	return 0;

}


// Function that creates shaders
void UCreateShader(void) {
	// pyramid SHADERS
	// Creates the shader program and returns and ID for the pyramid
	objectShaderProgram = glCreateProgram();

	// Attach the pyramid's vertex shader to the shader program
	AttachShader(objectShaderProgram, GL_VERTEX_SHADER, objectVertexShaderSource);
	// Attach the pyramid's fragment shader to the shader program
	AttachShader(objectShaderProgram, GL_FRAGMENT_SHADER, objectFragmentShaderSource);
	// Link vertex and fragment shaders to the pyramid's shader program
	glLinkProgram(objectShaderProgram);
	CheckStatus(objectShaderProgram, false);

	// KEY LAMP SHADERS
	keyLightShaderProgram = glCreateProgram();
	AttachShader(keyLightShaderProgram, GL_VERTEX_SHADER, keyLightVertexShaderSource);
	AttachShader(keyLightShaderProgram, GL_FRAGMENT_SHADER, keyLightFragmentShaderSource);
	glLinkProgram(keyLightShaderProgram);
	CheckStatus(keyLightShaderProgram, false);

	// FILL LAMP SHADERS
	fillLightShaderProgram = glCreateProgram();
	AttachShader(fillLightShaderProgram, GL_VERTEX_SHADER, fillLightVertexShaderSource);
	AttachShader(fillLightShaderProgram, GL_FRAGMENT_SHADER, fillLightFragmentShaderSource);
	glLinkProgram(fillLightShaderProgram);
	CheckStatus(fillLightShaderProgram, false);
}


// Resize window
void UResizeWindow(int w, int h) {

	windowWidth = w;
	windowHeight = h;
	glViewport(0, 0, windowWidth, windowHeight);

}


// Render graphics
void URenderGraphics(void) {

	glEnable(GL_DEPTH_TEST); // allows z-axis

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clears screen

	GLint modelLoc;
	GLint viewLoc;
	GLint projLoc;
	GLint uTextureLoc;
	GLint keyLightColorLoc;
	GLint fillLightColorLoc;
	GLint keyLightPositionLoc;
	GLint fillLightPositionLoc;
	GLint viewPositionLoc;

	glm::mat4 model(1.0f);
	glm::mat4 view(1.0f);
	glm::mat4 projection;


	// Table Leg Draw
	// USE THE SHADER AND ACTIVIATE pyramid VAO FOR RENDERING AND TRANSFORMING
	glUseProgram(objectShaderProgram);
	glBindVertexArray(legVAO);

	// Transform the pyramid
	model = glm::translate(model, objectPosition);
	model = glm::scale(model, objectScale);	

	// Transform the camera
	CameraForwardZ = front; // Replaces camera forward vector with Radians normalized as a unit vector
	view = glm::translate(view, cameraPosition);
	view = glm::rotate(view, cameraRotation, glm::vec3(0.0f, 0.0f, 0.0f));
	view = glm::lookAt(cameraPosition - CameraForwardZ, cameraPosition, CameraUpY); // moves the world 0.5 units on X and 5 units in Z
   
	// Set the camera projection to perspective
	projection = glm::perspective(45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.1f, 100.0f);

	// Reference matrix uniforms from the pyramid Shader Program
	modelLoc = glGetUniformLocation(objectShaderProgram, "model");
	viewLoc = glGetUniformLocation(objectShaderProgram, "view");
	projLoc = glGetUniformLocation(objectShaderProgram, "projection");

	// Pass matrix data to the pyramid Shader Program's matrix uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Reference matrix uniforms from the pyramid Shader program for:
	// the pyramid color, light color, light position and camera position
	uTextureLoc = glGetUniformLocation(objectShaderProgram, "uTexture");
	keyLightColorLoc = glGetUniformLocation(objectShaderProgram, "keyLightColor");
	fillLightColorLoc = glGetUniformLocation(objectShaderProgram, "fillLightColor");
	keyLightPositionLoc = glGetUniformLocation(objectShaderProgram, "keyLightPos");
	fillLightPositionLoc = glGetUniformLocation(objectShaderProgram, "fillLightPos");
	viewPositionLoc = glGetUniformLocation(objectShaderProgram, "viewPosition");

	// Pass color, light and camera data to the pyramid Shader program's corresponding uniforms
	glUniform1i(uTextureLoc, 0);
	glUniform3f(keyLightColorLoc, keyLightColor.r, keyLightColor.g, keyLightColor.b);
	glUniform3f(fillLightColorLoc, fillLightColor.r, fillLightColor.g, fillLightColor.b);
	glUniform3f(keyLightPositionLoc, keyLightPosition.x, keyLightPosition.y, keyLightPosition.z);
	glUniform3f(fillLightPositionLoc, fillLightPosition.x, fillLightPosition.y, fillLightPosition.z);
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	// Provide texture to the pyramid
	glBindTexture(GL_TEXTURE_2D, legTex);

	//Draw the triangles
	glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, 0);

	// Deactiviate the pyramid Vertex Array Object
	glBindVertexArray(0);


	// Table Top Draw
	glBindVertexArray(topVAO);
	glBindTexture(GL_TEXTURE_2D, topTex);
	glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);


	// KEY LIGHT DRAW
	// USE THE KEY LIGHT SHADER AND ACTIVATE LAMP VERTEX ARRAY OBJECT FOR RENDERING AND TRANSFORMING
	glUseProgram(keyLightShaderProgram);
	glBindVertexArray(keyLightVAO);

	// Transform the smaller pyramid used as a visual que for the light source
	model = glm::translate(model, keyLightPosition);
	model = glm::scale(model, lightScale);

	// Reference matrix uniforms from the lamp shader program
	modelLoc = glGetUniformLocation(keyLightShaderProgram, "model");
	viewLoc = glGetUniformLocation(keyLightShaderProgram, "view");
	projLoc = glGetUniformLocation(keyLightShaderProgram, "projection");

	// Pass matrix data to the lamp shader program's matrix uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Draw the smaller LAMP cube
	glDrawArrays(GL_TRIANGLES, 0, 36);

	// Deactivate the lamp vertex array object
	glBindVertexArray(0);

	
	// FILL LIGHT DRAW
	// USE THE FILL LIGHT SHADER AND ACTIVATE LAMP VERTEX ARRAY OBJECT FOR RENDERING AND TRANSFORMING
	glUseProgram(fillLightShaderProgram);
	glBindVertexArray(fillLightVAO);
	model = glm::translate(model, keyLightPosition);
	model = glm::scale(model, lightScale);
	modelLoc = glGetUniformLocation(keyLightShaderProgram, "model");
	viewLoc = glGetUniformLocation(keyLightShaderProgram, "view");
	projLoc = glGetUniformLocation(keyLightShaderProgram, "projection");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	// CLEAN UP
	glutPostRedisplay();
	glBindVertexArray(0); //Deactivate the vertex array object
	glutSwapBuffers(); // Flips the back buffer to the front buffer every frame.

}


// Implements the UCreateBuffers function
void UCreateBuffers() {


	float legVertices[] = {

		// Position				// Normals				//TexCoords
		// Left Front Leg		
		-0.5f, -0.5f,  0.5f,	0.0f, 0.0f,  1.0f,		0.0f, 0.0f,		// vert 0 
		-0.4f, -0.5f,  0.5f,	0.0f, 0.0f,  1.0f,		1.0f, 0.0f,		// vert 1
		-0.4f,  0.5f,  0.5f,	0.0f, 0.0f,  1.0f,		1.0f, 1.0f,		// vert 2
		-0.5f,  0.5f,  0.5f,	0.0f, 0.0f,  1.0f,		0.0f, 1.0f,		// vert 3
		-0.5f, -0.5f,  0.4f,	0.0f, 0.0f, -1.0f,		1.0f, 0.0f,		// vert 4 
		-0.5f,  0.5f,  0.4f,	0.0f, 0.0f, -1.0f,		1.0f, 1.0f,		// vert 5
		-0.4f, -0.5f,  0.4f,	0.0f, 0.0f, -1.0f,		0.0f, 0.0f,		// vert 6
		-0.4f,	0.5f,  0.4f,	0.0f, 0.0f, -1.0f,		0.0f, 1.0f,		// vert 7

		 // Right Front Leg
		 0.4f, -0.5f,  0.5f,	0.0f, 0.0f,  1.0f,		0.0f, 0.0f,		// vert 8 
		 0.5f, -0.5f,  0.5f,	0.0f, 0.0f,  1.0f,		1.0f, 0.0f,		// vert 9
		 0.5f,  0.5f,  0.5f,	0.0f, 0.0f,  1.0f,		1.0f, 1.0f,		// vert 10
		 0.4f,  0.5f,  0.5f,	0.0f, 0.0f,  1.0f,		0.0f, 1.0f,		// vert 11
		 0.4f, -0.5f,  0.4f,	0.0f, 0.0f, -1.0f,		1.0f, 0.0f,		// vert 12
		 0.4f,  0.5f,  0.4f,	0.0f, 0.0f, -1.0f,		1.0f, 1.0f,		// vert 13
		 0.5f, -0.5f,  0.4f,	0.0f, 0.0f, -1.0f,		0.0f, 0.0f,		// vert 14
		 0.5f,  0.5f,  0.4f,	0.0f, 0.0f, -1.0f,		0.0f, 1.0f,		// vert 15

		 // Right Back Leg
		 0.4f, -0.5f, -0.4f,	0.0f, 0.0f,  1.0f,		0.0f, 0.0f,		// vert 16
		 0.5f, -0.5f, -0.4f,	0.0f, 0.0f,  1.0f,		1.0f, 0.0f,		// vert 17
		 0.5f,  0.5f, -0.4f,	0.0f, 0.0f,  1.0f,		1.0f, 1.0f,		// vert 18
		 0.4f,  0.5f, -0.4f,	0.0f, 0.0f,  1.0f,		0.0f, 1.0f,		// vert 19
		 0.4f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 0.0f,		// vert 20
		 0.4f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 1.0f,		// vert 21
		 0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 0.0f,		// vert 22
		 0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 1.0f,		// vert 23

		// Left Back Leg
		-0.5f, -0.5f, -0.4f,	0.0f, 0.0f,  1.0f,		0.0f, 0.0f,		// vert 24
		-0.4f, -0.5f, -0.4f,	0.0f, 0.0f,  1.0f,		1.0f, 0.0f,		// vert 25
		-0.4f,  0.5f, -0.4f,	0.0f, 0.0f,  1.0f,		1.0f, 1.0f,		// vert 26
		-0.5f,  0.5f, -0.4f,	0.0f, 0.0f,  1.0f,		0.0f, 1.0f,		// vert 27
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 0.0f,		// vert 28
		-0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 1.0f,		// vert 29
		-0.4f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 0.0f,		// vert 30
		-0.4f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 1.0f		// vert 31
	};

	float topVertices[] 
	{

		// Table Top
		-0.6f,  0.5f,  0.6f,	0.0f, 1.0f, 1.0f,		0.0f, 0.0f,		// vert 0
		 0.6f,  0.5f,  0.6f,	0.0f, 1.0f, 1.0f,		1.0f, 0.0f,		// vert 1	
		 0.6f,  0.6f,  0.6f,	0.0f, 1.0f, 1.0f,		1.0f, 1.0f,		// vert 2	
		-0.6f,  0.6f,  0.6f,	0.0f, 1.0f, 1.0f,		0.0f, 1.0f,		// vert 3	
		-0.6f,  0.5f, -0.6f,	0.0f, 1.0f, 1.0f,		1.0f, 0.0f,		// vert 4	
		-0.6f,  0.6f, -0.6f,	0.0f, 1.0f, 1.0f,		1.0f, 1.0f,		// vert 5	
		 0.6f,  0.5f, -0.6f,	0.0f, 1.0f, 1.0f,		0.0f, 0.0f,		// vert 6	
		 0.6f,  0.6f, -0.6f,	0.0f, 1.0f, 1.0f,		0.0f, 1.0f		// vert 7	

	};

	GLuint legIndicies[]
	{
		// Left Front Leg
		0,1,2,		2,3,0,		3,0,4,		4,5,3,		5,4,6,		6,7,5,
		6,7,2,		2,1,6,		3,2,7,		7,5,3,		0,1,6,		6,4,0,

		// Right Front Leg
		8,9,10,		10,11,8,	11,8,12,	12,13,11,	13,12,14,	14,15,13,
		14,15,10,	10,9,14,	11,10,15,	15,13,11,	8,9,14,		14,12,8,

		// Right Back Leg
		16,17,18,	18,19,16,	19,16,20,	20,21,19,	21,20,22,	22,23,21,
		22,23,18,	18,17,22,	19,18,23,	23,21,19,	16,17,22,	22,20,16,

		// Left Back Leg
		24,25,26,	26,27,24,	27,24,28,	28,29,27,	29,28,30,	30,31,29,
		30,31,26,	26,25,30,	27,26,31,	31,29,27,	24,25,30,	30,28,24

	};

	GLuint topIndices[]
	{

		// Table Top
		0,1,2,		2,3,0,		3,0,4,		4,5,3,		5,4,6,		6,7,5,
		6,7,2,		2,1,6,		3,2,7,		7,5,3,		0,1,6,		6,4,0

	};

	float lightV[]
	{
		//Position				
		// Back Face			
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		// Front Face
		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		// Left Face
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		// Right Face
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		// Bottom Face
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		// Top Face	
		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f
	
	};

	// Generate buffer IDs
	glGenBuffers(1, &legVBO);
	glGenBuffers(1, &legEBO);
	glGenBuffers(1, &topVBO);
	glGenBuffers(1, &topEBO);
	glGenBuffers(1, &lightVBO);

	glGenVertexArrays(1, &legVAO);
	glGenVertexArrays(1, &topVAO);
	glGenVertexArrays(1, &fillLightVAO);
	glGenVertexArrays(1, &keyLightVAO);

	// Table Legs
	// Activate the VAO before binding and setting any VBOs and Vertex Attribute Pointers
	glBindVertexArray(legVAO);

	// Activate the VBO
	glBindBuffer(GL_ARRAY_BUFFER, legVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(legVertices), legVertices, GL_STATIC_DRAW); // Copy vertices to VBO

	// Activate the EBO for index connections
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, legEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(legIndicies), legIndicies, GL_STATIC_DRAW);

	// Set attrib ptr 0 to hold Position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); // Enable vertex attribute 0

	// Set attrib ptr 1 to hold normal data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1); // Enable vertex attribute 1

	// Set attrib ptr 2 to hold texture data
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0); // Deactivates the VAO

	// Table Top
	glBindVertexArray(topVAO);
	glBindBuffer(GL_ARRAY_BUFFER, topVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(topVertices), topVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, topEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(topIndices), topIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	// KEY LIGHT
	glBindVertexArray(keyLightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lightV), lightV, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// FILL LIGHT
	glBindVertexArray(fillLightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lightV), lightV, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

}


void UGenerateTexture(void) {
	// Local Variable Declarations
	int width;
	int height;
	// Loads texture file
	unsigned char* topImage = SOIL_load_image("TableTop.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	unsigned char* legImage = SOIL_load_image("TableLeg.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	
	// Create texture
	glGenTextures(1, &topTex);
	// Bind to type of texture
	glBindTexture(GL_TEXTURE_2D, topTex);

	// Load the texture image - TableTop.jpg
	// Parameters: (1) Texture Target, (2) Level of Detail, (3) Internal Pixel Format
	// (4) Width, (5) Height, (6) "0", (7 & 8) Format of Pixels, (9) Image Array
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, topImage);
	// Filter texture with mipmap to provide higher quality and performance
	glGenerateMipmap(GL_TEXTURE_2D);
	// Load configured image texture
	SOIL_free_image_data(topImage);
	// Deactivate texture after using it
	glBindTexture(GL_TEXTURE_2D, 0);

	// Load the texture image - TableLeg.jpg
	glGenTextures(1, &legTex);
	glBindTexture(GL_TEXTURE_2D, legTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, legImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(legImage);
	glBindTexture(GL_TEXTURE_2D, 0);

}


// Implements the UMouseClick function
void UMouseClick(int button, int state, int x, int y) {

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		leftButton = true; // pressed
	}

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
		leftButton = false; // released
	}

	if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
		rightButton = true; // pressed
	}

	if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)) {
		rightButton = false; // released
	}

}



// Implements the UMouseMove function
void UMouseMove(int x, int y) {

	// Allows access to SHIFT, CTRL, and ALT keys
	int mod = glutGetModifiers();

	if (mouseDetected) {

		lastMouseX = x;
		lastMouseY = y;
		mouseDetected = false;

	}

	front.x = 10.0f * cos(yaw);
	front.y = 10.0f * sin(pitch);
	front.z = sin(yaw) * cos(pitch) * 10.0f;

	// Orbit around 3D object while holding ALT and click/hold the left mouse button, then move mouse
	if (leftButton && mod == 4) {

		// Gets the direction the mouse was moved in x and y
		mouseXOffset = x - lastMouseX;
		mouseYOffset = lastMouseY - y; // inverted Y

		// Updates with new mouse coords
		lastMouseX = x;
		lastMouseY = y;

		// Applies sensitivity to mouse direction
		mouseXOffset *= sensitivity;
		mouseYOffset *= sensitivity;

		// Accumulates the yaw and pitch vars
		yaw += mouseXOffset;
		pitch += mouseYOffset;

		front.x = 10.0f * cos(yaw);
		front.y = 10.0f * sin(pitch);
		front.z = sin(yaw) * cos(pitch) * 10.0f;

	}
	// Right mouse click and hold while holding ALT to zoom IN (Drag UP) and zoom OUT (Drag DOWN)
	if (rightButton && mod == 4) {

		if (y > prevY) {
			cameraPosition += cameraSpeed * CameraForwardZ;
		}
		if (y < prevY) {
			cameraPosition -= cameraSpeed * CameraForwardZ;
		}
		prevY = y;

	}
}


// Implement Keyboard function
void UKeyboard(unsigned char key, int x, int y)
{
	glm::mat4 projection;
	int VIEW = glutGetModifiers();

	// Hold SHIFT to view ORTHO
	if (VIEW == 1)
	{
		projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f); // orthographic view

	}
	// views PERSPECTIVE by deafault
	else
	{
		projection = glm::perspective(45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.1f, 100.0f); // perspective view
	}

}
