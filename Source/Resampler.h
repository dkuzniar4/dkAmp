/*
  ==============================================================================

    Resampler.h
    Created: 26 Sep 2025 10:14:20pm
    Author:  dkuzn

  ==============================================================================
*/

#pragma once
#include <cstdint>

class Resampler
{
public:
    // Funkcja zwraca nowy zresamplowany bufor w `dst` (zaalokowany przez new[])
    // U¿ytkownik musi potem zwolniæ pamiêæ: delete[] dst;
    static void resample(uint32_t srcSampleRate,
        uint32_t dstSampleRate,
        const float* src,
        uint32_t srcLength,
        float*& dst,
        uint32_t& dstLength);
};

