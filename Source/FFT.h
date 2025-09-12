/*
  ==============================================================================

    FFT.h
    Created: 12 Sep 2025 11:13:52pm
    Author:  dkuzn

  ==============================================================================
*/

#pragma once

#include "stdint.h"

class FFT
{
public:
    FFT();
    void FFT_process(float* Re, float* Im, uint32_t size);
    void IFFT_process(float* Re, float* Im, uint32_t size);
    void rectangularToPolar(float* Re, float* Im, float* Mag, float* Phase, uint32_t size);
    void polarToRectangular(float* Mag, float* Phase, float* Re, float* Im, uint32_t size);
    uint32_t calculateFFTWindow(uint32_t length);

private:

};
