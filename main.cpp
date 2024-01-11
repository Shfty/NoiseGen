#include <iostream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <WorleyNoise.h>
#include <WorleyNoiseGL.h>
#include <ShaderProgram.h>

#define USE_GPU

using namespace std;

const uint16_t RANDOM_SEED = 65534;
const int WINDOW_WIDTH = 320;
const int WINDOW_HEIGHT = 320;
const int NOISE_WIDTH = 160;
const int NOISE_HEIGHT = 160;
const int MAX_POINTS_PER_CELL = 10;
const int DISTANCE_METRIC = 5;
const char* WINDOW_TITLE = "NoiseGen";

GLFWwindow* g_window;
GLuint g_screenTexture;

bool initGL()
{
    // Get a window + context
    if( !glfwInit() ) return false;

    g_window = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL );

    if( !g_window )
    {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent( g_window );

    // Init GLEW
    GLenum err = glewInit();
    if ( GLEW_OK != err )
    {
        std::cerr << "GLEW Init Error: " << glewGetErrorString( err ) << std::endl;
        glfwTerminate();
        return false;
    }
    std::cout << "GLEW Init Success, Using Version " << glewGetString( GLEW_VERSION ) << std::endl << std::endl;


    // Swap buffers so the window starts out black
    glfwSwapBuffers( g_window );

    return true;
}

int main()
{
    // Try to initialize OpenGL, quit on error
    if( !initGL() ) return -1;

    // Setup noise
    ivec2 noiseSize( NOISE_WIDTH, NOISE_HEIGHT );
#ifdef USE_GPU
    WorleyNoiseGL noise( noiseSize, MAX_POINTS_PER_CELL, RANDOM_SEED );
#else
    WorleyNoise noise( noiseSize, MAX_POINTS_PER_CELL, RANDOM_SEED );
#endif
    //noise.SetDistanceMetric( DISTANCE_METRIC );
    //noise.SetFValue( 1 );
    //noise.Render();

    // Create noise buffer
    GLfloat* pixels = new GLfloat[ noiseSize.x * noiseSize.y * 3 ];
    for( int x = 0; x < noiseSize.x * 3; x += 3 )
    {
        for( int y = 0; y < noiseSize.y * 3; y += 3 )
        {
            GLfloat val = noise.Noise2D( vec2( x / 3, y / 3 ) );
            pixels[ ( noiseSize.x * y ) + x + 0 ] = val;
            pixels[ ( noiseSize.x * y ) + x + 1 ] = val;
            pixels[ ( noiseSize.x * y ) + x + 2 ] = val;
        }
    }

    // Generate and bind screen texture, setup params
    glActiveTexture( GL_TEXTURE0 );
    glGenTextures( 1, &g_screenTexture );
    glBindTexture( GL_TEXTURE_2D, g_screenTexture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    // Populate texture with noise buffer
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        noiseSize.x,
        noiseSize.y,
        0,
        GL_RGB,
        GL_FLOAT,
        pixels
    );

    glUseProgram( 0 );

    // Draw to screen
    glClear( GL_COLOR_BUFFER_BIT );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glEnable( GL_TEXTURE_2D );
    glBegin( GL_QUADS );
        glTexCoord2f( 0.0, 0.0 ); glVertex3f(-1.0, 1.0, 0.0 );
        glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 1.0, 0.0 );
        glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0,-1.0, 0.0 );
        glTexCoord2f( 0.0, 1.0 ); glVertex3f(-1.0,-1.0, 0.0 );
    glEnd();
    glDisable( GL_TEXTURE_2D );

    /* Swap front and back buffers */
    glfwSwapBuffers( g_window );

    while ( !glfwWindowShouldClose( g_window ) )
    {
        /* Poll for and process events */
        if( glfwGetKey( g_window, GLFW_KEY_ESCAPE ) )
        {
            glfwSetWindowShouldClose( g_window, true );
        }

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
