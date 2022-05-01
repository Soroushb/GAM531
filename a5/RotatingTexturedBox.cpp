/////////////////////////////////////////////////////////////////////////////////////
//
// This code is used to teach the course "game engine foundations" in Seneca college
// Developed by Alireza Moghaddam on Sep. 2020 
//
////////////////////////////////////////////////////////////////////////////////////

using namespace std;

#include "vgl.h"
#include "LoadShaders.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtx\rotate_vector.hpp"
#include "..\SOIL\src\SOIL.h"
#include <iostream>


const GLint NumBuffers = 2;

GLuint Buffers[NumBuffers];

const GLuint NumVertices = 18;

GLuint location;

//Array of pointers to textrure data in VRAM. We use two textures in this example.
GLuint texture[2];

//Will be used to create animation (rotate the square)
GLfloat alpha = 0;

//Transformation matrices and camera vectors
glm::mat4 model_view;

// inititializing buffers, coordinates, setting up pipeline, etc.
void init(void)
{
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	GLuint program = LoadShaders(shaders);
	glUseProgram(program);	//My Pipeline is set up


	//We use six vertices, i.e. three vertex for one triangle and three for the other triangle
	GLfloat vertices[NumVertices][6] = {

		//First triangle
		{ -0.45, 0.45 },
		{ 0.45, 0.45 },
		{ 0.45, -0.45 },

		//Second triangle
		{ 0.45, -0.45 },
		{ -0.45, -0.45 },
		{ -0.45, 0.45 },

		//Second triangle
		{ 0.45, -0.45 },
		{ -0.45, -0.45 },
		{ -0.45, 0.45 },

		{ -0.45, 0.45 },
		{ 0.45, 0.45 },
		{ 0.45, -0.45 },

		//Second triangle
		{ 0.45, -0.45 },
		{ -0.45, -0.45 },
		{ -0.45, 0.45 },

		//Second triangle
		{ 0.45, -0.45 },
		{ -0.45, -0.45 },
		{ -0.45, 0.45 }


	};

	//These are the texture coordinates for each triangle geometry
	GLfloat textureCoordinates[NumVertices][6] = {
		0.0f, 1.0f,	/*Texture coordinates for first triangle*/
		1.0f, 1.0f,
		1.0f, 0.0f,

		1.0f, 0.0f, /*Texture coordinates for second triangle*/
		0.0f, 0.0f,
		0.0f, 1.0f,

		1.0f, 0.0f, /*Texture coordinates for second triangle*/
		0.0f, 0.0f,
		0.0f, 1.0f,

	    0.0f, 1.0f,	/*Texture coordinates for first triangle*/
		1.0f, 1.0f,
		1.0f, 0.0f,

		1.0f, 0.0f, /*Texture coordinates for second triangle*/
		0.0f, 0.0f,
		0.0f, 1.0f,

		1.0f, 0.0f, /*Texture coordinates for second triangle*/
		0.0f, 0.0f,
		0.0f, 1.0f,

	};


	//Creating our texture:
	//This texture is loaded from file. To do this, we use the SOIL (Simple OpenGL Imaging Library) library.
	//When using the SOIL_load_image() function, make sure the you are using correct patrameters, or else, your image will NOT be loaded properly, or will not be loaded at all.
	GLint width1, height1;
	unsigned char* textureData1 = SOIL_load_image("grass.png", &width1, &height1, 0, SOIL_LOAD_RGB);

	GLint width2, height2;
	unsigned char* textureData2 = SOIL_load_image("apple.png", &width2, &height2, 0, SOIL_LOAD_RGB);

	glGenBuffers(2, Buffers);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindAttribLocation(program, 0, "vPosition");
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoordinates), textureCoordinates, GL_STATIC_DRAW);
	glBindAttribLocation(program, 1, "vTexCoord");
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);

	location = glGetUniformLocation(program, "model_view");


	///////////////////////TEXTURE SET UP////////////////////////

	//Allocating two buffers in VRAM
	glGenTextures(2, texture);

	//First Texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	//Loading the second texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData1);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//And now, second texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	glBindTexture(GL_TEXTURE_2D, texture[1]);

	//Loading the second texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData2);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//////////////////////////////////////////////////////////////
}



//---------------------------------------------------------------------
//
// display
//
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);	//Clears the screen and ready to draw


	//Building our transformation matrices using glm library

	//NOTE: 
	//THE ORDER OF TRANSFOEMATION DOES MATTER. A rotation followed by a translation- 
	//might return a different outcome than a translation followed by a rotation.

	//We'd like to translate our geometry 0.5 units along the X-axis
	glm::mat4 model_view_matrix = glm::translate(glm::mat4(1.0), glm::vec3(0.5, 0.0, 0.0));

	//Next, we'd like to rtate the translated geometry about Z-axis
	model_view_matrix = glm::rotate(model_view_matrix, alpha, glm::vec3(0, 0, 1));

	//Finally, we'll shrink the geometry to 70% of the original dimensions in X, Y directions
	model_view_matrix = glm::scale(model_view_matrix, glm::vec3(0.7, 0.7, 0.0));

	//Passing the built model-view_matrix to the vertex shader. Recall: variable 'location' contains the memory address of the model_view
	//matrix that is used in our vertex shader.
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view_matrix[0][0]);

	glBindTexture(GL_TEXTURE_2D, texture[0]);	//Select the first texture to be the active texture when drawing the first triangle.
	glDrawArrays(GL_TRIANGLES, 0, 3);	//Connect the vertices using "GL_TRIANGLES" modes.
										//Read more about draw modes here:
										//https://www.glprogramming.com/red/chapter02.html
										//https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glDrawArrays.xml

	glBindTexture(GL_TEXTURE_2D, texture[1]);	//Select the second texture to be the active texture when drawing the second triangle.
	glDrawArrays(GL_TRIANGLES, 3, 3);

	glFlush();							//Flush the rendered contents on the screen.
}


void keyboard(unsigned char key, int x, int y)
{

}

//Controlling Pitch with vertical mouse movement
void mouse(int x, int y)
{

}

void idle()
{
	float omega = 1;	//let omega to be the angular speed
	float timeSinceStart = glutGet(GLUT_ELAPSED_TIME) / 1000.f;		//Retreives the time in seconds
	alpha = omega * timeSinceStart;		//We use Theta = omega * time to calculate the amount of rotation of the geometry in this frame

	glutPostRedisplay();
}

//---------------------------------------------------------------------
//
// main
//
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(1024, 1024);
	glutCreateWindow("Texture Mapping");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init();

	glutDisplayFunc(display);

	glutKeyboardFunc(keyboard);

	glutIdleFunc(idle);

	glutPassiveMotionFunc(mouse);

	glutMainLoop();



}
