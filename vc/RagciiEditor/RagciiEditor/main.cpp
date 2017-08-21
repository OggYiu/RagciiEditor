#include <SDL.h>
#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H    

#include <iostream>
#include <fstream>
#include <string>

static const int SCREEN_WIDTH = 400;
static const int SCREEN_HEIGHT = 400;
static GLuint gProgram;
static GLuint gVBO;
static GLuint gIBO;
static GLuint gTex;
GLint gVertexPos2DLocation = -1;
GLint gUniformTex;
//GLint gUniformColor;

FT_Library gFtLibrary;
FT_Face gFtFace;

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

//bool writeFile(unsigned char* data, const int size)
//{
//	char* buffer = new char[size];
//
//	std::ofstream myFile("yiu.bmp", std::ios::out | std::ios::binary);
//	myFile.write(buffer, size);
//	myFile.close();
//
//	delete[] buffer;
//	return true;
//}

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

/**
* Store all the file's contents in memory, useful to pass shaders
* source code to OpenGL.  Using SDL_RWops for Android asset support.
*/
char* file_read(const char* filename, int* size) {
	SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
	if (rw == NULL) return NULL;

	Sint64 res_size = SDL_RWsize(rw);
	char* res = (char*)malloc(res_size + 1);

	Sint64 nb_read_total = 0, nb_read = 1;
	char* buf = res;
	while (nb_read_total < res_size && nb_read != 0) {
		nb_read = SDL_RWread(rw, buf, 1, (res_size - nb_read_total));
		nb_read_total += nb_read;
		buf += nb_read;
	}
	SDL_RWclose(rw);
	if (nb_read_total != res_size) {
		free(res);
		return NULL;
	}

	res[nb_read_total] = '\0';
	if (size != NULL)
		*size = nb_read_total;
	return res;
}
/**
* Store all the file's contents in memory, useful to pass shaders
* source code to OpenGL
*/
char* file_read(const char* filename)
{
	FILE* in = fopen(filename, "rb");
	if (in == NULL) return NULL;

	int res_size = BUFSIZ;
	char* res = (char*)malloc(res_size);
	int nb_read_total = 0;

	while (!feof(in) && !ferror(in)) {
		if (nb_read_total + BUFSIZ > res_size) {
			if (res_size > 10 * 1024 * 1024) break;
			res_size = res_size * 2;
			res = (char*)realloc(res, res_size);
		}
		char* p_res = res + nb_read_total;
		nb_read_total += fread(p_res, 1, BUFSIZ, in);
	}

	fclose(in);
	res = (char*)realloc(res, nb_read_total + 1);
	res[nb_read_total] = '\0';
	return res;
}

void initTexture()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);

	// We require 1 byte alignment when uploading texture data
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Generate a texture object
	glGenTextures(1, &gTex);

	// Bind the texture object
	glBindTexture(GL_TEXTURE_2D, gTex);

	//FT_Set_Pixel_Sizes(gFtFace, 0, 60);

	//// Load the texture
	//FT_GlyphSlot g = gFtFace->glyph;
	//if (FT_Load_Char(gFtFace, 'A', FT_LOAD_RENDER))
	//{
	//	std::cerr << "Failed to load char A with freetype2" << std::endl;
	//	pressAnyKeyToContinue();
	//	exit(1);
	//}

	// 2 x 2 Image, 3 bytes per pixel(R, G, B)
	//GLubyte pixels[4 * 3] =
	//{
	//	255, 0, 0, // Red
	//	0, 255, 0, // Green
	//	0, 0, 255, // Blue
	//	255, 255, 0 // Yellow
	//};

	//std::cout << "text size : " << g->bitmap.width << ", " << g->bitmap.rows << std::endl;
