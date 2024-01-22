#include "WorleyNoiseGL.h"

#include <sstream>

#include "ShaderProgram.h"

WorleyNoiseGL::WorleyNoiseGL( const ivec2& bounds, const uint16_t maxPointsPerCell, uint16_t seed )
    : WorleyNoise( bounds, maxPointsPerCell, seed )
{
    if( glfwGetCurrentContext() == NULL )
    {
        std::cerr << "WorleyNoiseGL: GL Context Required";
        exit( -1 );
    }
    else
    {
        setup();
    }
}

WorleyNoiseGL::~WorleyNoiseGL()
{
    if( m_noiseArray != NULL ) delete m_noiseArray;

    glDeleteTextures( 1, &m_framebufferTexture );
    glDeleteFramebuffers( 1, &m_framebuffer );
}

GLfloat WorleyNoiseGL::Noise2D( const vec2& pt )
{
    return m_noiseArray[ int( ( pt.y * m_bounds.x ) + pt.x ) * 3 ];
}

void WorleyNoiseGL::setup()
{
    setupFramebuffer();
    setupShaders();
    setupDataBuffers();
    Render();
}

void WorleyNoiseGL::setupFramebuffer()
{
    // Color Texture
    glActiveTexture( GL_TEXTURE0 );
    glGenTextures( 1, &m_framebufferTexture );
    glBindTexture( GL_TEXTURE_2D, m_framebufferTexture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_bounds.x, m_bounds.y, 0, GL_RGB, GL_FLOAT, NULL );

    // Generate framebuffer and bind color texture
    glGenFramebuffers( 1, &m_framebuffer );
    glBindFramebuffer( GL_FRAMEBUFFER, m_framebuffer );
    glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_framebufferTexture, 0 );

    if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
        std::cerr << "WorleyNoiseGL: Unable to complete Framebuffer setup";
        exit( -1 );
    }

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void WorleyNoiseGL::setupShaders()
{
    // Setup vertex and fragment shaders
    ShaderProgram basicVert( "shaders/BasicVert.vert", GL_VERTEX_SHADER );
    basicVert.Compile();
    ShaderProgram noiseFrag( "shaders/WorleyNoise.frag", GL_FRAGMENT_SHADER );

    stringstream nb;
    nb << "ivec2( " << m_bounds.x << ", " << m_bounds.y << " )";

    stringstream gd;
    gd << "ivec2( " << m_gridDivisions.x << ", " << m_gridDivisions.y << " )";

    stringstream mp;
    mp << m_maxPointsPerCell;

    stringstream fv;
    fv << m_fValue;

    vector< vector< string > > params = {
        { "ivec2", "RENDER_BOUNDS", nb.str() },
        { "ivec2", "GRID_DIVISIONS", gd.str() },
        { "int", "MAX_POINTS_PER_CELL", mp.str() },
        { "int", "F_VALUE", "1" },
        { "int", "DISTANCE_METRIC", "0" },
        { "float", "MINKOWSKI_NUMBER", "3.0" }
    };
    noiseFrag.Compile( &params );

    // Create shader program, bind vertex/fragment shaders and link
    m_shaderProgram = glCreateProgram();
    glBindAttribLocation( m_shaderProgram, 0, "vertex" );
    glBindFragDataLocation( m_shaderProgram, 0, "color" );
    glAttachShader( m_shaderProgram, basicVert.GetID() );
    glAttachShader( m_shaderProgram, noiseFrag.GetID() );
    glLinkProgram( m_shaderProgram );

    // Check for link errors
    GLint linked = GL_FALSE;
    glGetProgramiv( m_shaderProgram, GL_LINK_STATUS, &linked );
    std::cout << "Program " << m_shaderProgram << " link status: " << ( linked ? "GL_TRUE" : "GL_FALSE" ) << std::endl;

    // Print info log
    GLchar infoLog[ GL_INFO_LOG_LENGTH ] = { 0 };
    glGetProgramInfoLog( m_shaderProgram, GL_INFO_LOG_LENGTH, NULL, infoLog );
    std::cout << std::endl << "Basic Program info log:" << std::endl << infoLog << std::endl;

    // Activate shader
    glUseProgram( m_shaderProgram );
}

