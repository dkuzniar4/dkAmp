/*
  ==============================================================================

    CabSim.h
    Created: 30 Aug 2025 7:40:48pm
    Author:  dkuzn

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cstring>
#include "FFT.h"
#include "Resampler.h"


class AudioLoader
{
public:
    AudioLoader();
    bool loadWavFile(const juce::File& file);
    juce::AudioBuffer<float>& getAudioBuffer();

    juce::AudioBuffer<float> audioBuffer;

    uint32_t fileSampleRate = 0.0;
private:
    juce::AudioFormatManager formatManager;

};


class FIR_FFT_OLS
{
public:
    FIR_FFT_OLS();
    ~FIR_FFT_OLS();
    void setFFTSize(uint32_t fftSize);
    void prepare(const float* h, uint32_t h_len);
    float process(float input);
    void setNormFactor(float value);
    void clearBuffers();

    bool normalize = false;
    FFT fft;

private:

    std::vector<std::vector<float>> h_fft_Re;
    std::vector<std::vector<float>> h_fft_Im;
    std::vector<float> inputBufferRe; // Input samples buffer
    std::vector<float> inputBufferIm;
    std::vector<float> inputBuffer;
    std::vector<float> overlapBuffer;
    std::vector<float> outputBuffer;
    std::vector<float> mulBufferRe;
    std::vector<float> mulBufferIm;
    // ring buffer for input block FFTs
    std::vector<std::vector<float>> inputFFT_Re;
    std::vector<std::vector<float>> inputFFT_Im;
    uint32_t fftRingPos = 0; // point place where to save actual FFT block
    uint32_t bufferIndex = 0;
    uint32_t outputBufferIndex = 0;
    uint32_t fftSize = 0;
    uint32_t fftSizeHalf = 0;
    uint32_t IR_len = 0;
    uint32_t numSegments = 0;
    float normFactor = 1.0f;
};

class Convolver
{
public:
    Convolver();
    ~Convolver();
    void init(double sampleRate, int blockLength);
    float process(float input);
    void loadIR(const juce::File& file);
    void setEnable(bool enable);
    void setNormalize(bool enable);
    void normalize();

    bool IR_loaded = false;
    uint32_t IR_len = 0;
    bool normPending = false;

private:
    AudioLoader IR_loader;
    FIR_FFT_OLS fir_fft_ols;
    Resampler rs;
    float* IR = nullptr;
    double sampleRate = 48000.0;
    int blockLength = 64;
    uint32_t fftSizeN;
    bool enable = false;
    bool reinitFlag = false;

    const float* IR_ptr = nullptr;
};