/*
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g->bitmap.width, g->bitmap.rows, 0, GL_RGB,
		GL_UNSIGNED_BYTE, g->bitmap.buffer);*/

	FT_UInt  glyph_index;

	/* retrieve glyph index from character code */
	glyph_index = FT_Get_Char_Index(gFtFace, 'A');

	/* load glyph image into the slot (erase previous one) */
	bool error = FT_Load_Glyph(gFtFace, glyph_index, FT_LOAD_DEFAULT);
	if (error)
	{
		std::cout << "Failed to FT_Load_Glyph" << std::endl;
		pressAnyKeyToContinue();
		exit(1);
	}

	// convert to an anti-aliased bitmap
	error = FT_Render_Glyph(gFtFace->glyph, FT_RENDER_MODE_NORMAL);
	if (error)
	{
		std::cout << "Failed to FT_Render_Glyph" << std::endl;
		pressAnyKeyToContinue();
		exit(1);
	}

	FT_GlyphSlot g = gFtFace->glyph;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g->bitmap.width, g->bitmap.rows, 0, GL_LUMINANCE_ALPHA,
		GL_UNSIGNED_BYTE, g->bitmap.buffer);

	// Clamping to edges is important to prevent artifacts when scaling
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Linear filtering usually looks best for text
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glUniform1i(gUniformTex, 0);

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
	SDL_SetWindowResizable(gWindow, SDL_TRUE);

	////////////////////////////////////////////////////////////
	// set up sdl opengl features
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

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
	//std::string vertexShaderSourceStr = "#version 130\nvoid main(){\ngl_Position = vec4(0, 0, 0, 1);\ngl_PointSize = 10.0;\n}";
	//const GLchar* vertexShaderSource = readFileToString("assets/shaders/shader.vert").c_str();

	const GLchar* vertexShaderSource = file_read("assets/shaders/shader.vert");
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

	const GLchar* fragShaderSourceStr = file_read("assets/shaders/shader.frag");
	//const GLchar* fragShaderSource = readFileToString("assets/shaders/shader.vert").c_str();
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragShaderSourceStr, nullptr);
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
	if (gProgram == 0)
	{
		std::cerr << "Failed to create program" << std::endl;
		pressAnyKeyToContinue();
		return 1;
	}

	// create shaders
	glAttachShader(gProgram, vertexShader);
	if (glGetError() != GL_NO_ERROR) {
		GLenum glError = glGetError();
		std::cerr << "Failed to attach vertexShader : " << glError << std::endl;
		pressAnyKeyToContinue();
		return 1;
	}

	glAttachShader(gProgram, fragShader);
	if (glGetError() != GL_NO_ERROR) {
		GLenum glError = glGetError();
		std::cerr << "Failed to attach fragShader : " << glError << std::endl;
		pressAnyKeyToContinue();
		return 1;
	}

	{
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
	}

	glUseProgram(gProgram);

	//Get vertex attribute location
	gVertexPos2DLocation = glGetAttribLocation(gProgram, "aPosition");
	if (gVertexPos2DLocation == -1)
	{
		printf("aPosition is not a valid glsl program variable!\n");
		pressAnyKeyToContinue();
		return 1;
	}

	gUniformTex = glGetUniformLocation(gProgram, "uTex");
	if (gUniformTex == -1)
	{
		printf("uTex is not a valid glsl program variable!\n");
		pressAnyKeyToContinue();
		return 1;
	}
/*
	gUniformColor = glGetUniformLocation(gProgram, "uColor");
	if (gUniformColor == -1)
	{
		printf("uColor is not a valid glsl program variable!\n");
		pressAnyKeyToContinue();
		return 1;
	}*/

	GLfloat black[4] = { 0, 0, 0, 1 };
	GLfloat red[4] = { 1, 0, 0, 1 };
	GLfloat transparent_green[4] = { 0, 1, 0, 0.5 };

	/* Set font size to 48 pixels, color to black */
	FT_Set_Pixel_Sizes(gFtFace, 0, 64);
	//glUniform4fv(gUniformColor, 1, red);

	glUseProgram(NULL);
