/*
  ==============================================================================

    CabSim.cpp
    Created: 30 Aug 2025 7:40:48pm
    Author:  dkuzn

  ==============================================================================
*/

#include "CabSim.h"
#include <cmath>

#define IR_NORM_FACTOR 0.95f

AudioLoader::AudioLoader()
{
    formatManager.registerBasicFormats();
}

bool AudioLoader::loadWavFile(const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader.get() != nullptr)
    {
        // Create buffer
        audioBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&audioBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
        return true;
    }
    return false;
}

juce::AudioBuffer<float>& AudioLoader::getAudioBuffer()
{
    return audioBuffer;
}


FIR_FFT_OLS::FIR_FFT_OLS()
{

}

FIR_FFT_OLS::~FIR_FFT_OLS()
{
    delete[] inputBufferRe;
    delete[] inputBufferIm;
    delete[] inputBuffer;
    delete[] overlapBuffer;
    delete[] outputBuffer;
    delete[] h_norm;
    delete[] mulBufferRe;
    delete[] mulBufferIm;

    if (h_fft_Re)
    {
        for (uint32_t i = 0; i < numSegments; i++) delete[] h_fft_Re[i];
        delete[] h_fft_Re;
    }
    if (h_fft_Im)
    {
        for (uint32_t i = 0; i < numSegments; i++) delete[] h_fft_Im[i];
        delete[] h_fft_Im;
    }
}

void FIR_FFT_OLS::setFFTSize(uint32_t size)
{
    fftSize = size;
    fftSizeHalf = fftSize / 2;

    if(inputBufferRe != nullptr)
        delete[] inputBufferRe;

    inputBufferRe = new float[fftSize];
    std::memset(inputBufferRe, 0, fftSize * sizeof(float));

    if(inputBufferIm != nullptr)
        delete[] inputBufferIm;

    inputBufferIm = new float[fftSize];
    std::memset(inputBufferIm, 0, fftSize * sizeof(float));

    if(inputBuffer != nullptr)
        delete[] inputBuffer;

    inputBuffer = new float[fftSizeHalf];
    std::memset(inputBuffer, 0, fftSizeHalf * sizeof(float));

    if(mulBufferRe != nullptr)
        delete[] mulBufferRe;

    mulBufferRe = new float[fftSize];
    std::memset(mulBufferRe, 0, fftSize * sizeof(float));

    if(mulBufferIm != nullptr)
        delete[] mulBufferIm;

    mulBufferIm = new float[fftSize];
    std::memset(mulBufferIm, 0, fftSize * sizeof(float));

    if(overlapBuffer != nullptr)
        delete[] overlapBuffer;

    overlapBuffer = new float[fftSizeHalf];
    std::memset(overlapBuffer, 0, fftSizeHalf * sizeof(float));

    if(outputBuffer != nullptr)
        delete[] outputBuffer;

    outputBuffer = new float[fftSizeHalf];
    std::memset(outputBuffer, 0, fftSizeHalf * sizeof(float));
}

void FIR_FFT_OLS::setBypass(bool state)
{
    bypass = state;
}

void FIR_FFT_OLS::prepare(const float* h, uint32_t h_len)
{
    IR_len = h_len;
    numSegments = ceil(((float)(IR_len)) / ((float)(fftSizeHalf)));

    if (h_norm != nullptr)
        delete[] h_norm;

    h_norm = new float[IR_len];

    // normalise h
    float L1 = 0.0f;
    for (uint32_t i = 0u; i < IR_len; i++)
    {
        L1 += h[i];
    }

    float normFactor;

    if (L1 > IR_NORM_FACTOR)
    {
        normFactor = IR_NORM_FACTOR / L1;
    }
    else
    {
        normFactor = 1.0f;
    }

    for (uint32_t i = 0u; i < IR_len; i++)
    {
        h_norm[i] = normFactor * h[i];
    }

    h_fft_Re = new float* [numSegments];
    h_fft_Im = new float* [numSegments];

    for (uint32_t i = 0; i < numSegments; i++)
    {
        h_fft_Re[i] = new float[fftSize];
        h_fft_Im[i] = new float[fftSize];
        std::memset(h_fft_Re[i], 0, fftSize * sizeof(float));
        std::memset(h_fft_Im[i], 0, fftSize * sizeof(float));

        // Copying IR segment to FFT buffer
        uint32_t startIdx = i * (fftSizeHalf);
        uint32_t copyLength = std::min(h_len - startIdx, fftSizeHalf);
        std::memcpy(h_fft_Re[i], &h_norm[startIdx], copyLength * sizeof(float));

        // FFT on segment
        fft.FFT_process(h_fft_Re[i], h_fft_Im[i], fftSize);
    }
}

