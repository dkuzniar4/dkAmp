/*
  ==============================================================================

    ShapeProcessor.cpp
    Created: 28 Sep 2025 4:55:00pm
    Author:  dkuzn

  ==============================================================================
*/
#include "ShapeProcessor.h"
#include <iostream>

void ShapeProcessor::load(float* tab)
{
    this->tab = tab;
}

void ShapeProcessor::init(uint32_t ampSteps)
{
    this->ampSteps = ampSteps;
}

float ShapeProcessor::process(float input)
{
    float idx_f;
    float frac;
    float g1;
    float g2;
    float gain;
    int idx;
    int idxNext;


    float amp = std::fabs(input);

    idx_f = amp * (ampSteps - 1);
    idx = std::clamp(static_cast<int>(std::floor(idx_f)), 0, static_cast<int>(ampSteps - 1));
    idxNext = std::min(idx + 1, static_cast<int>(ampSteps - 1));
    frac = idx_f - idx;

    g1 = tab[idx];
    g2 = tab[idxNext];
    gain = g1 + frac * (g2 - g1);

    float out = input * gain;

    return out;
}
