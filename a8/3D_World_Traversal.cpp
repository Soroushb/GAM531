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
#include <vector>
#include <iostream>

enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer };
enum Attrib_IDs { vPosition = 0 };

const GLint NumBuffers = 3;
GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
GLuint location;
GLuint cam_mat_location;
GLuint proj_mat_location;
GLuint texture[3];	//Array of pointers to textrure data in VRAM. We use two textures in this example.
GLfloat alpha = 0;
int countHits = 0;


enum GameObject_Type {
	PLAYER,
	ENEMY,
	BULLET,
	OBSTACLE
};

class GameObjects {
public:
	char type;
	glm::vec3 location;
	glm::vec3 rotation;
	glm::vec3 movingDirection;
	glm::vec3 scale;
	GLfloat velocity;
	GLfloat collider_dimension; //We use box as wrapper with radius = 0.9 * scale of the object Note: 0.9 is the original dimension of the boxes we generate
	int lifeSpan = glutGet(GLUT_ELAPSED_TIME);
	bool isAlive;
	bool isCollided;
	int living_time;


};

/*class GameObjects2 {
public:
	char type;
	glm::vec3 location;
	glm::vec3 movingDirection;
};*/

vector<GameObjects*> GameScene;
//vector<GameObjects2*> GameScene2;


const GLuint NumVertices = 28;

//Height of camera (player) from the level
float height = 0.8f;

//Player motion speed for movement and pitch/yaw
float travel_speed = 300.0f;		//Motion speed
float mouse_sensitivity = 0.01f;	//Pitch/Yaw speed

//Used for tracking mouse cursor position on screen
int x0 = 0;
int y_0 = 0;

//Transformation matrices and camera vectors
glm::mat4 model_view;
glm::vec3 unit_z_vector = glm::vec3(0, 0, 1);	//Assigning a meaningful name to (0,0,1) :-)
glm::vec3 cam_pos = glm::vec3(0.0f, 0.0f, height);
glm::vec3 forward_vector = glm::vec3(1, 1, 0);	//Forward vector is parallel to the level at all times (No pitch)

//The direction which the camera is looking, at any instance
glm::vec3 looking_dir_vector = glm::vec3(1, 1, 0);
glm::vec3 up_vector = unit_z_vector;
glm::vec3 side_vector = glm::cross(up_vector, forward_vector);

glm::vec3 center = glm::vec3(0, 0, 0);

//Used to measure time between two frames
int oldTimeSinceStart = 0;
int deltaTime;

//Creating and rendering bunch of objects on the scene to interact with
const int Num_Obstacles = 40;
float obstacle_data[Num_Obstacles][3];




//Helper function to generate a random float number within a range
float randomFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