uint32_t FIR_FFT_OLS::calculateFFTWindow(uint32_t length)
{
    uint32_t fftSize = 1;
    while (fftSize < length)
    {
        fftSize *= 2; // Find the next power of 2 larger than 2 * IR_len
    }
    return fftSize;
}

float FIR_FFT_OLS::process(float input)
{
    if(bypass == true)
    {
        return input;
    }
    else
    {
        // Add sample to input buffer
        inputBufferRe[bufferIndex + fftSizeHalf] = input;
        inputBuffer[bufferIndex] = input;
        bufferIndex++;

        // Check if input buffer is full
        if (bufferIndex >= fftSizeHalf)
        {
            // Prepare input data with overlap
            std::memcpy(inputBufferRe, overlapBuffer, fftSizeHalf * sizeof(float));

            // FFT
            fft.FFT_process(inputBufferRe, inputBufferIm, fftSize);

            // Clear mulBufferRe and mulBufferIm
            std::memset(mulBufferRe, 0, fftSize * sizeof(float));
            std::memset(mulBufferIm, 0, fftSize * sizeof(float));

            // Sum IR segments
            for (uint32_t seg = 0; seg < numSegments; seg++)
            {
                for (uint32_t i = 0; i < fftSize; i++)
                {
                    float tempRe = (inputBufferRe[i] * h_fft_Re[seg][i]) - (inputBufferIm[i] * h_fft_Im[seg][i]);
                    float tempIm = (inputBufferRe[i] * h_fft_Im[seg][i]) + (inputBufferIm[i] * h_fft_Re[seg][i]);
                    mulBufferRe[i] += tempRe;
                    mulBufferIm[i] += tempIm;
                }
            }

            // IFFT
            fft.IFFT_process(mulBufferRe, mulBufferIm, fftSize);

            // overlap update
            std::memcpy(overlapBuffer, inputBuffer, fftSizeHalf * sizeof(float));

            // Store output to output buffer
            std::memcpy(outputBuffer, &mulBufferRe[fftSizeHalf], fftSizeHalf * sizeof(float));

            // Clean buffer
            std::memset(inputBufferIm, 0, fftSize * sizeof(float));

            bufferIndex = 0;
            outputBufferIndex = 0;
        }

        return outputBuffer[outputBufferIndex++];
    }
}

Convolver::Convolver() : IR_len(0), IR_loaded(false), IR_ptr(nullptr)
{
}

float Convolver::process(float input)
{
    if (IR_loaded == true)
    {
        if (enable == true)
        {
            return fir_fft_ols.process(input);
        }
        else
        {
            return input;
        }
    }
    else
    {
        return input;
    }
}

void Convolver::loadIR(const juce::File& file)
{
    IR_loaded = false;
    IR_loader.loadWavFile(file);
    this->IR_ptr = IR_loader.audioBuffer.getReadPointer(0);
    this->IR_len = IR_loader.audioBuffer.getNumSamples();


    fir_fft_ols.setFFTSize(512);
    fir_fft_ols.setBypass(true);
    fir_fft_ols.prepare(this->IR_ptr, this->IR_len);
    fir_fft_ols.setBypass(false);

    IR_loaded = true;
}

void Convolver::setSampleRate(uint32_t sampleRate)
{
    this->sampleRate = sampleRate;
}

uint32_t Convolver::getSampleRate(void)
{
    return sampleRate;
}

void Convolver::setEnable(bool enable)
{
    this->enable = enable;
}
