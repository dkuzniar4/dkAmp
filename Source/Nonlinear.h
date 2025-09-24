/*
  ==============================================================================

    Nonlinear.h
    Created: 13 Sep 2025 7:40:12pm
    Author:  dkuzn

  ==============================================================================
*/

#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <vector>

// first band : 0 - FREQ_1
// second band: FREQ_1 - FREQ_2
// third band:  FREQ_2 - NYQUIST
#define FREQ_1 300.0f
#define FREQ_2 3000.0f

// test tones was: 200 Hz, 1kHz, 8kHz


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
    void init(uint32_t ampSteps, float freq_1, float freq_2, float sampleRate);

    // Process data
    float process(float input);

    void enableLow(bool state);
    void enableMid(bool state);
    void enableHigh(bool state);

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

    bool enLow = true;
    bool enMid = true;
    bool enHigh = true;
};
