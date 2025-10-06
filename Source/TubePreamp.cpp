#include "TubePreamp.h"

#ifndef M_PI
namespace
{
    const double M_PI = std::acos(-1.0);
}
#endif

TubePreamp::TubePreamp()
{
    inputGain = 3.0f;
    bias = 0.0f;
    outputGain = 1.0f;
    toneCutoff = 5000.0f; // dafault 5 kHz
    sampleRate = 44100.0f;

    a0 = 0.0f;
    b1 = 0.0f;
    z1 = 0.0f;

    updateToneFilter();
}

// --- Params setting ---
void TubePreamp::setSampleRate(float sr)
{
    sampleRate = sr;
    updateToneFilter();
}

void TubePreamp::setGain(float g)
{
    inputGain = g;
}

void TubePreamp::setBias(float b)
{
    bias = b;
}

void TubePreamp::setOutput(float o)
{
    outputGain = o;
}

// --- LPF ---
void TubePreamp::setTone(float cutoffHz)
{
    // ograniczamy zakres aby unikn¹æ niestabilnoœci filtra
    if (cutoffHz < 20.0f) cutoffHz = 20.0f;
    if (cutoffHz > sampleRate * 0.45f) cutoffHz = sampleRate * 0.45f;

    toneCutoff = cutoffHz;
    updateToneFilter();
}


float TubePreamp::process(float input)
{
    // Input inputGain + bias
    float x = inputGain * input + bias;

    // tube nonlinearity
    float nonlinear = std::tanh(0.8f * x);

    // LPF
    float filtered = toneFilter(nonlinear);

    // output
    return outputGain * filtered;
}

// --- LPF 1 order ---
float TubePreamp::toneFilter(float x)
{
    float y = a0 * x + b1 * z1;
    z1 = y;
    return y;
}

// --- LPF params update ---
void TubePreamp::updateToneFilter()
{
    float x = std::exp(-2.0f * static_cast<float>(M_PI) * toneCutoff / sampleRate);
    a0 = 1.0f - x;
    b1 = x;
}
