/*
  ==============================================================================

    Compressor.cpp
    Created: 23 Sep 2025 11:02:16pm
    Author:  dkuzn

  ==============================================================================
*/

#include "Compressor.h"

Compressor::Compressor(float ratio)
    : m_ratio(ratio)
{
}

void Compressor::setRatio(float ratio)
{
    m_ratio = ratio;
}

float Compressor::getRatio() const
{
    return m_ratio;
}

float Compressor::process(float input, float threshold, bool limit)
{
    if (limit)
    {
        // Limiter z miêkkim ograniczeniem (soft clipping przez tanh)
        if (std::fabs(input) > threshold)
        {
            float sign = (input >= 0.0f) ? 1.0f : -1.0f;
            float over = std::fabs(input) - threshold;
            input = sign * (threshold + over / m_ratio);
        }

        // Soft clipping -> naturalne ograniczenie sygna³u
        input = std::tanh(input);
        return input;
    }
    else
    {
        // Kompresor standardowy
        float absIn = std::fabs(input);
        if (absIn > threshold)
        {
            float sign = (input >= 0.0f) ? 1.0f : -1.0f;
            float over = absIn - threshold;
            input = sign * (threshold + over / m_ratio);
        }
        return input;
    }
}

