//
//  main.cpp
//  OpenGL Shadows
//
//  Created by CGIS on 05/12/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#include <windows.h>
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"

//<mouse>
float lastX = 400, lastY = 300;
bool firstMouse = true;
float yaw = -90, pitch = 0;

//</mouse>

int glWindowWidth = 640;
int glWindowHeight = 480;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;
bool flag = false;
const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;
GLuint difureTextureLoc;
glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

glm::vec3 skyBoxLight;
GLuint skyBoxLightLoc;

gps::Camera myCamera(glm::vec3(80.0f, 20.0f, 100.0f), glm::vec3(0.0f, 20.0f, 100.0f), model);
//gps::Camera myPlayer(glm::vec3(0.0f, 0.0f,0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
GLfloat cameraSpeed = 0.8f;
float xpos=-5.0f, zpos = -5.0f;
bool pressedKeys[1024];
GLfloat angle;
GLfloat lightAngle;

gps::Model3D myModel;
gps::Model3D ground;
gps::Model3D lightCube;
//<model>
gps::Model3D tree1;
gps::Model3D tree2;
gps::Model3D tree3;
gps::Model3D tree4;
gps::Model3D rain;
gps::Model3D wolf;
gps::Model3D wolf3;
gps::Model3D mountain;
gps::Model3D river;
gps::Model3D stoneRoad;
gps::Model3D bigtree;
gps::Model3D road;
gps::Model3D cloud;
gps::Model3D stone;
gps::Model3D stones;
gps::Model3D plant;
//gps::Model3D valley;
gps::Model3D bridge;
//</model>
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

//<texture>
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;
std::vector<const GLchar*> faces;
//</texture>
GLuint treeTexture, wolfTexture;
gps::Shader objectShader;
glm::mat4 projection_o;
GLint projLoc_o;
GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	
	lightShader.useShaderProgram();
	
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//new
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.30;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);

	//gps::Camera::t = glm::normalize(front);
	//new
	
}


bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "Project PADURE", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouse_callback);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initFBOs()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix()
{
	const GLfloat near_plane = 1.0f, far_plane = 10.0f;
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(myCamera.getCameraTarget() + 1.0f * lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}

void initModels()
{
	//myModel = gps::Model3D("objects/nanosuit/nanosuit.obj", "objects/nanosuit/");
	ground = gps::Model3D("objects/ground/ground.obj", "objects/ground/");
	lightCube = gps::Model3D("objects/cube/cube.obj", "objects/cube/");
	tree1 = gps::Model3D("objects/trees/tree1.obj", "objects/trees/");
	tree2 = gps::Model3D("objects/trees/tree2.obj", "objects/trees/");
	tree3 = gps::Model3D("objects/trees/tree3.obj", "objects/trees/");
	tree4 = gps::Model3D("objects/trees/tree4.obj", "objects/trees/");
	wolf3 = gps::Model3D("objects/wolf3/wolf.obj", "objects/wolf3/");
	stone = gps::Model3D("objects/stone/stone.obj", "objects/stone/");
	plant = gps::Model3D("objects/stone/plant.obj", "objects/stone/");
	stones = gps::Model3D("objects/stone/stones.obj", "objects/stone/");
	stoneRoad = gps::Model3D("objects/ground/stoneRoad.obj", "objects/ground/");
	bridge = gps::Model3D("objects/bridge/untitled.obj", "objects/bridge/");

//	mountain = gps::Model3D("objects/mountain/mount2.obj", "objects/mountain/");
	river = gps::Model3D("objects/ground/river.obj", "objects/ground/");
	//road = gps::Model3D("objects/road/road.obj", "objects/road/");
	rain = gps::Model3D("objects/rain/rain.obj", "objects/rain/");
	cloud = gps::Model3D("objects/cloud/cloud.obj", "objects/cloud/");
	//bigtree = gps::Model3D("objects/tree/Tree2.obj", "objects/tree/");

}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	objectShader.loadShader("shaders/object.vert", "shaders/object.frag");
}
int a = 0;
glm::vec3 direction(15, -1, -15);
float turnangle = 0.0f;
float riv = 0;
float lightR = 1.0f, lightG = 1.0f;
void initUniforms()
{
	myCustomShader.useShaderProgram();

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	
	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 2.0f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(lightR, lightG, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	skyboxShader.useShaderProgram();


	skyBoxLight = glm::vec3(lightR, lightG, 1.0f);
	skyBoxLightLoc = glGetUniformLocation(skyboxShader.shaderProgram, "light");
	glUniform3fv(skyBoxLightLoc, 1, glm::value_ptr(skyBoxLight));

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


}
bool flag2 = true;
void processMovement()
{

	if (pressedKeys[GLFW_KEY_Q]) {
		lightR = 0; lightG = 0;
		initUniforms();
	}
	if (pressedKeys[GLFW_KEY_E])
	{
		lightR = 1; lightG = 1;
		initUniforms();
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_I]) {
		flag = !flag;
	}
	if (pressedKeys[GLFW_KEY_O]) {
		riv -= 0.3f;
	}
	riv += 0.05f;
	if (riv > 5)
		riv = 0;
	if (pressedKeys[GLFW_KEY_J]) {

		lightAngle += 0.3f;
		if (lightAngle > 360.0f)
			lightAngle -= 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle -= 0.3f;
		if (lightAngle < 0.0f)
			lightAngle += 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}
	if (pressedKeys[GLFW_KEY_T]) {//up

		direction.z += 0.1f;
	}
	if (pressedKeys[GLFW_KEY_F]) {//left
		direction.x += 0.1f;
		turnangle += 5.0f;
	}
	if (pressedKeys[GLFW_KEY_G]) {//down
		direction.z -= 0.1f;
	}
	if (pressedKeys[GLFW_KEY_H]) {//right
		direction.x -= 0.1f;
		turnangle -= 5.0f;
	}
	if (pressedKeys[GLFW_KEY_X]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	}
	if (pressedKeys[GLFW_KEY_C]) {//right
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	if (pressedKeys[GLFW_KEY_Z])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (pressedKeys[GLFW_KEY_Y])
		flag2 = false;
	if (pressedKeys[GLFW_KEY_U])
	myCamera.rotate(0, -90);
}
void drawObject(gps::Model3D obj, float tx, float ty, float tz, float sx, float sy, float sz)
{

	//create model matrix for copac
	model = glm::translate(glm::mat4(1.0f), glm::vec3(tx, ty, tz));
	model = glm::scale(model, glm::vec3(sx, sy, sz));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	obj.Draw(myCustomShader);
}

void drawNanoSuitShadow()
{
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	//create model matrix for nanosuit
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));

	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	wolf3.Draw(depthMapShader);
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(25.0f, 25.0f, 25.0f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	ground.Draw(depthMapShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void secondPass()
{
	myCustomShader.useShaderProgram();

	//send lightSpace matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	//send view matrix to shader
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
		1,
		GL_FALSE,
		glm::value_ptr(view));

	//compute light direction transformation matrix
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	//send lightDir matrix data to shader
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);
	myCustomShader.useShaderProgram();

	//bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
}
void rain_an()
{
	if (flag == true)
	{
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				drawObject(rain, 20 + 9 * i, 5 + rand() % 30,  - 7 * j, 2, 2, 2);
			}
		}
	}

}
void drawBridge(gps::Model3D obj, float tx, float ty, float tz, float sx, float sy, float sz)
{

	//create model matrix for copac
	model = glm::translate(glm::mat4(1.0f), glm::vec3(tx, ty, tz));
	model = glm::scale(model, glm::vec3(sx, sy, sz));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	obj.Draw(myCustomShader);
}
void drawNanoSuit()
{
	//create model matrix for nanosuit
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0,4,0));
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::scale(model, glm::vec3(2, 2, 2));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	myModel.Draw(myCustomShader);
}

