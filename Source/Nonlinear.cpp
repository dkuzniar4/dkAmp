/*
  ==============================================================================

    Nonlinear.cpp
    Created: 13 Sep 2025 7:40:12pm
    Author:  dkuzn

  ==============================================================================
*/

#include "Nonlinear.h"


#ifndef M_PI
namespace
{
    const double M_PI = std::acos(-1.0);
}
#endif


void NonlinearProcessor::load(float* tab)
{
    tran = tab;
}

void NonlinearProcessor::init(uint32_t ampSteps, float freq_1, float freq_2, float sampleRate)
{
    this->ampSteps = ampSteps;
    this->sampleRate = sampleRate;

    // Filters for crossovers
    designLowpass(freq_1, sampleRate, low_lp1);
    designLowpass(freq_1, sampleRate, low_lp2);

    designHighpass(freq_1, sampleRate, mid_hp1);
    designHighpass(freq_1, sampleRate, mid_hp2);
    designLowpass(freq_2, sampleRate, mid_lp1);
    designLowpass(freq_2, sampleRate, mid_lp2);

    designHighpass(freq_2, sampleRate, high_hp1);
    designHighpass(freq_2, sampleRate, high_hp2);
}

float NonlinearProcessor::process(float input)
{
    // Crossover
    float low = low_lp2.process(low_lp1.process(input));

    float midBand = mid_hp2.process(mid_hp1.process(input));
    midBand = mid_lp2.process(mid_lp1.process(midBand));

    float high = high_hp2.process(high_hp1.process(input));

    float bands[3] = { low, midBand, high };
    float out = 0.0f;

    for (int b = 0; b < 3; b++)
    {
        float amp = std::fabs(bands[b]);

        float idx_f = amp * (ampSteps - 1);
        int idx = std::clamp(static_cast<int>(std::floor(idx_f)), 0, static_cast<int>(ampSteps - 1));
        int idxNext = std::min(idx + 1, static_cast<int>(ampSteps - 1));
        float frac = idx_f - idx;

        float g1 = tran[b * ampSteps + idx];
        float g2 = tran[b * ampSteps + idxNext];
        float gain = g1 + frac * (g2 - g1);

        out += bands[b] * gain;
    }

    return out;
}

void NonlinearProcessor::reset()
{
    low_lp1.reset(); low_lp2.reset();
    mid_hp1.reset(); mid_hp2.reset();
    mid_lp1.reset(); mid_lp2.reset();
    high_hp1.reset(); high_hp2.reset();
}

// ------------------- Filters ---------------------

void NonlinearProcessor::designLowpass(float fc, float fs, SimpleBiquad& bq)
{
    float w0 = 2.0f * M_PI * fc / fs;
    float cosw0 = std::cos(w0);
    float sinw0 = std::sin(w0);
    float Q = std::sqrt(0.5f); // Butterworth
    float alpha = sinw0 / (2.0f * Q);

    float b0 = (1 - cosw0) / 2.0f;
    float b1 = 1 - cosw0;
    float b2 = (1 - cosw0) / 2.0f;
    float a0 = 1 + alpha;
    float a1 = -2 * cosw0;
    float a2 = 1 - alpha;

    bq.b0 = b0 / a0;
    bq.b1 = b1 / a0;
    bq.b2 = b2 / a0;
    bq.a1 = a1 / a0;
    bq.a2 = a2 / a0;
}

void NonlinearProcessor::designHighpass(float fc, float fs, SimpleBiquad& bq)
{
    float w0 = 2.0f * M_PI * fc / fs;
    float cosw0 = std::cos(w0);
    float sinw0 = std::sin(w0);
    float Q = std::sqrt(0.5f); // Butterworth
    float alpha = sinw0 / (2.0f * Q);

    float b0 = (1 + cosw0) / 2.0f;
    float b1 = -(1 + cosw0);
    float b2 = (1 + cosw0) / 2.0f;
    float a0 = 1 + alpha;
    float a1 = -2 * cosw0;
    float a2 = 1 - alpha;

    bq.b0 = b0 / a0;
    bq.b1 = b1 / a0;
    bq.b2 = b2 / a0;
    bq.a1 = a1 / a0;
    bq.a2 = a2 / a0;
}