void WorleyNoiseGL::setupDataBuffers()
{
    // Setup feature point buffer
    GLuint featPointTBO;
    glGenBuffers( 1, &featPointTBO );
    glBindBuffer( GL_TEXTURE_BUFFER, featPointTBO );

    int pointBufferDim = m_maxPointsPerCell * m_gridDivisions.x * m_gridDivisions.y;
    glBufferData( GL_TEXTURE_BUFFER, pointBufferDim * sizeof( glm::vec4 ), 0, GL_DYNAMIC_DRAW );

    // Create feature point texture & bind it to the buffer
    GLuint featPointTex;
    glGenTextures( 1, &featPointTex );
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_BUFFER, featPointTex );
    glTexBuffer( GL_TEXTURE_BUFFER, GL_RGBA32F, featPointTBO );
    glBindBuffer( GL_TEXTURE_BUFFER, 0 );

    // Buffer feature points
    glBindBuffer( GL_TEXTURE_BUFFER, featPointTBO );
    glm::vec4* featPointBuffer = ( glm::vec4* ) glMapBuffer( GL_TEXTURE_BUFFER, GL_WRITE_ONLY );

    vector< int > cellIndices;
    uint16_t idx = 0;

    for( uint16_t i = 0; i < m_pointGrid.size(); ++i )
    {
        cellIndices.push_back( idx );
        for( uint16_t o = 0; o < m_pointGrid[ i ].size(); ++o )
        {
            featPointBuffer[ idx ] = vec4( m_pointGrid[ i ][ o ], -1, -1 );
            ++idx;
        }
        featPointBuffer[ idx ] = vec4( -1, -1, 0, 0 ); // Terminating value
        ++idx;
    }

    glUnmapBuffer( GL_TEXTURE_BUFFER );
    glBindBuffer( GL_TEXTURE_BUFFER, 0 );

    // Setup cell reference buffer
    GLuint cellRefTBO;
    glGenBuffers( 1, &cellRefTBO );
    glBindBuffer( GL_TEXTURE_BUFFER, cellRefTBO );

    glBufferData( GL_TEXTURE_BUFFER, cellIndices.size() * sizeof( glm::vec4 ), 0, GL_DYNAMIC_DRAW );

    // Create cell reference texture & bind it to the buffer
    GLuint cellRefTex;
    glGenTextures( 1, &cellRefTex );
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_BUFFER, cellRefTex );
    glTexBuffer( GL_TEXTURE_BUFFER, GL_RGBA32F, cellRefTBO );
    glBindBuffer( GL_TEXTURE_BUFFER, 0 );

    // Buffer cell indices
    glBindBuffer( GL_TEXTURE_BUFFER, cellRefTBO );
    glm::vec4* cellRefBuffer = ( glm::vec4* ) glMapBuffer( GL_TEXTURE_BUFFER, GL_WRITE_ONLY );

    for( uint16_t i = 0; i < cellIndices.size(); ++i )
    {
        cellRefBuffer[ i ] = vec4( cellIndices[ i ], -1, -1, -1 );
    }

    glUnmapBuffer( GL_TEXTURE_BUFFER );
    glBindBuffer( GL_TEXTURE_BUFFER, 0 );
}

void WorleyNoiseGL::Render()
{
    glUseProgram( m_shaderProgram );
    glUniform1i( glGetUniformLocation( m_shaderProgram, "FeaturePointSampler" ), 1 );
    glUniform1i( glGetUniformLocation( m_shaderProgram, "CellRefSampler" ), 2 );

    glBindFramebuffer( GL_FRAMEBUFFER, m_framebuffer );

    glClear( GL_COLOR_BUFFER_BIT );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glEnable( GL_TEXTURE_2D );
    glBegin( GL_QUADS );
        glVertex3f(-1.0, 1.0, 0.0 );
        glVertex3f( 1.0, 1.0, 0.0 );
        glVertex3f( 1.0,-1.0, 0.0 );
        glVertex3f(-1.0,-1.0, 0.0 );
    glEnd();
    glDisable( GL_TEXTURE_2D );

    if( m_noiseArray != NULL ) delete m_noiseArray;
    m_noiseArray = new GLfloat[ m_bounds.x * m_bounds.y * 3 ];
    glBindTexture( GL_TEXTURE_2D, m_framebufferTexture );
    glGetTexImage( GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, m_noiseArray );

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}
