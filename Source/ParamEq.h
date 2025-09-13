/*
  ==============================================================================

    ParamEq.h
    Created: 30 Aug 2025 7:40:12pm
    Author:  dkuzn

  ==============================================================================
*/
#pragma once


class Biquad
{
public:
    enum Type { LowShelf, HighShelf, Peak };

    Biquad();

    void setParams(Type type, double sampleRate, double freq, double Q, float gainDB);

    inline float processSample(float x)
    {
        double y = b0 * x + b1 * x1 + b2 * x2
            - a1 * y1 - a2 * y2;

        // shift data
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;

        return (float)y;
    }

    void reset();

private:
    Type type;
    double b0 = 1.0, b1 = 0.0, b2 = 0.0;
    double a1 = 0.0, a2 = 0.0;

    double x1 = 0.0, x2 = 0.0;
    double y1 = 0.0, y2 = 0.0;
};

// ==========================
// 3-band EQ
// ==========================

class SimpleEQ
{
public:

    SimpleEQ();

    void initialise(double sampleRate, float freqLow, float freqMid, float freqHigh);

    void setLowGain(float dB);
    void setMidGain(float dB);
    void setHighGain(float dB);

    inline float processSample(float x)
    {
        float y = x;
        y = low.processSample(y);
        y = mid.processSample(y);
        y = high.processSample(y);
        return y;
    }

    void processBlock(float* samples, int numSamples);

    void reset();

private:
    double sampleRate = 44100.0;
    float freqLow = 200.0f;
    float freqMid = 1000.0f;
    float freqHigh = 8000.0f;

    float lowGain = 0.0f;
    float midGain = 0.0f;
    float highGain = 0.0f;

    Biquad low, mid, high;

    void updateFilters();
};