// inititializing buffers, coordinates, setting up pipeline, etc.
void init(void)
{
	glEnable(GL_DEPTH_TEST);

	//Normalizing all vectors
	up_vector = glm::normalize(up_vector);
	forward_vector = glm::normalize(forward_vector);
	looking_dir_vector = glm::normalize(looking_dir_vector);
	side_vector = glm::normalize(side_vector);


	//obstacle_data[0][0] = 0; //X
	//obstacle_data[0][1] = 0; //Y
	//obstacle_data[0][2] = 2; //Scale

	for (int i = 0; i < Num_Obstacles; i++)
	{
		obstacle_data[i][0] = randomFloat(-50, 50); //X
		obstacle_data[i][1] = randomFloat(-50, 50); //Y
		obstacle_data[i][2] = randomFloat(0.1, 10.0); //Scale
	}

	/*
	for (int i = 0; i < GameScene2.size(); i++)
	{
		GameScene2[i]->location.x = randomFloat(0, 50); //X
		GameScene2[i]->location.y = randomFloat(0, 50); //Y
		obstacle_data[i][2] = randomFloat(0.1, 10.0); //Scale
	}
	*/

	GameObjects *go = new GameObjects();
	(*go).location = cam_pos;	
	(*go).collider_dimension = 5;	//The default dimension of a box is 1 unit of length. So, we scale it to scale.x.
	(*go).isAlive = true;
	(*go).isCollided = false;
	(*go).velocity = 0;
	(*go).type = PLAYER;
	GameScene.push_back(go);


	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	GLuint program = LoadShaders(shaders);
	glUseProgram(program);	//My Pipeline is set up


	//Since we use texture mapping, to simplify the task of texture mapping, 
	//and to clarify the demonstration of texture mapping, we consider 4 vertices per face.
	//Overall, we will have 24 vertices and we have 4 vertices to render the sky (a large square).
	//Therefore, we'll have 28 vertices in total.
	GLfloat vertices[NumVertices][3] = {

		{ -100.0, -100.0, 0.0 }, //Plane to walk on and a sky
		{ 100.0, -100.0, 0.0 },
		{ 100.0, 100.0, 0.0 },
		{ -100.0, 100.0, 0.0 },

		{ -0.45, -0.45 ,0.01 }, // bottom face
		{ 0.45, -0.45 ,0.01 },
		{ 0.45, 0.45 ,0.01 },
		{ -0.45, 0.45 ,0.01 },

		{ -0.45, -0.45 ,0.9 }, //top face
		{ 0.45, -0.45 ,0.9 },
		{ 0.45, 0.45 ,0.9 },
		{ -0.45, 0.45 ,0.9 },

		{ 0.45, -0.45 , 0.01 }, //left face
		{ 0.45, 0.45 , 0.01 },
		{ 0.45, 0.45 ,0.9 },
		{ 0.45, -0.45 ,0.9 },

		{ -0.45, -0.45, 0.01 }, //right face
		{ -0.45, 0.45 , 0.01 },
		{ -0.45, 0.45 ,0.9 },
		{ -0.45, -0.45 ,0.9 },

		{ -0.45, 0.45 , 0.01 }, //front face
		{ 0.45, 0.45 , 0.01 },
		{ 0.45, 0.45 ,0.9 },
		{ -0.45, 0.45 ,0.9 },

		{ -0.45, -0.45 , 0.01 }, //back face
		{ 0.45, -0.45 , 0.01 },
		{ 0.45, -0.45 ,0.9 },
		{ -0.45, -0.45 ,0.9 },
	};

	//These are the texture coordinates for the second texture
	GLfloat textureCoordinates[28][2] = {
		0.0f, 0.0f,
		200.0f, 0.0f,
		200.0f, 200.0f,
		0.0f, 200.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
	};


	//Creating our texture:
	//This texture is loaded from file. To do this, we use the SOIL (Simple OpenGL Imaging Library) library.
	//When using the SOIL_load_image() function, make sure the you are using correct patrameters, or else, your image will NOT be loaded properly, or will not be loaded at all.
	GLint width1, height1;
	unsigned char* textureData1 = SOIL_load_image("grass.png", &width1, &height1, 0, SOIL_LOAD_RGB);

	GLint width2, height2;
	unsigned char* textureData2 = SOIL_load_image("newtank.png", &width2, &height2, 0, SOIL_LOAD_RGB);


	GLint width3, height3;
	unsigned char* textureData3 = SOIL_load_image("wheel.png", &width3, &height3, 0, SOIL_LOAD_RGB);

	glGenBuffers(3, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindAttribLocation(program, 0, "vPosition");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoordinates), textureCoordinates, GL_STATIC_DRAW);
	glBindAttribLocation(program, 1, "vTexCoord");
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);

	glGenBuffers(3, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindAttribLocation(program, 0, "vPosition");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	location = glGetUniformLocation(program, "model_matrix");
	cam_mat_location = glGetUniformLocation(program, "camera_matrix");
	proj_mat_location = glGetUniformLocation(program, "projection_matrix");

	///////////////////////TEXTURE SET UP////////////////////////

	//Allocating two buffers in VRAM
	glGenTextures(3, texture);

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



	//And now, Third texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	glBindTexture(GL_TEXTURE_2D, texture[2]);

	//Loading the second texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width3, height3, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData3);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//////////////////////////////////////////////////////////////
}

//Helper function to draw a cube
void drawCube(float scale)
{
	model_view = glm::scale(model_view, glm::vec3(scale, scale, scale));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

}


