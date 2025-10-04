/*
  ==============================================================================

    FFT.cpp
    Created: 12 Sep 2025 11:13:52pm
    Author:  dkuzn

  ==============================================================================
*/

#include <cmath>
#include "FFT.h"


#ifndef M_PI
namespace
{
    const double M_PI = std::acos(-1.0);
}
#endif


FFT::FFT()
{

}

void FFT::FFT_process(float* Re, float* Im, uint32_t size)
{
    uint32_t nm1 = size - 1;
    uint32_t nd2 = size / 2;
    uint32_t m = (uint32_t)(log(size) / log(2));
    uint32_t j = nd2;
    uint32_t k, le, le2, ip;

    float tr, ti, ur, ui, sr, si;

    // bit reversal
    for (uint32_t i = 1; i <= (size - 2); i++)
    {
        if (i < j)
        {
            tr = Re[j];
            ti = Im[j];
            Re[j] = Re[i];
            Im[j] = Im[i];
            Re[i] = tr;
            Im[i] = ti;
        }

        k = nd2;

        while (k <= j)
        {
            j -= k;
            k /= 2;
        }

        j += k;
    }

    for (uint32_t i0 = 1; i0 <= m; i0++)
    {
        le = pow(2, i0);
        le2 = le / 2;
        ur = 1.0f;
        ui = 0.0f;
        sr = cos(M_PI / (float)le2);
        si = -sin(M_PI / (float)le2);

        for (uint32_t i1 = 1; i1 <= le2; i1++)
        {
            for (uint32_t i2 = i1 - 1; i2 <= nm1; i2 += le)
            {
                ip = i2 + le2;
                tr = (Re[ip] * ur) - (Im[ip] * ui);
                ti = (Re[ip] * ui) + (Im[ip] * ur);
                Re[ip] = Re[i2] - tr;
                Im[ip] = Im[i2] - ti;
                Re[i2] = Re[i2] + tr;
                Im[i2] = Im[i2] + ti;
            }
            tr = ur;
            ur = (tr * sr) - (ui * si);
            ui = (tr * si) + (ui * sr);
        }
    }
}

void FFT::IFFT_process(float* Re, float* Im, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
    {
        Im[i] = -Im[i];
    }

    FFT_process(Re, Im, size);

    for (uint32_t i = 0; i < size; i++)
    {
        Re[i] /= size;
        Im[i] = -Im[i] / size;
    }
}

void FFT::rectangularToPolar(float* Re, float* Im, float* Mag, float* Phase, uint32_t size)
{
    if (!Re || !Im || !Mag || !Phase) {
        return; // Check if nullptr
    }

    for (uint32_t i = 0u; i < size; i++)
    {
        Mag[i] = sqrt((Re[i] * Re[i]) + (Im[i] * Im[i]));
        if (Re[i] == 0)
        {
            Re[i] = 0.00000000000001f;
        }

        Phase[i] = atan(Im[i] * (1.0f / Re[i]));

        if ((Re[i] < 0.0f) && (Im[i] < 0.0f))
        {
            Phase[i] = Phase[i] - M_PI;
        }

        if ((Re[i] < 0.0f) && (Im[i] >= 0.0f))
        {
            Phase[i] = Phase[i] + M_PI;
        }
    }
}

void FFT::polarToRectangular(float* Mag, float* Phase, float* Re, float* Im, uint32_t size)
{
    if (!Mag || !Phase || !Re || !Im) {
        return; // Check if nullptr
    }

    for (uint32_t i = 0; i < size; i++)
    {
        Re[i] = Mag[i] * cos(Phase[i]);
        Im[i] = Mag[i] * sin(Phase[i]);
    }
}

uint32_t FFT::calculateFFTWindow(uint32_t length)
{
    uint32_t fftSize = 1;
    while (fftSize < length)
    {
        fftSize *= 2; // Find the next power of 2 larger than 2 * IR_len
    }
    return fftSize;
}
