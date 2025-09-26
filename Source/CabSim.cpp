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
        fileSampleRate = (uint32_t)(reader->sampleRate);

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
    : fftSize(0),
    fftSizeHalf(0),
    inputBufferRe(nullptr),
    inputBufferIm(nullptr),
    inputBuffer(nullptr),
    overlapBuffer(nullptr),
    outputBuffer(nullptr),
    h_norm(nullptr),
    mulBufferRe(nullptr),
    mulBufferIm(nullptr),
    inputFFT_Re(nullptr),
    inputFFT_Im(nullptr),
    h_fft_Re(nullptr),
    h_fft_Im(nullptr),
    numSegments(0),
    IR_len(0),
    bufferIndex(0),
    outputBufferIndex(0)
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
        for (uint32_t i = 0; i < numSegments; ++i) delete[] h_fft_Re[i];
        delete[] h_fft_Re;
    }
    if (h_fft_Im)
    {
        for (uint32_t i = 0; i < numSegments; ++i) delete[] h_fft_Im[i];
        delete[] h_fft_Im;
    }
}

void FIR_FFT_OLS::setFFTSize(uint32_t size)
{
    // size must be power of two and >= 2
    fftSize = size;
    fftSizeHalf = fftSize / 2;

    // free old
    if (inputBufferRe) delete[] inputBufferRe;
    if (inputBufferIm) delete[] inputBufferIm;
    if (inputBuffer) delete[] inputBuffer;
    if (mulBufferRe) delete[] mulBufferRe;
    if (mulBufferIm) delete[] mulBufferIm;
    if (overlapBuffer) delete[] overlapBuffer;
    if (outputBuffer) delete[] outputBuffer;

    // allocate and zero
    inputBufferRe = new float[fftSize];   std::memset(inputBufferRe, 0, fftSize * sizeof(float));
    inputBufferIm = new float[fftSize];   std::memset(inputBufferIm, 0, fftSize * sizeof(float));
    inputBuffer = new float[fftSizeHalf]; std::memset(inputBuffer, 0, fftSizeHalf * sizeof(float));
    mulBufferRe = new float[fftSize];   std::memset(mulBufferRe, 0, fftSize * sizeof(float));
    mulBufferIm = new float[fftSize];   std::memset(mulBufferIm, 0, fftSize * sizeof(float));
    overlapBuffer = new float[fftSizeHalf]; std::memset(overlapBuffer, 0, fftSizeHalf * sizeof(float));
    outputBuffer = new float[fftSizeHalf]; std::memset(outputBuffer, 0, fftSizeHalf * sizeof(float));

    // reset indices
    bufferIndex = 0;
    outputBufferIndex = 0;
}

