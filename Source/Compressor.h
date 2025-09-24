/*
  ==============================================================================

    Compressor.h
    Created: 23 Sep 2025 11:02:16pm
    Author:  dkuzn

  ==============================================================================
*/

#pragma once
#include <cmath>

class Compressor
{
public:
    Compressor(float ratio = 4.0f);

    void setRatio(float ratio);
    float getRatio() const;

    // G��wna funkcja przetwarzaj�ca
    float process(float input, float threshold, bool limit);

private:
    float m_ratio; // wsp�czynnik kompresji (np. 4:1)
};

