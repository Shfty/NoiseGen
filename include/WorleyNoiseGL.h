#ifndef WORLEY_NOISE_GL_H
#define WORLEY_NOISE_GL_H

#include "WorleyNoise.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class WorleyNoiseGL : WorleyNoise
{
public:
    WorleyNoiseGL( const ivec2& bounds, const uint16_t maxPointsPerCell, uint16_t seed );
    ~WorleyNoiseGL();

    // TODO: Inherit these properly
    void SetFValue( const uint16_t v ) { m_fValue = v; }
    void SetDistanceMetric( const uint16_t m ) { m_distanceMetric = m; }

    void Render();
    GLfloat Noise2D( const vec2& point );

private:
    void setup();
    bool initGL();
    void setupFramebuffer();
    void setupShaders();
    void setupDataBuffers();

    GLuint m_framebufferTexture = 0;
    GLuint m_framebuffer = 0;
    GLuint m_shaderProgram = 0;
    GLfloat* m_noiseArray = 0;
};

#endif // WORLEY_NOISE_GL