//This function takes in two game objects and finds out if they are colliding.
bool isColliding(GameObjects* one, GameObjects* two) {
	bool result = false;

	//cout << "Scales:" << one.scale.x << ", " << one.collider_dimension << ", " << two.scale.x << ", " << two.collider_dimension << endl;
	//cout << "Coordinates:" << glm::abs(one.location.x - two.location.x) << ", " << glm::abs(one.location.y - two.location.y) << ", "<< glm::abs(one.location.z - two.location.z) << ", " << one.collider_dimension / 2 + two.collider_dimension / 2 << endl;

	bool cond = glm::abs(one->location.x - two->location.x) <= (one->collider_dimension / 2 + two->collider_dimension / 2) &&
		glm::abs(one->location.y - two->location.y) <= (one->collider_dimension / 2 + two->collider_dimension / 2) &&
		glm::abs(one->location.z - two->location.z) <= (one->collider_dimension / 2 + two->collider_dimension / 2);

	if (cond) {
		cout << one->type << " has collided with" << two->type << endl;
		//cout << one.location.x << ", " << one.location.y << ", " << one.location.z << endl;
		result = true;
	}
	return result;
}



//This function iterates through the scene graph and checks the collision status between each and every two objects
//When collided, the .isCollided property of the game object is set to true
void checkCollisions() {


	for (int i = 0; i < GameScene.size(); i++) {
		for (int j = 0; j < GameScene.size(); j++) {


			if (i != j && /*if i=j then it means that we are checking self-collilsion. We do NOT consider self-collision as a collision*/
				GameScene[i]->isAlive &&
				GameScene[j]->isAlive &&
				!(GameScene[i]->type == OBSTACLE && GameScene[j]->type == OBSTACLE) && //We ignore the collision between two obstacles :-)
				isColliding(GameScene[i], GameScene[j])) {

				GameScene[i]->isCollided = true;
				GameScene[j]->isCollided = true;

				GameScene[i]->isAlive = false;
				GameScene[j]->isAlive = false;

				if (GameScene[i]->type == BULLET || GameScene[j]->type == BULLET) {
					countHits += 1;
				}

				if ((GameScene[i]->type == PLAYER || GameScene[j]->type == PLAYER) && (GameScene[i]->type != BULLET && GameScene[j]->type != BULLET)) {
					cout << "LOST" << endl;
				}


			}
		}
	}

	if (countHits == 10) {
		cout << "Victory";
		countHits = 0;
	}
	//cout << "Victory!";

}


void drawHex(float scale)
{
	model_view = glm::scale(model_view, glm::vec3(scale, scale, scale));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

}


//Renders level
void draw_level()
{
	//Select the first texture (grass.png) when drawing the first geometry (floor)
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glDrawArrays(GL_QUADS, 0, 4);

	//Rendering obstacles obstacles
	/*for (int i = 0; i < Num_Obstacles; i++)
	{
		model_view = glm::translate(model_view, glm::vec3(obstacle_data[i][0], obstacle_data[i][1], 0.0));
		glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
		drawCube(obstacle_data[i][2]);
		model_view = glm::mat4(1.0);
	}
	*/
}