void drawWolf(float tx, float ty, float tz, float sx, float sy, float sz,  glm::vec3  direction)
{
	//create model matrix for copac
	model = glm::translate(glm::mat4(1.0f), glm::vec3(direction.x, direction.y, direction.z));
	model = glm::scale(model, glm::vec3(sx, sy, sz));
	//model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
	//model = glm::rotate(model, glm::radians(turnangle), glm::vec3(0, 1, 0)*myCamera.cameraDirection);

	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	wolf3.Draw(myCustomShader);
}
void renderScene(float randx, float randy, float randz)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	processMovement();	
	glfwSetCursorPosCallback(glWindow, mouse_callback);




	drawNanoSuitShadow();
	secondPass();
	//drawNanoSuit();

	drawWolf(0, -1.0f, 0, 4, 4, 4, direction);

	//<trees>
	for (int i = 1; i < 5; i++)
	{
		for (int j = 1; j < 5; j++)
		{
			drawObject(tree1, 20 * i, -1.0f, 20 * j, 3, 4, 3);
			drawObject(tree1, -20 * i, -1.0f, 20 * j, 3, 3, 3);
			drawObject(tree1, 20 * i, -1.0f, -20 * j, 3, 4, 3);
			drawObject(tree1, -20 * i, -1.0f, -20 * j, 3, 4, 3);

			drawObject(tree1, 8 + 20 * i, -1.0f, 5 + 20 * j, 3, 3, 3);
			drawObject(tree1, 8 - 20 * i, -1.0f, 5 + 20 * j, 3, 4, 3);
			drawObject(tree1, 8 + 20 * i, -1.0f, 5 - 20 * j, 3, 4, 3);
			drawObject(tree1, 8 - 20 * i, -1.0f, 5 - 20 * j, 3, 3, 3);

		}
	}

	//</trees>




	for (int i = 0; i < 5; i++)
	{
		drawObject(river, 0, -0.95f, 20*i+riv, 1, 1, 1);
		drawObject(river,0, -0.95f, -20*i+riv, 1, 1, 1);
	}
	for (int i = 1; i < 5; i++)
	{
		drawObject(stoneRoad, 20*i, -0.95f, 0, 1, 1, 1);
		drawObject(stoneRoad, -20*i, -0.95f, 0, 1, 1, 1);
	}
	for (int i = 1; i < 7; i++)
	{
		drawObject(stone, -10, -0.5f, 11 * i, randx/3, randy/3, randz/3);
		drawObject(stone,  9, -0.5f, -11 * i, randy / 3, randz / 3, randx / 3);
		drawObject(stone, -10, -0.5f, -11 * i, randx / 3, randy / 3, randz / 3);
		drawObject(stone, 9, -0.5f, 11 * i, randy / 3, randz / 3, randx / 3);
	}
	
	for (int i = 1; i < 10; i++)
	{
		drawObject(plant, -10+10*i, -6, -10, 1.4, 1.4, 1.4);
		if (i>1)
			drawObject(plant, -10-10*i, -6,-10, 1.4, 1.4, 1.4);
		drawObject(plant, -10 + 10 * i, -6, 15, 1.4, 1.4, 1.4);
		if (i > 1)
		drawObject(plant, -10 - 10 * i, -6, 15, 1.4, 1.4, 1.4);
	}
	
	
	drawObject(ground, 0, -1.0f, 0, 10, 10, 10);

	drawObject(cloud, 20, 40, -20, 6.0f, 3.0f, 6.0f);

	drawObject(stones, 0, -6,-88, 5, 5, 6);
	drawObject(stones, 0, -6, 95, 5, 4, 5);
	drawObject(stones, -5, -6.0f, +50, 3, 2, 3);
	drawObject(stones, +5, -6.0f, -44, 3, 2, 2);
	drawBridge(bridge,-0, 0, 0, 0.050, 0.025, 0.060);


	
	rain_an();

	
	//draw a white cube around the light



	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, 120.0f * lightDir);
	model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	lightCube.Draw(lightShader);


	mySkyBox.Draw(skyboxShader, view, projection);
}
//glm::vec3 a(-80, 20, 100);
void intro(bool flag2)
{
	if (flag2 == true)
	{
		for (int i = 0; i < 100000; i++)
		{
			//Sleep(1);
			myCamera.move(gps::MOVE_FORWARD, 0.00001f);
			//if (myCamera.cameraPosition.x==0 || myCamera.cameraPosition.y == 0)
				
			//	myCamera.rotate(0, -90);

		}
		//if (myCamera.cameraPosition.x == 0)
		//{
			//m
		//}
			
		for (int i = 0; i < 100000; i++)
		{

			myCamera.move(gps::MOVE_FORWARD, 0.00001f);

		}
		
	}
	

}
int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initFBOs();
	initModels();
	initShaders();
	
	initUniforms();	
	glCheckError();
	


	faces.push_back("textures/skybox/st_ft.tga");
	faces.push_back("textures/skybox/st_bk.tga");

	
	faces.push_back("textures/skybox/st_up.tga");
	faces.push_back("textures/skybox/st_dn.tga");

	faces.push_back("textures/skybox/st_rt.tga");
	faces.push_back("textures/skybox/st_lf.tga");

	

	
	float randx = 1 + rand() % 3;
	float randy = 1 + rand() % 3;
	float randz = 1 + rand() % 3;
	
	mySkyBox.Load(faces);
	
	while (!glfwWindowShouldClose(glWindow)) {
		
		renderScene(randx, randy, randz);
		intro(flag2);
		glfwPollEvents();
		glfwSwapBuffers(glWindow);

	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
/*
drawObject(bigtree, 30,-1,30,2,2,2);
for (int i = 0; i < 3; i++)
{
	for (int j = 0; j < 3; j++)
	{
		drawObject(bigtree, 50 + 70.0f * i, -1, 50 + 70.0f * j, 2, 2, 2);
	}
}
*/