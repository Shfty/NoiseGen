#ifndef PSEUDORANDOM_H
#define PSEUDORANDOM_H

#include <iostream>

using namespace std;

inline void advanceSeed( uint16_t& seed )
{
    seed = ( seed * std::log( seed ) ) / atan2( seed, std::cos( seed ) );
}

namespace PseudoRandom {
    inline uint16_t PRInteger( uint16_t& seed )
    {
        advanceSeed( seed );
        return seed;
    }

    inline float PRScalar( uint16_t& seed )
    {
        static int interval = 0;

        float out;
        switch( interval % 4 )
        {
            case 0:
                out = sin( seed );
                break;
            case 1:
                out = cos( seed * seed );
                break;
            case 2:
                out = sin( seed * seed * seed );
                break;
            case 3:
                out = cos( seed * seed * seed * seed );
                break;
            default:
                break;
        }

        advanceSeed( seed );
        ++interval;

        return out;
    }

    inline float PRScalarAbs( uint16_t& seed )
    {
        return abs( PRScalar( seed ) );
    }
}

#endif // PSEUDORANDOM_H