/*
	GLfloat vertexData[] = {
		-0.5f, -0.5f, 0.0f, 0.0f,
		0.5f, -0.5f, 1.0f, 0.0f,
		0.5f,  0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f, 1.0f,
	};
*/

	float boxSize = 0.5f;
	GLfloat vertexData[] = {
		-boxSize, boxSize, 0.0f, 1.0f,
		-boxSize, -boxSize, 0.0f, 0.0f,
		boxSize, boxSize, 1.0f, 1.0f,
		boxSize, -boxSize, 1.0f, 0.0f,
	};

	// create buffer
	glGenBuffers(1, &gVBO);
	if (glGetError() != GL_NO_ERROR) {
		GLenum glError = glGetError();
		std::cerr << "Failed to gen VBO : " << glError << std::endl;
		pressAnyKeyToContinue();
		return 1;
	}

	// active buffer
	glBindBuffer(GL_ARRAY_BUFFER, gVBO);
	if (glGetError() != GL_NO_ERROR) {
		GLenum glError = glGetError();
		std::cerr << "Failed to bind buffer VBO : " << glError << std::endl;
		pressAnyKeyToContinue();
		return 1;
	}

	// put content in buffer data
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 4, vertexData, GL_STATIC_DRAW);
	if (glGetError() != GL_NO_ERROR) {
		GLenum glError = glGetError();
		std::cerr << "Failed to do buffer data for VBO : " << glError << std::endl;
		pressAnyKeyToContinue();
		return 1;
	}

	//IBO data
	GLuint indexData[] = { 0, 1, 2, 3 };
	//Create IBO
	glGenBuffers(1, &gIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), indexData, GL_STATIC_DRAW);

	// init freetype
	{
		bool error = FT_Init_FreeType(&gFtLibrary);
		if (error) {
			std::cerr << "Failed to init FT library" << std::endl;
			pressAnyKeyToContinue();
			return 1;
		}
	}

	{
		bool error = FT_New_Face(
			gFtLibrary,
			"assets/fonts/DwarfFortressVan.ttf",
			0,
			&gFtFace);
		if (error) {
			std::cerr << "Failed to init FT Face" << std::endl;
			pressAnyKeyToContinue();
			return 1;
		}

		//error = FT_Set_Char_Size(
		//	gFtFace,    /* handle to face object           */
		//	0,       /* char_width in 1/64th of points  */
		//	64 * 64,   /* char_height in 1/64th of points */
		//	SCREEN_WIDTH,     /* horizontal device resolution    */
		//	SCREEN_HEIGHT);   /* vertical device resolution      */
		//if (error)
		//{
		//	return 4;
		//}

		error = FT_Set_Pixel_Sizes(gFtFace, 0, 300);
		if (error)
		{
			return 4;
		}
	}

	initTexture();

	mainloop();

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
			//User presses a key
			else if (e.type == SDL_KEYDOWN)
			{
				// handle key press
				switch (e.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					loop = false;
					break;
				}
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);

		//Bind program
		glUseProgram(gProgram);

		//renderText("A", 0, 0);

		//Enable vertex position
		glEnableVertexAttribArray(gVertexPos2DLocation);

		//Set vertex data
		glBindBuffer(GL_ARRAY_BUFFER, gVBO);
		//glVertexAttribPointer(gVertexPos2DLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);
		glVertexAttribPointer(gVertexPos2DLocation, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), NULL);

		//Set index data and render
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, NULL);
		//glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);
		//glDrawArrays(GL_TRIANGLE_FAN, 4, NULL);

		//Disable vertex position
		//glDisableVertexAttribArray(gVertexPos2DLocation);

		//Unbind program
		glUseProgram(NULL);

		SDL_GL_SwapWindow(gWindow);
	}

	//Destroy window	
	SDL_DestroyWindow(gWindow);
	gWindow = nullptr;

	//Deallocate program
	glDeleteProgram(gProgram);

	//Quit SDL subsystems
	SDL_Quit();
}