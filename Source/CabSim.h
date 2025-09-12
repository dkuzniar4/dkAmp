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
#include <cmath>



class AudioLoader
{
public:
    AudioLoader();
    bool loadWavFile(const juce::File& file);
    juce::AudioBuffer<float>& getAudioBuffer();

    juce::AudioBuffer<float> audioBuffer;

private:
    juce::AudioFormatManager formatManager;

};

class FFT
{
public:
    FFT();
    void FFT_process(float* Re, float* Im, uint32_t size);
    void IFFT_process(float* Re, float* Im, uint32_t size);
    void rectangularToPolar(float* Re, float* Im, float* Mag, float* Phase, uint32_t size);
    void polarToRectangular(float* Mag, float* Phase, float* Re, float* Im, uint32_t size);
    uint32_t calculateFFTWindow(uint32_t length);

private:

};

class FIR_FFT_OLS
{
public:
    FIR_FFT_OLS();
    ~FIR_FFT_OLS();
    void setFFTSize(uint32_t fftSize);
    void prepare(const float* h, uint32_t h_len);
    float process(float input);
    void setBypass(bool state);
    uint32_t calculateFFTWindow(uint32_t length);

private:
    FFT fft;

    float** h_fft_Re = nullptr;
    float** h_fft_Im = nullptr;
    float* h_norm = nullptr;
    float* inputBufferRe = nullptr;    // Input samples buffer
    float* inputBufferIm = nullptr;
    float* inputBuffer = nullptr;
    float* overlapBuffer = nullptr; // Overlap buffer for OLA
    float* outputBuffer = nullptr;   // Output samples buffer
    float* mulBufferRe = nullptr;
    float* mulBufferIm = nullptr;
    uint32_t bufferIndex = 0;
    uint32_t outputBufferIndex = 0;
    uint32_t fftSize = 0;
    uint32_t fftSizeHalf = 0;
    uint32_t IR_len = 0;
    uint32_t numSegments = 0;
    bool bypass = false;
};

class Convolver
{
public:
    Convolver();
    float process(float input);
    void loadIR(const juce::File& file);
    void setSampleRate(uint32_t sampleRate);
    uint32_t getSampleRate(void);
    void setEnable(bool enable);

    bool IR_loaded;
    uint32_t IR_len;

private:
    AudioLoader IR_loader;
    FIR_FFT_OLS fir_fft_ols;
    FFT fft;
    uint32_t sampleRate;
    bool enable = false;

    const float* IR_ptr;
};
