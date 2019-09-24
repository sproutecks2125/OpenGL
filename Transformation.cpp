/*
*	Title: ModuleFourOpenGL
*	Author: Brandon Rickman <brandon.rickman@snhu.edu>
*	Date: July 27th, 2019
*
*	Description: Using Transformations in OpenGL (Modern)
*/


#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

//GLM OpenMath Header
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std; //standard namespace

#define WINDOW_TITLE "Transformation - Brandon Rickman" //window title

GLint shaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VBO, VAO, EBO, texture;

//Function Prototypes
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateBuffers(void);

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

//vertex shader source code
const char* vert = 1 + R"GLSL(
	#version 330 core
	layout (location = 0) in vec3 position;
	layout (location = 1) in vec3 color;
	out vec3 mobileColor;
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;
	void main()
	{
	gl_Position = projection * view * model * vec4(position, 1.0f);
	mobileColor = color;
	}
	)GLSL";

//fragment shader source code
const char* frag = 1 + R"GLSL(
	#version 330 core
	in vec3 mobileColor;
	out vec4 gpuColor;
	void main()
	{
	gpuColor = vec4(mobileColor, 1.0);
	}
	)GLSL";

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	//use the shader program
	shaderProgram = glCreateProgram();
	AttachShader(shaderProgram, GL_VERTEX_SHADER, vert);
	AttachShader(shaderProgram, GL_FRAGMENT_SHADER, frag);
	glLinkProgram(shaderProgram);
	CheckStatus(shaderProgram, false);
	glUseProgram(shaderProgram);

	UCreateBuffers();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glutDisplayFunc(URenderGraphics);

	glutMainLoop();

	//destroys buffer objects once used
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	return 0;

}

void UResizeWindow(int w, int h)
{
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}

void URenderGraphics(void)
{
	glEnable(GL_DEPTH_TEST);	// enables z-depth

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(VAO); //Activate the vertex array object before rendering

	//declares a 4x4 identity martix
	// Transforms the object
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(0.0, 0.0f, 0.0f));	// places the object at the center of the viewport
	model = glm::rotate(model, 45.0f, glm::vec3(1.0, 1.0f, 1.0f));	// rotate the object 45 degrees on the XYZ
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));	// increase the object size by a scale of 2

	// Transforms the camera
	glm::mat4 view(1.0f);
	view = glm::translate(view, glm::vec3(0.5f, 0.0f, -5.0f)); // moves the world 0.5 units on X and 5 units in Z

	// Creates a projection
	glm::mat4 projection;
	projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f); // perspective view
	//projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f); // orthographic view

	// Retrieves and passes the transform matrices to the shader program
	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glutPostRedisplay();

	//Draw the triangles
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0); //Deactivate the vertex array object

	glutSwapBuffers();
}

//creates the buffer and array
void UCreateBuffers()
{
	//position and color data
	GLfloat vertices[] = {
						
							// position				// color
							0.0f, 0.5f, -0.5f,		1.0f, 0.0f, 0.0f,	// Top (T) vertex 0
							0.5f, -0.5f, 0.0f,		0.0f, 1.0f, 0.0f,	// Bottom Right (BR) vertex 1
							-0.5f, -0.5f, 0.0f,		0.0f, 0.0f, 1.0f,	// Bottom Left (BL) vertex 2

							0.5f, -0.5f, -1.0f,		0.5f, 0.5f, 1.0f,	// BR right vertex 3
							-0.5f, -0.5f, -1.0f,	1.0f, 0.0f, 1.0f	// BL back vertex 4
						};

	//index data to share position
	GLuint indices[] = {
							0, 1, 2,	// Triangle 1
							2, 3, 0,	// Triangle 2
							3, 4, 0,	// Triangle 3
							4, 1, 0,	// Triangle 4
							1, 2, 4,	// Triangle 5
							2, 3, 4		// Triangle 6
						};

	//Generate buffer ids
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}










