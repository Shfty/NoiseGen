#include "WorleyNoise.h"

#include <iostream>
#include <algorithm>

#include "PseudoRandom.h"

float g_minkowskiNumber = 3.0f;
float WorleyNoise::GetMinkowskiNumber() const { return g_minkowskiNumber; }
void WorleyNoise::SetMinkowskiNumber( const float n ) { g_minkowskiNumber = n; }

WorleyNoise::WorleyNoise( const ivec2& bounds, const uint16_t maxPointsPerCell, uint16_t seed )
{
    m_bounds = bounds;
    m_maxPointsPerCell = maxPointsPerCell;
    setupCells( seed );
}

void WorleyNoise::setupCells( uint16_t seed )
{
    m_cellSize.x = m_bounds.x / m_gridDivisions.x;
    m_cellSize.y = m_bounds.y / m_gridDivisions.y;

    m_pointGrid = vector< vector< vec2 > >( m_gridDivisions.x * m_gridDivisions.y, vector< vec2 >() );

    for( uint16_t i = 0; i < m_pointGrid.size(); ++i )
    {
        uint16_t cellX = i % m_gridDivisions.x;
        uint16_t cellY = i / m_gridDivisions.x;
        uint16_t pointCount = PseudoRandom::PRScalarAbs( seed ) * m_maxPointsPerCell;
        for( uint16_t j = 0; j < pointCount; ++j )
        {
            float offsetX = cellX * m_cellSize.x;
            float offsetY = cellY * m_cellSize.y;
            float randX = PseudoRandom::PRScalarAbs( seed ) * m_cellSize.x;
            float randY = PseudoRandom::PRScalarAbs( seed ) * m_cellSize.y;
            m_pointGrid[ i ].push_back( vec2( offsetX + randX, offsetY + randY ) );
        }
    }
}

float dist( const vec2& a, const vec2& b, const int metric )
{
    float distance = 0;

    vec2 d = a - b;

    switch( metric )
    {
    case 0: // Linear
    {
        distance = glm::length( d );
        break;
    }
    case 1: // Linear Squared
    {
        distance = glm::dot( d, d );
        break;
    }
    case 2: // Manhattan
    {
        distance = std::abs( d.x ) + std::abs( d.y );
        break;
    }
    case 3: // Chebyshev
    {
        float x = std::abs( d.x );
        float y = std::abs( d.y );
        if( x == y || x < y )
        {
            distance = y;
        }
        else
        {
            distance = x;
        }
        break;
    }
    case 4: // Quadratic
    {
        distance = ( d.x * d.x + d.x * d.y + d.y * d.y );
        break;
    }
    case 5: // Minkowski
    {
        distance = pow( pow( abs( d.x ), g_minkowskiNumber ) + pow( abs( d.y ), g_minkowskiNumber ), ( 1.0f / g_minkowskiNumber ) );
        break;
    }
    default:
    {
        break;
    }
    }

    return distance;
}

vec2 g_sortDistPt;
int g_sortDistMetric;
bool distSort( const vec2& a, const vec2& b )
{
    float distA = dist( g_sortDistPt, a, g_sortDistMetric );
    float distB = dist( g_sortDistPt, b, g_sortDistMetric );
    return ( distA < distB );
}

float WorleyNoise::Noise2D( const vec2& pt )
{
    // Return error if the point is out of bounds
    if( pt.x < 0 || pt.x > m_bounds.x || pt.y < 0 || pt.y > m_bounds.y ) return -1;

    // Calculate grid coordinates
    int m_cellX = std::floor( pt.x / m_bounds.x * m_gridDivisions.x );
    int m_cellY = std::floor( pt.y / m_bounds.y * m_gridDivisions.y );

    // Add 3x3 block of cells surrounding point to search candidates
    vector< vec2 > searchPoints;
    for( int i = 0; i < 9; ++i )
    {
        int xOff = ( i % 3 ) - 1;
        int yOff = ( i / 3 ) - 1;
        if( m_cellX + xOff < 0 ) continue;
        if( m_cellX + xOff > m_gridDivisions.x - 1 ) continue;
        if( m_cellY + yOff < 0 ) continue;
        if( m_cellY + yOff > m_gridDivisions.y - 1 ) continue;
        int cellIdx = ( m_cellY + yOff ) * m_gridDivisions.x + m_cellX + xOff;
        vector< vec2 > cell = m_pointGrid[ cellIdx ];
        searchPoints.insert( searchPoints.end(), cell.begin(), cell.end() );
    }

    // Return error if the fValue is greater than the number of potential search points
    if( m_fValue == 0 || m_fValue > searchPoints.size() ) return -1;

    // Sort the points from near-far
    g_sortDistPt = pt;
    g_sortDistMetric = m_distanceMetric;
    std::sort( searchPoints.begin(), searchPoints.end(), distSort );

    // Calculate the distance and maximum length using our predefined metric
    float distance = dist( pt, searchPoints[ m_fValue - 1 ], m_distanceMetric );
    float maxLength = dist( vec2( 0, 0 ), vec2( m_cellSize ), m_distanceMetric );

    // Adjust maxLength to account for squared outputs
    if( m_distanceMetric == 1 || m_distanceMetric == 4 )
    {
        maxLength *= 0.1f;
    }
    else
    {
        maxLength *= 0.5f;
    }

    float normDist = distance / maxLength;

    return normDist;
}
