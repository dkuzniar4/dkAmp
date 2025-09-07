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
    uint32_t calculateFFTWindow(uint32_t length);

private:
    FFT fft;

    float** h_fft_Re;
    float** h_fft_Im;
    float* inputBufferRe;    // Input samples buffer
    float* inputBufferIm;
    float* inputBuffer;
    float* overlapBuffer; // Overlap buffer for OLA
    float* outputBuffer;   // Output samples buffer
    float* mulBufferRe;
    float* mulBufferIm;
    uint32_t bufferIndex = 0;
    uint32_t outputBufferIndex = 0;
    uint32_t fftSize = 0;
    uint32_t fftSizeHalf = 0;
    uint32_t IR_len = 0;
    uint32_t numSegments;
};

class Convolver
{
public:
    Convolver();
    float process(float input);
    void loadIR(const juce::File& file);
    void setSampleRate(uint32_t sampleRate);
    uint32_t getSampleRate(void);

    bool IR_loaded;
    uint32_t IR_len;

private:
    AudioLoader IR_loader;
    FIR_FFT_OLS fir_fft_ols;
    FFT fft;
    uint32_t sampleRate;

    const float* IR_ptr;
};