/*void drawObjects2() {

	for (int i = 0; i < GameScene2.size(); i++)
	{
		model_view = glm::mat4(1.0);


		model_view = glm::translate(model_view, GameScene2[i]->location);

		//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
		drawCube(model_view[0][0]);
		glBindTexture(GL_TEXTURE_2D, texture[1]);
		glDrawArrays(GL_QUADS, 4, 24);
		glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);



		glm::mat4 temp = model_view;

		model_view = glm::translate(model_view, glm::vec3(-0.45f, 0.45f, 0.0f));
		model_view = glm::rotate(model_view, alpha / 1000, glm::vec3(0.0f, 1.0f, 0.0f));
		model_view = glm::scale(model_view, glm::vec3(0.5f, 0.5f, 1.0f));
		//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
		drawCube(model_view[0][0]);
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glDrawArrays(GL_QUADS, 4, 24);

		model_view = temp;

		model_view = glm::translate(model_view, glm::vec3(0.45f, -0.45f, 0.0f));
		model_view = glm::rotate(model_view, alpha / 1000, glm::vec3(0.0f, 1.0f, 0.0f));
		model_view = glm::scale(model_view, glm::vec3(0.5f, 0.5f, 1.0f));
		//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
		drawCube(model_view[0][0]);
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glDrawArrays(GL_QUADS, 4, 24);

		model_view = temp;

		model_view = glm::translate(model_view, glm::vec3(0.45f, 0.45f, 0.0f));
		model_view = glm::rotate(model_view, alpha / 1000, glm::vec3(0.0f, 1.0f, 0.0f));
		model_view = glm::scale(model_view, glm::vec3(0.5f, 0.5f, 1.0f));
		//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
		drawCube(model_view[0][0]);
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glDrawArrays(GL_QUADS, 4, 24);

		model_view = temp;

		model_view = glm::translate(model_view, glm::vec3(-0.45f, -0.45f, 0.0f));
		model_view = glm::rotate(model_view, alpha / 1000, glm::vec3(0.0f, 1.0f, 0.0f));
		model_view = glm::scale(model_view, glm::vec3(0.5f, 0.5f, 1.0f));
		//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
		drawCube(model_view[0][0]);
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glDrawArrays(GL_QUADS, 4, 24);
	}
}
*/
void updateScene() {


	checkCollisions();

	for (int i = 0; i < GameScene.size(); i++)
	{

		if (GameScene[i]->type == BULLET) {
			GameScene[i]->location.x += 0.1 * GameScene[i]->movingDirection.x;
			GameScene[i]->location.y += 0.1 * GameScene[i]->movingDirection.y;
			GameScene[i]->location.z += 0.1 * GameScene[i]->movingDirection.z;
		}

		if (GameScene[i]->type == ENEMY) {

			if (GameScene[i]->location.x > 0) {
				GameScene[i]->location.x -= 0.005;
				GameScene[i]->rotation = glm::vec3(-1, 0, 0);
			}
			else if (GameScene[i]->location.x < 0) {
				GameScene[i]->location.x += 0.005;
				GameScene[i]->rotation = glm::vec3(1, 0, 0);

			}


			if (GameScene[i]->location.y > 0) {
				GameScene[i]->location.y -= 0.005;
				GameScene[i]->rotation = glm::vec3(0, -1, 0);

			}
			else if (GameScene[i]->location.y < 0) {
				GameScene[i]->location.y += 0.005;
				GameScene[i]->rotation = glm::vec3(0, 1, 0);
			}

			if (1 < GameScene[i]->location.x < 1 && 1 < GameScene[i]->location.y < 1) {
				GameScene[i]->isAlive = false;
			}

		}
		//cout << GameScene[i]->lifeSpan << endl;

	}
}




void drawObjects() {

	updateScene();


	for (int i = 0; i < GameScene.size(); i++)
	{
		if (GameScene[i]->isAlive) {

			if (GameScene[i]->type == BULLET) {
				model_view = glm::mat4(1.0);
				model_view = glm::translate(model_view, GameScene[i]->location);
				//model_view = glm::translate(model_view, glm::vec3(alpha / 1.0f, 0.0f, 0.0f));

				glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
				drawCube(model_view[0][0] * 0.1);
				glBindTexture(GL_TEXTURE_2D, texture[2]);
				glDrawArrays(GL_QUADS, 4, 24);
			}
			else if (GameScene[i]->type == ENEMY && GameScene[i]->isAlive == true) {
				model_view = glm::mat4(1.0);


				model_view = glm::translate(model_view, GameScene[i]->location);

				//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
				drawCube(model_view[0][0]);
				glBindTexture(GL_TEXTURE_2D, texture[1]);
				glDrawArrays(GL_QUADS, 4, 24);
				glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);



				glm::mat4 temp = model_view;

				model_view = glm::translate(model_view, glm::vec3(-0.45f, 0.45f, 0.0f));
				model_view = glm::rotate(model_view, alpha / 1000, GameScene[i]->rotation);
				model_view = glm::scale(model_view, glm::vec3(0.5f, 0.5f, 1.0f));
				//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
				drawCube(model_view[0][0]);
				glBindTexture(GL_TEXTURE_2D, texture[2]);
				glDrawArrays(GL_QUADS, 4, 24);

				model_view = temp;

				model_view = glm::translate(model_view, glm::vec3(0.45f, -0.45f, 0.0f));
				model_view = glm::rotate(model_view, alpha / 1000, GameScene[i]->rotation);
				model_view = glm::scale(model_view, glm::vec3(0.5f, 0.5f, 1.0f));
				//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
				drawCube(model_view[0][0]);
				glBindTexture(GL_TEXTURE_2D, texture[2]);
				glDrawArrays(GL_QUADS, 4, 24);

				model_view = temp;

				model_view = glm::translate(model_view, glm::vec3(0.45f, 0.45f, 0.0f));
				model_view = glm::rotate(model_view, alpha / 1000, GameScene[i]->rotation);
				model_view = glm::scale(model_view, glm::vec3(0.5f, 0.5f, 1.0f));
				//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
				drawCube(model_view[0][0]);
				glBindTexture(GL_TEXTURE_2D, texture[2]);
				glDrawArrays(GL_QUADS, 4, 24);

				model_view = temp;

				model_view = glm::translate(model_view, glm::vec3(-0.45f, -0.45f, 0.0f));
				model_view = glm::rotate(model_view, alpha / 1000, GameScene[i]->rotation);
				model_view = glm::scale(model_view, glm::vec3(0.5f, 0.5f, 1.0f));
				//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
				drawCube(model_view[0][0]);
				glBindTexture(GL_TEXTURE_2D, texture[2]);
				glDrawArrays(GL_QUADS, 4, 24);
			}
		}
	}
}
/*
void updateScene2() {

	for (int i = 0; i < GameScene2.size(); i++)
	{

		if (GameScene2[i]->location.x > 0) {
			GameScene2[i]->location.x -= 0.01;
		}
		else {
			GameScene2[i]->location.x += 0.01;

		}

		if (GameScene2[i]->location.y > 0) {
			GameScene2[i]->location.y -= 0.01;
		}
		else {
			GameScene2[i]->location.y += 0.01;

		}

		//GameScene2[i]->location.x += 0.01 * GameScene2[i]->movingDirection.x;
		//GameScene2[i]->location.y += 0.01 * GameScene2[i]->movingDirection.y;
		//GameScene2[i]->location.z += 0.01 * GameScene2[i]->movingDirection.z;

	}
}
*/

