// #version def and CPU-exposed constants are appended in ShaderProgram.cpp

// const vec2 RENDER_BOUNDS
// const vec2 GRID_DIVISIONS
// const int MAX_POINTS_PER_CELL
// const int F_VALUE
// const int DISTANCE_METRIC
// const int MINKOWSKI_NUMBER

out vec4 color;

uniform samplerBuffer FeaturePointSampler;
uniform samplerBuffer CellRefSampler;

float dist( in vec2 a, in vec2 b )
{
    float dist;

    vec2 d = a - b;

    switch( DISTANCE_METRIC )
    {
    case 0: // Linear
    {
        dist = length( d );
        break;
    }
    case 1: // Linear Squared
    {
        dist = dot( d, d );
        break;
    }
    case 2: // Manhattan
    {
        dist = abs( d.x ) + abs( d.y );
        break;
    }
    case 3: // Chebyshev
    {
        float x = abs( d.x );
        float y = abs( d.y );
        if( x == y || x < y )
        {
            dist = y;
        }
        else
        {
            dist = x;
        }
        break;
    }
    case 4: // Quadratic
    {
        dist = ( d.x * d.x + d.x * d.y + d.y * d.y );
        break;
    }
    case 5: // Minkowski
    {
        dist = pow( pow( abs( d.x ), MINKOWSKI_NUMBER ) + pow( abs( d.y ), MINKOWSKI_NUMBER ), ( 1.0f / MINKOWSKI_NUMBER ) );
        break;
    }
    default:
    {
        break;
    }
    }

    return dist;
}

void main()
{
    vec2 screenCoord = gl_FragCoord.xy;
    int cellX = int( floor( ( screenCoord.x / RENDER_BOUNDS.x ) * GRID_DIVISIONS.x ) );
    int cellY = int( floor( ( screenCoord.y / RENDER_BOUNDS.y ) * GRID_DIVISIONS.y ) );

    // Add 3x3 block of cells surrounding point to search candidates
    vec2[ MAX_POINTS_PER_CELL * 9 ] searchPoints;
    int searchPointsIdx = 0;
    for( int i = 0; i < 9; ++i )
    {
        int xOff = ( i % 3 ) - 1;
        int yOff = ( i / 3 ) - 1;
        if( cellX + xOff < 0 ) continue;
        if( cellX + xOff > GRID_DIVISIONS.x - 1 ) continue;
        if( cellY + yOff < 0 ) continue;
        if( cellY + yOff > GRID_DIVISIONS.y - 1 ) continue;
        int cellIdx = ( cellY + yOff ) * GRID_DIVISIONS.x + cellX + xOff;

        // Fetch cell reference ( starting idx in feature point array )
        int cellRef = int( texelFetch( CellRefSampler, cellIdx ).x );

        // Iterate through cell until terminating value
        for( int o = 0; o < MAX_POINTS_PER_CELL; ++o )
        {
            vec4 pointVal = texelFetch( FeaturePointSampler, cellRef + o );
            if( pointVal == vec4( -1, -1, 0, 0 ) ) break; // Terminating value
            searchPoints[ searchPointsIdx ] = pointVal.xy;
            ++searchPointsIdx;
        }
    }

    // Return error if the fValue is greater than the number of potential search points
    if( F_VALUE == 0 || F_VALUE > MAX_POINTS_PER_CELL * 9 ) discard;

    float maxLength = dist( vec2( 0, 0 ), vec2( RENDER_BOUNDS / GRID_DIVISIONS ) );
    float f1 = maxLength;
    float f2 = maxLength;
    float f3 = maxLength;

    // Calculate f values
    for( int i = 0; i < searchPointsIdx; ++i )
    {
        float curDist = dist( screenCoord, searchPoints[ i ] );
        if( curDist < f1 )
        {
            f1 = curDist;
        }
    }

    for( int i = 0; i < searchPointsIdx; ++i )
    {
        float curDist = dist( screenCoord, searchPoints[ i ] );
        if( curDist < f2 && curDist > f1 )
        {
            f2 = curDist;
        }
    }

    for( int i = 0; i < searchPointsIdx; ++i )
    {
        float curDist = dist( screenCoord, searchPoints[ i ] );
        if( curDist < f3 && curDist > f2 )
        {
            f3 = curDist;
        }
    }

    // Adjust maxLength to account for squared outputs
    if( DISTANCE_METRIC == 1 || DISTANCE_METRIC == 4 )
    {
        maxLength *= 0.1f;
    }
    else
    {
        maxLength *= 0.5f;
    }

    float normDist = 0;
    switch( F_VALUE )
    {
        case 1:
            normDist = f1 / maxLength;
            break;
        case 2:
            normDist = f2 / maxLength;
            break;
        case 3:
            normDist = f3 / maxLength;
            break;
        case -1:
            normDist = ( f2 - f1 ) / maxLength;
            break;
        case -2:
            normDist = ( f3 - f2 ) / maxLength;
            break;
        default:
            discard;
    }
    color = vec4( normDist, normDist, normDist, 1.0 );
}
