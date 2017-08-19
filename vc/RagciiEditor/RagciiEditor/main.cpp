#include <SDL.h>
#include <GL/glew.h>

#include <iostream>
#include <fstream>
#include <string>

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;
static GLuint gProgram;
static GLuint gVBO;

SDL_Window* gWindow = nullptr;

void mainloop();

void pressAnyKeyToContinue()
{
	std::cin.get();
}

int showSDLErrorAndExit()
{
	std::cerr << SDL_GetError() << std::endl;
	pressAnyKeyToContinue();
	return 1;
}

std::string readFileToString(std::string filePath)
{
	std::ifstream ifs(filePath);
	std::string content((std::istreambuf_iterator<char>(ifs)),
						(std::istreambuf_iterator<char>()));
	return content;
}

void printProgramLog(GLuint program)
{
	//Make sure name is shader
	if (glIsProgram(program))
	{
		//Program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char* infoLog = new char[maxLength];

		//Get info log
		glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0)
		{
			//Print Log
			printf("%s\n", infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf("Name %d is not a program\n", program);
	}
}
void printShaderLog(GLuint shader)
{
	//Make sure name is shader
	if (glIsShader(shader))
	{
		//Shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char* infoLog = new char[maxLength];

		//Get info log
		glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0)
		{
			//Print Log
			printf("%s\n", infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf("Name %d is not a shader\n", shader);
	}
}

int main(int argc, char** argv)
{
	// init sdl
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) return showSDLErrorAndExit();

	// init sdl windows
	gWindow = SDL_CreateWindow(
		"RagciiEditor",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (gWindow == nullptr) return showSDLErrorAndExit();

	////////////////////////////////////////////////////////////
	// set up sdl opengl features
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	// init sdl gl context
	SDL_GLContext glContext = SDL_GL_CreateContext(gWindow);
	if (glContext == nullptr) return showSDLErrorAndExit();

	// init glew
	glewExperimental = GL_TRUE; // for Opengl 3.0 or later
	GLenum glewInitResult = glewInit();
	if (glewInitResult != GLEW_OK) {
		std::cerr << "Failed to init glew : " << glewGetErrorString(glewInitResult) << std::endl;
		pressAnyKeyToContinue();
		return 1;
	}

	///////////////////////
	// render a texture
	// define the clear color
	glClearColor(0.2, 0.2, 0.2, 1.0);

	// create a fragment shader with coordination and sample2d past by vertex shader
	std::string vertexShaderSourceStr = "#version 130\nvoid main(){\ngl_Position = vec4(0, 0, 0, 1);\n}";
	//const GLchar* vertexShaderSource = readFileToString("assets/shaders/shader.vert").c_str();
	const GLchar* vertexShaderSource = vertexShaderSourceStr.c_str();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
	glCompileShader(vertexShader);

	//Check vertex shader for errors
	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if (vShaderCompiled != GL_TRUE)
	{
		printf("Unable to compile vertex shader %d!\n", vertexShader);
		printShaderLog(vertexShader);
		pressAnyKeyToContinue();
		return 1;
	}

	std::string fragShaderSourceStr = "#version 130\nprecision highp float;\nout vec4 fragColor;\nvoid main(){\nfragColor = vec4(1.0, 0.0, 0.0, 1.0);\n}";
	const GLchar* fragShaderSource = fragShaderSourceStr.c_str();
	//const GLchar* fragShaderSource = readFileToString("assets/shaders/shader.vert").c_str();
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragShaderSource, nullptr);
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if (vShaderCompiled != GL_TRUE)
	{
		printf("Unable to compile fragment shader %d!\n", fragShader);
		printShaderLog(fragShader);
		pressAnyKeyToContinue();
		return 1;
	}


	// create program
	gProgram = glCreateProgram();

	// create shaders
	glAttachShader(gProgram, vertexShader);
	glAttachShader(gProgram, fragShader);

	// link program
	glLinkProgram(gProgram);

	//Check for errors
	GLint programSuccess = GL_TRUE;
	glGetProgramiv(gProgram, GL_LINK_STATUS, &programSuccess);
	if (programSuccess != GL_TRUE)
	{
		printf("Error linking program %d!\n", gProgram);
		printProgramLog(gProgram);
		pressAnyKeyToContinue();
		return 1;
	}

	// define array with quad vertex and texture coord data
	float vertexData[] = {
		0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	// create buffer
	glGenBuffers(1, &gVBO);
	
	// active buffer
	glBindBuffer(GL_ARRAY_BUFFER, gVBO);

	// put content in buffer data
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 4, vertexData, GL_STATIC_DRAW);

	// get position location
	//glGetAttribLocation(program, "");
	// bind position location with buffer data
	// get texture coord location
	// bind 
	// get sample2d location
	// read the image file
	// bind sample2d location with texture
	// set the mag and min of opengl texture

	mainloop();
	// in mainloop
	// get sdl event
	// clear the buffer with clear color
	// draw the array with triangle strip
	// swap draw buffer
	return 0;
}

void mainloop()
{
	bool loop = true;
	while (loop)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT) {
				loop = false;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(gProgram);
		//glEnableVertexArrayAttrib(gPosition);
		glBindBuffer(GL_ARRAY_BUFFER, gVBO);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, NULL);
		SDL_GL_SwapWindow(gWindow);
	}

	//Destroy window	
	SDL_DestroyWindow(gWindow);
	gWindow = nullptr;

	//Quit SDL subsystems
	SDL_Quit();
}