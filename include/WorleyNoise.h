#ifndef NOISEGEN_H
#define NOISEGEN_H

#include <vector>
#include <glm/glm.hpp>

#include "PseudoRandom.h"

using namespace std;
using namespace glm;

class WorleyNoise
{
public:
    WorleyNoise( const ivec2& bounds, const uint16_t maxPointsPerCell, uint16_t seed );

    float GetMinkowskiNumber() const;
    void SetMinkowskiNumber( const float n );

    uint16_t GetMaxPointsPerCell() const { return m_maxPointsPerCell; }
    void SetMaxPointsPerCell( const uint16_t maxPointsPerCell ) { m_maxPointsPerCell = maxPointsPerCell; }

    ivec2 GetGridDivisions() const { return m_gridDivisions; }
    void SetGridDivisions( const ivec2 d ) { m_gridDivisions = d; }

    uint16_t GetFValue() const { return m_fValue; }
    void SetFValue( const uint16_t v ) { m_fValue = v; }

    uint16_t GetDistanceMetric() const { return m_distanceMetric; }
    void SetDistanceMetric( const uint16_t m ) { m_distanceMetric = m; }

    const vector< vector< vec2 > > GetPointGrid() const { return m_pointGrid; }

    float Noise2D( const vec2& pt );

protected:
    void setupCells( uint16_t seed );

    ivec2 m_bounds = ivec2( 320, 320 );
    uint16_t m_maxPointsPerCell = 10;
    ivec2 m_gridDivisions = ivec2( 4, 4 );
    uint16_t m_fValue = 1;
    uint16_t m_distanceMetric = 0;
    ivec2 m_cellSize;

    vector< vector< vec2 > > m_pointGrid;
};

#endif // NOISEGEN_H
