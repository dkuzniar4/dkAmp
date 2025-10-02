#pragma once

#include <JuceHeader.h>

#define LOGGER_ENABLE 0u

const juce::ParameterID gainParamID{ "gain", 1 };
const juce::ParameterID outputParamID{ "output", 1 };
const juce::ParameterID eqLowParamID{ "eqLow", 1 };
const juce::ParameterID eqMidParamID{ "eqMid", 1 };
const juce::ParameterID eqHighParamID{ "eqHigh", 1 };
const juce::ParameterID bypassParamID{ "bypass", 1 };
const juce::ParameterID cabEnableParamID{ "cabEnable", 1 };
const juce::ParameterID cabNormParamID{ "cabNorm", 1 };



class Parameters
{
public:
    Parameters(juce::AudioProcessorValueTreeState& apvts);

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void prepareToPlay(double sampleRate) noexcept;
    void reset() noexcept;
    void update() noexcept;
    void smoothen() noexcept;

    float gain = 3.0f;
    float output = 0.0f; // dB
    float eqLow = 0.0f; // dB
    float eqMid = 0.0f; // dB
    float eqHigh = 0.0f; // dB
    bool bypassed = false;
    bool cabEnabled = false;
    bool cabNorm = false;


    juce::AudioParameterBool* bypassParam;
    juce::AudioParameterBool* cabEnableParam;
    juce::AudioParameterBool* cabNormParam;


private:
    juce::AudioParameterFloat* gainParam;
    juce::LinearSmoothedValue<float> gainSmoother;

    juce::AudioParameterFloat* outputParam;
    juce::LinearSmoothedValue<float> outputSmoother;

    juce::AudioParameterFloat* eqLowParam;
    juce::LinearSmoothedValue<float> eqLowSmoother;

    juce::AudioParameterFloat* eqMidParam;
    juce::LinearSmoothedValue<float> eqMidSmoother;

    juce::AudioParameterFloat* eqHighParam;
    juce::LinearSmoothedValue<float> eqHighSmoother;
};
