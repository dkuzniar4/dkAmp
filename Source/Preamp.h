/*
  ==============================================================================

    Preamp.h
    Created: 30 Aug 2025 7:39:48pm
    Author:  dkuzn

  ==============================================================================
*/

#pragma once
#include <cmath>

/**
@sgn
\ingroup FX-Functions

@brief calculates sgn( ) of input
\param xn - the input value
\return -1 if xn is negative or +1 if xn is 0 or greater
*/
inline double sgn(double xn)
{
    return (xn > 0) - (xn < 0);
}

/**
@softClipWaveShaper
\ingroup FX-Functions

@brief calculates hyptan waveshaper
\param xn - the input value
\param saturation  - the saturation control
\return the waveshaped output value
*/
inline double softClipWaveShaper(double xn, double saturation)
{
    // --- un-normalized soft clipper from Reiss book
    return sgn(xn) * (1.0 - exp(-fabs(saturation * xn)));
}

inline float waveshaper(float x, float a1, float a2, float a3, float a4)
{
    float x2 = x * x;
    return (a1 * x) + (a2 * x2) + (a3 * x * x2) + (a4 * x2 * x2);
}
