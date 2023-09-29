#define GL_SILENCE_DEPRECIATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPE 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 640,
		  WINDOW_HEIGHT = 480;

const float BG_RED = 0.404f,
			BG_BLUE = 0.016f,
			BG_GREEN = 0.016f,
			BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
		  VIEWPORT_Y = 0,
		  VIEWPORT_WIDTH = WINDOW_WIDTH,
		  VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
		   F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// game running check
SDL_Window* g_display_window;
bool g_game_is_running = true;
ShaderProgram g_shader_program;
// matrixes
glm::mat4 view_matrix; // position of the camera
glm::mat4 m_model_matrix; // all changes made to object
glm::mat4 m_projection_matrix; // characteristics of camera

// TEXTURE VARIABLES
const char GABRIEL_SPRITE[] = "GabrielBase.png";
GLuint gabriel_texture_id;
const int NUMBER_OF_TEXTURES = 1; // change as needed
const GLint LEVEL_OF_DETAIL = 0; // base image level
const GLint TEXTURE_BORDER = 0; // MUST BE ZERO!!!

// loads textures that are to be rendered
GLuint load_texture(const char* filepath)
{
	// load the image file
	int width, height, number_of_components;
	unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

	// quit if there is no image
	if (image == NULL)
	{
		LOG("Unable to load image. Make sure the path is correct.");
		LOG(filepath);
		assert(false);
	}

	// generating and binding a texture ID to the image
	GLuint textureID;
	glGenTextures(NUMBER_OF_TEXTURES, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

	// sets texture filter parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// release file from memory and return texture id
	stbi_image_free(image);
	
	return textureID;
}

void initialise()
{
	// Create Window
	SDL_Init(SDL_INIT_VIDEO);
	g_display_window = SDL_CreateWindow("HW 1!!!!!",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
	SDL_GL_MakeCurrent(g_display_window, context);

	// for windows machines
#ifdef _WINDOWS
	glewInit();
#endif

	// initailize camera and shaders
	glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_HEIGHT, VIEWPORT_HEIGHT);
	g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

	// Initialize all matrices (above with global variables)
	m_model_matrix = glm::mat4(1.0f);
	view_matrix = glm::mat4(1.0f);
	m_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
	g_shader_program.set_projection_matrix(m_projection_matrix);
	g_shader_program.set_view_matrix(view_matrix);
	glUseProgram(g_shader_program.get_program_id());
	glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

	// Load shaders and image for Gabriel
	gabriel_texture_id = load_texture(GABRIEL_SPRITE);

	// enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
		{
			g_game_is_running = false;
		}
	}
}

void update()
{
	m_model_matrix = glm::mat4(1.0f);
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
	g_shader_program.set_model_matrix(object_model_matrix);
	glBindTexture(GL_TEXTURE_2D, object_texture_id);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT);

	// Vertices
	float vertices[] = {
		-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
		-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
	};

	// Textures
	float texture_coordinates[] = {
		0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
		0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
	};

	glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(g_shader_program.get_position_attribute());

	glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
	glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

	draw_object(m_model_matrix, gabriel_texture_id);

	// We disable two attribute arrays now
	glDisableVertexAttribArray(g_shader_program.get_position_attribute());
	glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

	SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
	SDL_Quit();
}

// call all game functions in main
int main(int argc, char* argv[])
{
	// initialize the game and everything in it
	initialise();

	while (g_game_is_running)
	{
		// check for player inputs
		process_input();

		// update game state every frame 
		update();

		// render all components in scene
		// done after processing input and updating logic
		render();
	}
	shutdown(); // game down, shutdown the game safely
	return 0;
}