/*
  ==============================================================================

    Transient.h
    Created: 13 Sep 2025 7:40:12pm
    Author:  dkuzn

  ==============================================================================
*/

#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <vector>

// Simple biquad
struct SimpleBiquad
{
    float b0 = 0, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float z1 = 0, z2 = 0;

    float process(float x)
    {
        float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }

    void reset() { z1 = z2 = 0; }
};

class NonlinearProcessor
{
public:
    NonlinearProcessor() = default;

    // Load table [3][ampSteps]
    void load(float* tab);

    // Init processor
    void init(uint32_t ampSteps,
        float freq_1, float freq_2, float freq_3,
        float sampleRate);

    // Process data
    float process(float input);

    void reset();

private:
    void designLowpass(float fc, float fs, SimpleBiquad& bq);
    void designHighpass(float fc, float fs, SimpleBiquad& bq);

    float* tran = nullptr;
    uint32_t ampSteps = 0;
    float sampleRate = 44100.0f;

    // Crossovers filters
    SimpleBiquad low_lp1, low_lp2;
    SimpleBiquad mid_hp1, mid_hp2;
    SimpleBiquad mid_lp1, mid_lp2;
    SimpleBiquad high_hp1, high_hp2;
};