/*
// Header Inclusions
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

// GLM Math Header Inclusions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std; // standard namespace

#define WINDOW_TITLE "Moder OpenGL - Transformations (Brandon Rickman)" // Window title macro

// Shader Program
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif

/* Variable declartions for shader, window size init, buffer and array objects 
GLint shaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VBO, VAO, EBO, texture;


// Function Prototypes
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);


// Vertex Shader Source Code
const GLchar * vertexShaderSource = GLSL(330,
	layout (location = 0) in vec3 position; // Vertex data from vertex attrib position 0
	layout (location = 1) in vec3 color; // Color data from vertex attrib position 1

	out vec3 mobileColor; // variable to transfer color data to fragment shader

	uniform mat4 shaderTransform; // 4x4 matrix var for transforming vertex data

	void main() {
		gl_position = shaderTransform * vec4(position, 1.0f); // transforms vertex data using matrix
		mobileColor = color; // references incoming color data
	}
);


// Fragment Shader Source Code
const GLchar * fragmentShaderSource = GLSL(330,

	in vec3 mobileColor; // variable to hold incoming color data from vertex shader

	out vec4 gpuColor; // variable to pass color data to the GPU

	void main() {
		gpuColor = vec4(mobileColor, 1.0); // Sends color data to the GPU for rendering
	}
);


/* Main Program 
int main(int argc, char* argv[]) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GLU_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initalize GLEW" << std::endl;
		return -1;
	}

	UCreateShader();

	UCreateBuffers();

	// Use the Shader Program
	glUseProgram(shaderProgram);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // sets bg color

	glutDisplayFunc(URenderGraphics);

	glutMainLoop(); // keeps window open until user clicks close

	// Destroy Buffer Objects once used
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	return 0;

}


/* Resize the Window 
void UResizeWindow(int w, int h) {
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}


/* Render Graphics in the Window 
void URenderGraphics(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the screen

	glBindVertexArray(VAO); // Activate the Vertex Array Object before rendering and transforming it

	// Declares a 4x4 identity matrix uniform variable to handle transformations
	glm::mat4 currentTransform;

	// moves the 0.5 in y
	currentTransform = glm::translate(currentTransform, glm::vec3(0.0f, 0.5f, 0.0f));

	// Roteates shape 45 degrees on the z axis
	currentTransform = glm::rotate(currentTransform, 45.0f, glm::vec3(0.0f, 0.0f, 1.0f));

	// Scales the shape down by half it's orginal size in the xyz
	currentTransform = glm::scale(currentTransform, glm::vec3(0.5f, 0.5f, 0.5f));

	// Sends transform information to the vertex shader
	GLuint transformLocation = glGetUniformLocation(shaderProgram, "shaderTransform");
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(currentTransform));

	// Draws the shape
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0); // Deactivates the Vertex Array Object

	glutSwapBuffers(); // Flips the back buffer the with front buffer every fram, similar to GL Flush

}


/* Create the shader program 
void UCreateShader() {

	// Vertex Shader
	GLint vertexShader = glCreateShader(GL_VERTEX_SHADER); // creates the vertex shader
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // Attaches the vertex shader to the source code
	glCompileShader(vertexShader); // complies the vertex shader source code

	// Fragment Shader
	GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // creates the fragment shader
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // attches the fragment shader to the source code
	glCompileShader(fragmentShader); // complies the fragment shader source code

	// Shader Program
	shaderProgram = glCreateProgram(); // creates the program and returns an ID
	glAttachShader(shaderProgram, vertexShader); // attaches vertex shader to the shader program
	glAttachShader(shaderProgram, fragmentShader); // attches vertex shader to the shader program
	glLinkProgram(shaderProgram); // Link vertex and fragment shaders to shader program

	// Delete the vertex and fragment shaders once linked
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

}


/* Create the Buffer and Array Objects (VBO and VAO) 
void UCreateBuffers() {

	// Position and Color data
	GLfloat vertices[] = {

		// location				// color
		0.5f, 0.5f, 0.0f,		1.0f, 0.0f, 0.0f,		// Top Right vertex 0
		0.5f, -0.5f, 0.0f,		0.0f, 1.0f, 0.0f,		// Bottom Right vertex 1
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f, 1.0f,		// Bottom Left vertex 2
		-0.5f, 0.5f, 0.0f,		1.0f, 0.0f, 1.0f		// Top Left vertex 3
	};

	// Index data to share position
	GLuint indices[] = {
							0, 1, 3,	// Triangle 1
							1, 2, 3		// Triangle 2
						};

	// Generate buffer IDs
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Activate the vertex array object before binding and setting any VBOs and Vertex Array Pointers
	glBindVertexArray(VAO);

	// Activate the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // copy vertices to VBO

	// Activate the Element Buffer Object / Indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // copy indices to EBO

	// Set attribute pointer 0 to hold position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); // enables vertex attribute

	// Set attribute pointer 1 to hold color data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1); // enables vertex attribute

	glBindVertexArray(0); // Deactivates the VAO which is good practice

}
*/