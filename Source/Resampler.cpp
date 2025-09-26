/*
  ==============================================================================

    Resampler.cpp
    Created: 26 Sep 2025 10:14:20pm
    Author:  dkuzn

  ==============================================================================
*/

#include "Resampler.h"
#include <cmath>

void Resampler::resample(uint32_t srcSampleRate,
    uint32_t dstSampleRate,
    const float* src,
    uint32_t srcLength,
    float*& dst,
    uint32_t& dstLength)
{
    dst = nullptr;
    dstLength = 0;

    if (srcSampleRate == 0 || dstSampleRate == 0 || src == nullptr || srcLength == 0)
        return;

    // wsp�czynnik zmiany d�ugo�ci
    double ratio = static_cast<double>(dstSampleRate) / static_cast<double>(srcSampleRate);

    // nowa d�ugo��
    dstLength = static_cast<uint32_t>(std::ceil(srcLength * ratio));

    // alokacja pami�ci na wynik
    dst = new float[dstLength];

    for (uint32_t i = 0; i < dstLength; ++i)
    {
        // pozycja w sygnale �r�d�owym
        double srcIndex = static_cast<double>(i) / ratio;

        // indeks ca�kowity
        uint32_t idx = static_cast<uint32_t>(srcIndex);

        if (idx + 1 < srcLength)
        {
            // interpolacja liniowa
            double frac = srcIndex - idx;
            dst[i] = static_cast<float>((1.0 - frac) * src[idx] + frac * src[idx + 1]);
        }
        else
        {
            // ostatnia pr�bka
            dst[i] = src[srcLength - 1];
        }
    }
}
