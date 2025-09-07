/*
  ==============================================================================

    ParamEq.h
    Created: 30 Aug 2025 7:40:12pm
    Author:  dkuzn

  ==============================================================================
*/
#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

#ifndef M_PI
namespace
{
    const double M_PI = std::acos(-1.0);
}
#endif

class Biquad
{
public:
    enum Type { LowShelf, HighShelf, Peak };

    Biquad() {}

    void setParams(Type type, double sampleRate, double freq, double Q, float gainDB)
    {
        this->type = type;

        double A = std::pow(10.0, gainDB / 40.0); // gain linear
        double w0 = 2.0 * M_PI * freq / sampleRate;
        double alpha = std::sin(w0) / (2.0 * Q);
        double cosw0 = std::cos(w0);

        double b0, b1, b2, a0, a1, a2;

        if (type == LowShelf)
        {
            double sqrtA = std::sqrt(A);
            b0 = A * ((A + 1) - (A - 1) * cosw0 + 2 * sqrtA * alpha);
            b1 = 2 * A * ((A - 1) - (A + 1) * cosw0);
            b2 = A * ((A + 1) - (A - 1) * cosw0 - 2 * sqrtA * alpha);
            a0 = (A + 1) + (A - 1) * cosw0 + 2 * sqrtA * alpha;
            a1 = -2 * ((A - 1) + (A + 1) * cosw0);
            a2 = (A + 1) + (A - 1) * cosw0 - 2 * sqrtA * alpha;
        }
        else if (type == HighShelf)
        {
            double sqrtA = std::sqrt(A);
            b0 = A * ((A + 1) + (A - 1) * cosw0 + 2 * sqrtA * alpha);
            b1 = -2 * A * ((A - 1) + (A + 1) * cosw0);
            b2 = A * ((A + 1) + (A - 1) * cosw0 - 2 * sqrtA * alpha);
            a0 = (A + 1) - (A - 1) * cosw0 + 2 * sqrtA * alpha;
            a1 = 2 * ((A - 1) - (A + 1) * cosw0);
            a2 = (A + 1) - (A - 1) * cosw0 - 2 * sqrtA * alpha;
        }
        else // Peak
        {
            b0 = 1 + alpha * A;
            b1 = -2 * cosw0;
            b2 = 1 - alpha * A;
            a0 = 1 + alpha / A;
            a1 = -2 * cosw0;
            a2 = 1 - alpha / A;
        }

        // normalizacja
        this->b0 = b0 / a0;
        this->b1 = b1 / a0;
        this->b2 = b2 / a0;
        this->a1 = a1 / a0;
        this->a2 = a2 / a0;
    }

    inline float processSample(float x)
    {
        double y = b0 * x + b1 * x1 + b2 * x2
            - a1 * y1 - a2 * y2;

        // przesuniêcie historii
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;

        return (float)y;
    }

    void reset()
    {
        x1 = x2 = y1 = y2 = 0.0;
    }

private:
    Type type;
    double b0 = 1.0, b1 = 0.0, b2 = 0.0;
    double a1 = 0.0, a2 = 0.0;

    double x1 = 0.0, x2 = 0.0;
    double y1 = 0.0, y2 = 0.0;
};

// ==========================
// 3-pasmowy EQ
// ==========================

class SimpleEQ
{
public:
    void initialise(double sampleRate, float freqLow, float freqMid, float freqHigh)
    {
        this->sampleRate = sampleRate;
        this->freqLow = freqLow;
        this->freqMid = freqMid;
        this->freqHigh = freqHigh;

        updateFilters();
    }

    void setLowGain(float dB) { lowGain = dB;   updateFilters(); }
    void setMidGain(float dB) { midGain = dB;   updateFilters(); }
    void setHighGain(float dB) { highGain = dB;  updateFilters(); }

    inline float processSample(float x)
    {
        float y = x;
        y = low.processSample(y);
        y = mid.processSample(y);
        y = high.processSample(y);
        return y;
    }

    void processBlock(float* samples, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
            samples[i] = processSample(samples[i]);
    }

    void reset()
    {
        low.reset();
        mid.reset();
        high.reset();
    }

private:
    double sampleRate = 44100.0;
    float freqLow = 200.0f;
    float freqMid = 1000.0f;
    float freqHigh = 8000.0f;

    float lowGain = 0.0f;
    float midGain = 0.0f;
    float highGain = 0.0f;

    Biquad low, mid, high;

    void updateFilters()
    {
        low.setParams(Biquad::LowShelf, sampleRate, freqLow, 0.707, lowGain);
        mid.setParams(Biquad::Peak, sampleRate, freqMid, 0.707, midGain);
        high.setParams(Biquad::HighShelf, sampleRate, freqHigh, 0.707, highGain);
    }
};