void FIR_FFT_OLS::prepare(const float* h, uint32_t h_len)
{
    IR_len = h_len;

    // -- copying IR --
    if (h_norm) delete[] h_norm;
    h_norm = new float[IR_len];
    std::memcpy(h_norm, h, IR_len * sizeof(float));

    // -- free old FFT segments --
    if (h_fft_Re)
    {
        for (uint32_t i = 0; i < numSegments; ++i)
        {
            delete[] h_fft_Re[i];
        }
        delete[] h_fft_Re;
        h_fft_Re = nullptr;
    }

    if (h_fft_Im)
    {
        for (uint32_t i = 0; i < numSegments; ++i)
        {
            delete[] h_fft_Im[i];
        }
        delete[] h_fft_Im;
        h_fft_Im = nullptr;
    }

    // --- alocate ring buffers for input FFT  ---
    if (inputFFT_Re)
    {
        for (uint32_t i = 0; i < numSegments; ++i)
        {
            delete[] inputFFT_Re[i];
        }
        delete[] inputFFT_Re;
        inputFFT_Re = nullptr;
    }

    if (inputFFT_Im)
    {
        for (uint32_t i = 0; i < numSegments; ++i)
        {
            delete[] inputFFT_Im[i];
        }
        delete[] inputFFT_Im;
        inputFFT_Im = nullptr;
    }

    numSegments = static_cast<uint32_t>(std::ceil((float)IR_len / (float)fftSizeHalf));

    // -- alocate FFT segments --
    h_fft_Re = new float* [numSegments];
    h_fft_Im = new float* [numSegments];
    for (uint32_t seg = 0; seg < numSegments; ++seg)
    {
        h_fft_Re[seg] = new float[fftSize];
        h_fft_Im[seg] = new float[fftSize];
        std::memset(h_fft_Re[seg], 0, fftSize * sizeof(float));
        std::memset(h_fft_Im[seg], 0, fftSize * sizeof(float));

        uint32_t startIdx = seg * fftSizeHalf;
        uint32_t copyLength = std::min(IR_len - startIdx, fftSizeHalf);
        if (copyLength > 0)
            std::memcpy(h_fft_Re[seg], &h_norm[startIdx], copyLength * sizeof(float));

        fft.FFT_process(h_fft_Re[seg], h_fft_Im[seg], fftSize);
    }

    // -- calculate normFactor in frequency domain --
    float Gmax = 0.0f;
    for (uint32_t k = 0; k < fftSize; ++k)
    {
        float sum = 0.0f;
        for (uint32_t seg = 0; seg < numSegments; ++seg)
            sum += std::sqrt(h_fft_Re[seg][k] * h_fft_Re[seg][k] + h_fft_Im[seg][k] * h_fft_Im[seg][k]);
        if (sum > Gmax) Gmax = sum;
    }

    normFactor = 1.0f / Gmax;

    inputFFT_Re = new float* [numSegments];
    inputFFT_Im = new float* [numSegments];
    for (uint32_t i = 0; i < numSegments; ++i)
    {
        inputFFT_Re[i] = new float[fftSize];
        inputFFT_Im[i] = new float[fftSize];
        std::memset(inputFFT_Re[i], 0, fftSize * sizeof(float));
        std::memset(inputFFT_Im[i], 0, fftSize * sizeof(float));
    }

    fftRingPos = 0;
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
    // push new sample into input window
    inputBufferRe[bufferIndex + fftSizeHalf] = input; // second half
    inputBuffer[bufferIndex] = input;
    ++bufferIndex;

    if (bufferIndex >= fftSizeHalf)
    {
        // prepare input block: first half = overlap, second half = inputBuffer (already written)
        std::memcpy(inputBufferRe, overlapBuffer, fftSizeHalf * sizeof(float));
        std::memset(inputBufferIm, 0, fftSize * sizeof(float)); // imag = 0

        // FFT of current input block -> store into ring at fftRingPos
        fft.FFT_process(inputBufferRe, inputBufferIm, fftSize);
        // copy FFT result into ring slot
        std::memcpy(inputFFT_Re[fftRingPos], inputBufferRe, fftSize * sizeof(float));
        std::memcpy(inputFFT_Im[fftRingPos], inputBufferIm, fftSize * sizeof(float));

        // clear accumulation buffers
        std::memset(mulBufferRe, 0, fftSize * sizeof(float));
        std::memset(mulBufferIm, 0, fftSize * sizeof(float));

        // accumulate contributions: for each H_seg multiply by X_{r - seg}
        for (uint32_t seg = 0; seg < numSegments; ++seg)
        {
            // index of input FFT to use (circular): current - seg
            int idx = int(fftRingPos) - int(seg);
            while (idx < 0) idx += int(numSegments);
            // multiply X_idx * H_seg and accumulate
            float* Xre = inputFFT_Re[idx];
            float* Xim = inputFFT_Im[idx];
            float* Hre = h_fft_Re[seg];
            float* Him = h_fft_Im[seg];

            for (uint32_t k = 0; k < fftSize; ++k)
            {
                float tmpRe = Xre[k] * Hre[k] - Xim[k] * Him[k];
                float tmpIm = Xre[k] * Him[k] + Xim[k] * Hre[k];
                mulBufferRe[k] += tmpRe;
                mulBufferIm[k] += tmpIm;
            }
        }

        // IFFT
        fft.IFFT_process(mulBufferRe, mulBufferIm, fftSize);

        // update overlap buffer with last fftSizeHalf samples of the **input** block
        std::memcpy(overlapBuffer, inputBuffer, fftSizeHalf * sizeof(float));

        // copy valid output (second half)
        std::memcpy(outputBuffer, &mulBufferRe[fftSizeHalf], fftSizeHalf * sizeof(float));

        // advance ring position
        fftRingPos = (fftRingPos + 1) % numSegments;

        // reset counters for next hop
        bufferIndex = 0;
        outputBufferIndex = 0;
    }

    // return next sample from output buffer
    float out = 0.0f;
    if (normalize)
    {
        out = outputBuffer[outputBufferIndex++] * normFactor;
    }
    else
    {
        out = outputBuffer[outputBufferIndex++];
    }
    
    if (outputBufferIndex >= fftSizeHalf) outputBufferIndex = fftSizeHalf; // clamp until next block
    return out;
}

Convolver::Convolver() : IR(nullptr)
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

    uint32_t IR_length;

    if ((IR_loader.fileSampleRate != sampleRate) && (sampleRate != 0))
    {
        if (IR)
        {
            delete[] IR;
        }
        IR = nullptr;

        // resampling the IR
        rs.resample(IR_loader.fileSampleRate, sampleRate, IR_loader.audioBuffer.getReadPointer(0), IR_loader.audioBuffer.getNumSamples(), IR, IR_length);

        this->IR_ptr = IR;
        this->IR_len = IR_length;
    }
    else
    {
        this->IR_ptr = (float*)IR_loader.audioBuffer.getReadPointer(0);
        this->IR_len = IR_loader.audioBuffer.getNumSamples();
    }

    fir_fft_ols.setFFTSize(512);
    fir_fft_ols.prepare(this->IR_ptr, this->IR_len);

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

void Convolver::setNormalize(bool enable)
{
    fir_fft_ols.normalize = enable;
}
