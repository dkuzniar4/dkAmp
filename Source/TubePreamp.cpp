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
    sampleRate = 48000.0f;


    updateToneFilter();
    updateDCBlocker();
}

// --- Params setting ---
void TubePreamp::setSampleRate(float sr)
{
    sampleRate = sr;
    updateToneFilter();
    updateDCBlocker();
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

void TubePreamp::updateDCBlocker()
{
    float dcCutoff = 10.0f; // 10 Hz
    dc_R = std::exp(-2.0f * static_cast<float>(M_PI) * dcCutoff / sampleRate);
}

float TubePreamp::DC_blocker(float x)
{
    float dcOut = x - dc_x1 + dc_R * dc_y1;
    dc_x1 = x;
    dc_y1 = dcOut;

    return dcOut;
}

float TubePreamp::process(float input)
{
    // Input inputGain + bias
    float signal = inputGain * input + bias;

    // tube nonlinearity
    signal = std::tanh(0.8f * signal);

    // LPF
    signal = toneFilter(signal);

    // HPF
    signal = DC_blocker(signal);

    // output
    return outputGain * signal;
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