//---------------------------------------------------------------------
//
// display
//
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	model_view = glm::mat4(1.0);
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	//model_view = glm::translate(model_view, glm::vec3(alpha / 1.0f, 0.0f, 0.0f));


	//The 3D point in space that the camera is looking
	glm::vec3 look_at = cam_pos + looking_dir_vector;

	glm::mat4 camera_matrix = glm::lookAt(cam_pos, look_at, up_vector);
	glUniformMatrix4fv(cam_mat_location, 1, GL_FALSE, &camera_matrix[0][0]);

	glm::mat4 proj_matrix = glm::frustum(-0.01f, +0.01f, -0.01f, +0.01f, 0.01f, 100.0f);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &proj_matrix[0][0]);

	draw_level();


	/*for (int i = 0; i < Num_Obstacles; i++) {

		model_view = glm::mat4(1.0);


		model_view = glm::translate(model_view, glm::vec3(obstacle_data[i][0], obstacle_data[i][1], 0.0));

		//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
		drawCube(model_view[0][0]);
		glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);



		glm::mat4 temp = model_view;

		model_view = glm::translate(model_view, glm::vec3(-0.45f, 0.45f, 0.0f));
		model_view = glm::rotate(model_view, alpha / 1000, glm::vec3(0.0f, 1.0f, 0.0f));
		model_view = glm::scale(model_view, glm::vec3(0.5f, 0.5f, 1.0f));
		//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
		drawCube(model_view[0][0]);

		model_view = temp;

		model_view = glm::translate(model_view, glm::vec3(0.45f, -0.45f, 0.0f));
		model_view = glm::rotate(model_view, alpha / 1000, glm::vec3(0.0f, 1.0f, 0.0f));
		model_view = glm::scale(model_view, glm::vec3(0.5f, 0.5f, 1.0f));
		//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
		drawCube(model_view[0][0]);


		model_view = temp;

		model_view = glm::translate(model_view, glm::vec3(0.45f, 0.45f, 0.0f));
		model_view = glm::rotate(model_view, alpha / 1000, glm::vec3(0.0f, 1.0f, 0.0f));
		model_view = glm::scale(model_view, glm::vec3(0.5f, 0.5f, 1.0f));
		//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
		drawCube(model_view[0][0]);


		model_view = temp;

		model_view = glm::translate(model_view, glm::vec3(-0.45f, -0.45f, 0.0f));
		model_view = glm::rotate(model_view, alpha / 1000, glm::vec3(0.0f, 1.0f, 0.0f));
		model_view = glm::scale(model_view, glm::vec3(0.5f, 0.5f, 1.0f));
		//glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
		drawCube(model_view[0][0]);

	}*/

	drawObjects();

	glFlush();
}


