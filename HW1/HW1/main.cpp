/**
* Author: Vitoria Tullo
* Assignment: Simple 2D Scene
* Date due: 2023-06-11, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"                // 4x4 Matrix
#include "glm/gtc/matrix_transform.hpp"  // Matrix transformation methods
#include "ShaderProgram.h"               // We'll talk about these later in the course
#include "stb_image.h"

#define LOG(argument) std::cout << argument << '\n'

// window dimensions + viewport
const int WINDOW_WIDTH = 640,
          WINDOW_HEIGHT = 480;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// Background color -- blood red
const float BG_RED = 0.404f,
            BG_BLUE = 0.016f,
            BG_GREEN = 0.016f,
            BG_OPACITY = 1.0f;

// shaders
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

ShaderProgram g_shader_program;

// to display game and check if running
bool g_game_is_running = true;
SDL_Window* g_display_window;

// VVVVV ALL MATRIXES VVVVV
glm::mat4 g_view_matrix,        // position of the camera
g_model_matrix,       // transformations of the object 
g_model_matrix_leftwing,
g_model_matrix_rightwing,
g_model_matrix_leftglow,
g_model_matrix_rightglow,
g_projection_matrix;  // characteristic of the camera

float g_frame_counter = 0.0f; // keeps track of the frames
// change speed constants
const float MAX_FRAME = 4;
const float MOVEMENT_SPEED = 1.0f;
const float GROWTH_FACTOR = .3f;

// for time.deltaTime
float g_previous_ticks = 0.0f;

// TEXTURE VARIABLES
const char GABRIEL_SPRITE[] = "GabrielBaseBIG.png",
           LEFT_WING_SPRITE[] = "GabrielWingLeftBIG.png",
           RIGHT_WING_SPRITE[] = "GabrielWingRightBIG.png",
           LEFT_GLOW_SPRITE[] = "GabrielWingLeftGLOW.png",
           RIGHT_GLOW_SPRITE[] = "GabrielWingRightGLOW.png";

GLuint gabriel_texture_id,
       left_wing_texture_id,
       right_wing_texture_id,
       left_glow_texture_id,
       right_glow_texture_id;

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

// keeps track of transformations
float x_movement = -3.0f,
    y_movement = -2.0f,
    left_flap = 0.0f,
    right_flap = 0.0f,
    growth = 0.0f;

// loads a texture to be used by OpenGL
GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

// initialises the game -- ONLY RUN ONCE AT START
void initialise()
{
    // create window
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("HW 1!!!!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    // for windows machines
#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    // initializes all the model matrixes
    g_model_matrix = glm::mat4(1.0f);
    g_model_matrix_leftwing = glm::mat4(1.0f);
    g_model_matrix_rightwing = glm::mat4(1.0f);
    g_model_matrix_leftglow = glm::mat4(1.0f);
    g_model_matrix_rightglow = glm::mat4(1.0f);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f); 

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // load the textures with the images 
    gabriel_texture_id = load_texture(GABRIEL_SPRITE);
    left_wing_texture_id = load_texture(LEFT_WING_SPRITE);
    right_wing_texture_id = load_texture(RIGHT_WING_SPRITE);
    left_glow_texture_id = load_texture(LEFT_GLOW_SPRITE);
    right_glow_texture_id = load_texture(RIGHT_GLOW_SPRITE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // since initalize gets calls once
    // shifts the objects over BEFORE any movement is done - starting point
    g_model_matrix = glm::translate(g_model_matrix, glm::vec3(-2.5f, -2.0f, 0.0f));
    g_model_matrix_leftwing = glm::translate(g_model_matrix, glm::vec3(-0.1f, -0.3f, 0.0f));
    g_model_matrix_rightwing = glm::translate(g_model_matrix, glm::vec3(0.1f, -0.3f, 0.0f));
    g_model_matrix_leftglow = glm::translate(g_model_matrix, glm::vec3(-0.1f, -0.3f, 0.0f));
    g_model_matrix_rightglow = glm::translate(g_model_matrix, glm::vec3(0.1f, -0.3f, 0.0f));
}

void process_input()
{
    SDL_Event event;
    // check if game is quit
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_game_is_running = false;
        }
    }
}

void move_in_v(glm::mat4& object_model_matrix, float delta_time)
{
    // reset matrix
    object_model_matrix = glm::mat4(1.0f);

    // account for delta time
    float speed = delta_time * MOVEMENT_SPEED;

    // moves object in a upside down V motion
    // breaks the movements into 4 segments
    if (g_frame_counter < (MAX_FRAME / 4))
    {
        x_movement += delta_time * MOVEMENT_SPEED;
        y_movement += delta_time * MOVEMENT_SPEED;
    }
    else if (g_frame_counter < (MAX_FRAME / 2) && g_frame_counter > (MAX_FRAME / 4))
    {
        x_movement += delta_time * MOVEMENT_SPEED;
        y_movement -= delta_time * MOVEMENT_SPEED;
    }
    else if (g_frame_counter < ((MAX_FRAME / 4) * 3) && g_frame_counter > (MAX_FRAME / 2))
    {
        x_movement -= delta_time * MOVEMENT_SPEED;
        y_movement += delta_time * MOVEMENT_SPEED;
    }
    else if (g_frame_counter < MAX_FRAME && g_frame_counter > ((MAX_FRAME / 4) * 3))
    {
        x_movement -= delta_time * MOVEMENT_SPEED;
        y_movement -= delta_time * MOVEMENT_SPEED;
    }
    object_model_matrix = glm::translate(object_model_matrix, glm::vec3(x_movement, y_movement, 0.0f));
}

// pulses the transparent textures
// simulates a glowing effect
void pulse(glm::mat4& object_model_matrix, float delta_time)
{
    if (g_frame_counter < MAX_FRAME / 2)
    {
        growth += GROWTH_FACTOR * delta_time;
    }
    if (g_frame_counter > MAX_FRAME / 2)
    {
        growth -= GROWTH_FACTOR * delta_time;
    }

    glm::vec3 scale_vector = glm::vec3(growth, growth, 1.0f);
    object_model_matrix = glm::scale(g_model_matrix, scale_vector);
}

void update()
{
    float ticks = (float)SDL_GetTicks() / 1000.0f;  // get the current number of ticks
    float delta_time = ticks - g_previous_ticks;     // the delta time is the difference from the last frame
    g_frame_counter += delta_time;
    g_previous_ticks = ticks;

    // translate body and left wing
    move_in_v(g_model_matrix, delta_time);
    move_in_v(g_model_matrix_leftwing, delta_time);
    move_in_v(g_model_matrix_leftglow, delta_time);
    if(g_frame_counter < (MAX_FRAME / 2))
    {
        left_flap += 20.0f * delta_time;
    }
    if (g_frame_counter > (MAX_FRAME / 2))
    {
        left_flap -= 20.0f * delta_time;
    }
    g_model_matrix_leftwing = glm::rotate(g_model_matrix_leftwing, glm::radians(left_flap), glm::vec3(0.0f, 0.0f, 1.0f)); // rotate wing
    pulse(g_model_matrix_leftglow, delta_time);
    g_model_matrix_leftglow = glm::rotate(g_model_matrix_leftglow, glm::radians(left_flap), glm::vec3(0.0f, 0.0f, 1.0f)); 
    // pulse flow and THEN move with wing

    // apply same to right wing
    move_in_v(g_model_matrix_rightwing, delta_time);
    move_in_v(g_model_matrix_rightglow, delta_time);
    if(g_frame_counter < (MAX_FRAME / 2))
    {
        right_flap -= 20.0f * delta_time;
    }
    if (g_frame_counter > (MAX_FRAME / 2))
    {
        right_flap += 20.0f * delta_time;
    }
    g_model_matrix_rightwing = glm::rotate(g_model_matrix_rightwing, glm::radians(right_flap), glm::vec3(0.0f, 0.0f, 1.0f));
    pulse(g_model_matrix_rightglow, delta_time);
    g_model_matrix_rightglow = glm::rotate(g_model_matrix_rightglow, glm::radians(right_flap), glm::vec3(0.0f, 0.0f, 1.0f));

    if (g_frame_counter >= MAX_FRAME) g_frame_counter = 0;
}


void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
        -2.0f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f,  // triangle 1
        -2.0f, -2.0f, 2.0f, 2.0f, -2.0f, 2.0f   // triangle 2
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

    // Bind textures
    draw_object(g_model_matrix, gabriel_texture_id);
    draw_object(g_model_matrix_leftwing, left_wing_texture_id);
    draw_object(g_model_matrix_rightwing, right_wing_texture_id);
    draw_object(g_model_matrix_leftglow, left_glow_texture_id);
    draw_object(g_model_matrix_rightglow, right_glow_texture_id);

    // Disable
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

// shutdown safely
void shutdown() { SDL_Quit(); }

int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}