/*
  ==============================================================================

    ParamEq.cpp
    Created: 30 Aug 2025 7:40:12pm
    Author:  dkuzn

  ==============================================================================
*/

#include "ParamEq.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

#ifndef M_PI
namespace
{
    const double M_PI = std::acos(-1.0);
}
#endif

Biquad::Biquad()
{

}

void Biquad::setParams(Type type, double sampleRate, double freq, double Q, float gainDB)
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

    // normalisation
    this->b0 = b0 / a0;
    this->b1 = b1 / a0;
    this->b2 = b2 / a0;
    this->a1 = a1 / a0;
    this->a2 = a2 / a0;
}

void Biquad::reset()
{
    x1 = x2 = y1 = y2 = 0.0;
}



SimpleEQ::SimpleEQ()
{

}

void SimpleEQ::initialise(double sampleRate, float freqLow, float freqMid, float freqHigh)
{
    this->sampleRate = sampleRate;
    this->freqLow = freqLow;
    this->freqMid = freqMid;
    this->freqHigh = freqHigh;

    updateFilters();
}

void SimpleEQ::setLowGain(float dB)
{
    lowGain = dB;
    updateFilters();
}

void SimpleEQ::setMidGain(float dB)
{
    midGain = dB;
    updateFilters();
}

void SimpleEQ::setHighGain(float dB)
{
    highGain = dB;
    updateFilters();
}

void SimpleEQ::processBlock(float* samples, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        samples[i] = processSample(samples[i]);
}

void SimpleEQ::reset()
{
    low.reset();
    mid.reset();
    high.reset();
}

void SimpleEQ::updateFilters()
{
    low.setParams(Biquad::LowShelf, sampleRate, freqLow, 0.707, lowGain);
    mid.setParams(Biquad::Peak, sampleRate, freqMid, 0.707, midGain);
    high.setParams(Biquad::HighShelf, sampleRate, freqHigh, 0.707, highGain);
}