void keyboard(unsigned char key, int x, int y)
{
	if (key == 'a')
	{
		//Moving camera along opposit direction of side vector
		cam_pos += side_vector * travel_speed * ((float)deltaTime) / 1000.0f;
	}
	if (key == 'd')
	{
		//Moving camera along side vector
		cam_pos -= side_vector * travel_speed * ((float)deltaTime) / 1000.0f;
	}
	if (key == 'w')
	{
		//Moving camera along forward vector. To be more realistic, we use X=V.T equation in physics
		cam_pos += forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
	}
	if (key == 's')
	{
		//Moving camera along backward (negative forward) vector. To be more realistic, we use X=V.T equation in physics
		cam_pos -= forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
	}
	if (key == 'f')
	{
		GameObjects* go = new GameObjects();
		(*go).location = cam_pos;
		(*go).movingDirection = looking_dir_vector;
		(*go).movingDirection = glm::normalize((*go).movingDirection);
		(*go).isAlive = true;
		(*go).type = BULLET;
		(*go).velocity = 0.01;
		(*go).collider_dimension = 0.01;
		(*go).isCollided = false;

		GameScene.push_back(go);
	}
}

//Controlling Pitch with vertical mouse movement
void mouse(int x, int y)
{
	//Controlling Yaw with horizontal mouse movement
	int delta_x = x - x0;

	//The following vectors must get updated during a yaw movement
	forward_vector = glm::rotate(forward_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	looking_dir_vector = glm::rotate(looking_dir_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	side_vector = glm::rotate(side_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	up_vector = glm::rotate(up_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	x0 = x;

	//The following vectors must get updated during a pitch movement
	int delta_y = y - y_0;
	glm::vec3 tmp_up_vec = glm::rotate(up_vector, delta_y * mouse_sensitivity, side_vector);
	glm::vec3 tmp_looking_dir = glm::rotate(looking_dir_vector, delta_y * mouse_sensitivity, side_vector);

	//The dot product is used to prevent the user from over-pitch (pitching 360 degrees)
	//The dot product is equal to cos(theta), where theta is the angle between looking_dir and forward vector
	GLfloat dot_product = glm::dot(tmp_looking_dir, forward_vector);

	//If the angle between looking_dir and forward vector is between (-90 and 90) degress 
	if (dot_product > 0)
	{
		up_vector = glm::rotate(up_vector, delta_y * mouse_sensitivity, side_vector);
		looking_dir_vector = glm::rotate(looking_dir_vector, delta_y * mouse_sensitivity, side_vector);
	}
	y_0 = y;
}



void idle()
{
	//Calculating the delta time between two frames
	//We will use this delta time when moving forward (in keyboard function)
	float omega = 1;	//let omega to be the angular speed

	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = timeSinceStart - oldTimeSinceStart;
	oldTimeSinceStart = timeSinceStart;
	alpha = omega * timeSinceStart * 2;		//We use Theta = omega * time to calculate the amount of rotation of the geometry in this frame


	if (GameScene.size() < 100 && (timeSinceStart % 2000 == 0)) {
		GameObjects* go1 = new GameObjects();
		(*go1).type = ENEMY;
		(*go1).isAlive = true;
		(*go1).location.x = randomFloat(-50, 50);
		(*go1).location.y = randomFloat(-50, 50);
		(*go1).collider_dimension = obstacle_data[1][2] / 4.5;
		(*go1).living_time = 0;
		(*go1).movingDirection = glm::normalize((*go1).movingDirection);
		(*go1).isCollided = false;


		GameScene.push_back(go1);
	}


	for (int i = 0; i < GameScene.size(); i++) {

		if (timeSinceStart - GameScene[i]->lifeSpan >= 2000 && GameScene[i]->type == BULLET) {
			GameScene[i]->isAlive = false;
		}

		if (GameScene[i]->type == ENEMY && -0.1 < GameScene[i]->location.x < 0.1 && -0.1 < GameScene[i]->location.y < 0.1)
		{
			GameScene[i]->isAlive = false;
		}

		if (GameScene[i]->type == PLAYER) {
			GameScene[i]->location = cam_pos;
		}
	}


	

	//drawObjects();
	//updateScene();
	//updateScene2();
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
	glutCreateWindow("Camera and Projection");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init();

	glutDisplayFunc(display);

	glutKeyboardFunc(keyboard);

	glutIdleFunc(idle);

	glutPassiveMotionFunc(mouse);

	glutMainLoop();



}
