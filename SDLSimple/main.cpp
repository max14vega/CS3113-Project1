/**
* Author: Maximiliano Vega
* Assignment: Simple 2D Scene
* Date due: 2024-02-17, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"


const int WINDOW_WIDTH = 640,
          WINDOW_HEIGHT = 480;

float BG_RED = 0.132f,
      BG_BLUE = 0.349f,
      BG_GREEN = 1.3059f,
      BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const char PIPLUP_SPRITE[] = "assets/piplup_sprite.png";
const char MANA_SPRITE[] = "assets/manaphy_sprite.png";
const char WHIRLPOOL[] = "assets/whirlpool.png";

GLuint piplup_id;
GLuint mana_id;
GLuint whirl_id;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 view_matrix, g_model_matrix_1, g_model_matrix_2, g_model_matrix_bg, g_projection_matrix, g_trans_matrix;

//globals for rotating + circular motion
float g_previous_ticks = 0.0f;
const float RADIUS = 2.0f;
const int SPEED = 2.3;
float rotation_angle_delta = 0.0f;

//globals for bg color change
const int MAX_FRAMES = 40;
int frame_counter = 1;


GLuint load_texture(const char* filepath){
    //loading the image file
    int width, height, num_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &num_components, STBI_rgb_alpha);
    
    if (image == NULL){
        LOG("Unable to load image. Make sure the path is correct.");
        LOG(filepath);
        assert(false);
    }
    
    //generating and binding a texture ID to our image
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

void initialise()
{
    // Initialise video and joystick subsystems
    SDL_Init(SDL_INIT_VIDEO);
    
    g_display_window = SDL_CreateWindow("Piplup's Whirlpool Adventure!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,                                         SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(view_matrix);

    
    glUseProgram(g_shader_program.get_program_id());
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    whirl_id = load_texture(WHIRLPOOL);
    piplup_id = load_texture(PIPLUP_SPRITE);
    mana_id = load_texture(MANA_SPRITE);
    
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input(){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE: g_game_is_running = false;
            default: break;
        }
    }
}

void update(){
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;
    
    glm::vec3 scale_vector;
    
    float scale_factor = 1.01f;
    
    scale_vector = glm::vec3(scale_factor, scale_factor, 1.0f);

    //handles the rotation of piplup about the origin
    rotation_angle_delta += SPEED * delta_time;
    float rotation_angle = SPEED * ticks;
    float tri_x = RADIUS * cos(rotation_angle_delta);
    float tri_y = RADIUS * sin(rotation_angle_delta);
    
    
    g_model_matrix_1 = glm::mat4(1.0f); //piplup
    g_model_matrix_2 = glm::mat4(1.0f); //manaphy
    g_model_matrix_bg = glm::mat4(1.0f); //whirlpool bg
    
    
    //operations on piplup
    g_model_matrix_1 = glm::translate(g_model_matrix_2, glm::vec3(tri_x, tri_y, 0.0f));
    g_model_matrix_1 = glm::rotate(g_model_matrix_1, -rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));
    
    //operations on manaphy
    g_model_matrix_2 = glm::scale(g_model_matrix_1, scale_vector);
    g_model_matrix_2 = glm::translate(g_model_matrix_2, glm::vec3(tri_y*0.4, tri_x*0.4, 0.0f));
    g_model_matrix_2 = glm::rotate(g_model_matrix_2, rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));
    
    //operations on bg
    g_model_matrix_bg = glm::scale(g_model_matrix_bg, glm::vec3(10.0f, 10.0f, 1.0f));
    g_model_matrix_bg = glm::rotate(g_model_matrix_bg, rotation_angle*(0.4f), glm::vec3(0.0f, 0.0f, 1.0f));

}

void render(){
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };
    
    //model bg (i learned they overlap in order of rendering! unless thats common knowledge...)
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    g_shader_program.set_model_matrix(g_model_matrix_bg);
    glBindTexture(GL_TEXTURE_2D, whirl_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    //Model 1
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    g_shader_program.set_model_matrix(g_model_matrix_1);
    glBindTexture(GL_TEXTURE_2D, piplup_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    //Model 2
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    g_shader_program.set_model_matrix(g_model_matrix_2);
    glBindTexture(GL_TEXTURE_2D, mana_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown(){
    SDL_Quit();
}


int main(int argc, char* argv[]){
    initialise();
    
    while (g_game_is_running){
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
