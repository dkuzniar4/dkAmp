#include "Parameters.h"


template<typename T>
static void castParameter(juce::AudioProcessorValueTreeState& apvts,
    const juce::ParameterID& id, T& destination)
{
    destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
    jassert(destination);  // parameter does not exist or wrong type
}

static juce::String stringGain2f(float value, int)
{
    return juce::String(value, 2);
}

static juce::String stringFromMilliseconds(float value, int)
{
    if (value < 10.0f) {
        return juce::String(value, 2) + " ms";
    }
    else if (value < 100.0f) {
        return juce::String(value, 1) + " ms";
    }
    else if (value < 1000.0f) {
        return juce::String(int(value)) + " ms";
    }
    else {
        return juce::String(value * 0.001f, 2) + " s";
    }
}

static float millisecondsFromString(const juce::String& text)
{
    float value = text.getFloatValue();
    if (!text.endsWithIgnoreCase("ms")) {
        if (text.endsWithIgnoreCase("s") || value < 1) {
            return value * 1000.0f;
        }
    }
    return value;
}

static juce::String stringFromDecibels(float value, int)
{
    return juce::String(value, 1) + " dB";
}

static juce::String stringFromPercent(float value, int)
{
    return juce::String(int(value)) + " %";
}

static juce::String stringFromHz(float value, int)
{
    if (value < 1000.0f) {
        return juce::String(int(value)) + " Hz";
    }
    else if (value < 10000.0f) {
        return juce::String(value / 1000.0f, 2) + " k";
    }
    else {
        return juce::String(value / 1000.0f, 1) + " k";
    }
}

static float hzFromString(const juce::String& str)
{
    float value = str.getFloatValue();
    if (value < 20.0f) {
        return value * 1000.0f;
    }
    return value;
}

Parameters::Parameters(juce::AudioProcessorValueTreeState& apvts)
{
    castParameter(apvts, gainParamID, gainParam);
    castParameter(apvts, outputParamID, outputParam);
    castParameter(apvts, eqLowParamID, eqLowParam);
    castParameter(apvts, eqMidParamID, eqMidParam);
    castParameter(apvts, eqHighParamID, eqHighParam);
    castParameter(apvts, bypassParamID, bypassParam);
    castParameter(apvts, cabEnableParamID, cabEnableParam);
    castParameter(apvts, lowGainParamID, lowGainParam);
    castParameter(apvts, midGainParamID, midGainParam);
    castParameter(apvts, highGainParamID, highGainParam);
    castParameter(apvts, cabNormParamID, cabNormParam);
    
    update();
}

juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        gainParamID,
        "Input Gain",
        juce::NormalisableRange<float> { 0.0f, 10.0f },
        3.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction(stringGain2f)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        outputParamID,
        "Output gain",
        juce::NormalisableRange<float> { -20.0f, 20.0f },
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction(stringFromDecibels)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        eqLowParamID,
        "Eq low",
        juce::NormalisableRange<float> { -12.0f, 12.0f },
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(stringFromDecibels)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        eqMidParamID,
        "Eq mid",
        juce::NormalisableRange<float> { -12.0f, 12.0f },
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(stringFromDecibels)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        eqHighParamID,
        "Eq high",
        juce::NormalisableRange<float> { -12.0f, 12.0f },
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(stringFromDecibels)
    ));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        bypassParamID, "Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        cabEnableParamID, "Cab Enable", false));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        cabNormParamID, "Cab normalize", false));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        lowGainParamID, "Low gain enable", false));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        midGainParamID, "Mid gain enable", false));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        highGainParamID, "High gain enable", false));


    return layout;
}

void Parameters::prepareToPlay(double sampleRate) noexcept
{
    double duration = 0.02;
    gainSmoother.reset(sampleRate, duration);
    outputSmoother.reset(sampleRate, duration);
    eqLowSmoother.reset(sampleRate, duration);
    eqMidSmoother.reset(sampleRate, duration);
    eqHighSmoother.reset(sampleRate, duration);
}

void Parameters::reset() noexcept
{
    gain = 0.0f;
    gainSmoother.setCurrentAndTargetValue(gainParam->get());
    
    output = 0.0f;
    outputSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(outputParam->get()));
    
    eqLow = 0.0f;
    eqLowSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(eqLowParam->get()));

    eqMid = 0.0f;
    eqMidSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(eqMidParam->get()));

    eqHigh = 0.0f;
    eqHighSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(eqHighParam->get()));
}

void Parameters::update() noexcept
{
    gainSmoother.setTargetValue(gainParam->get());
    outputSmoother.setTargetValue(juce::Decibels::decibelsToGain(outputParam->get()));
    eqLowSmoother.setTargetValue(eqLowParam->get());
    eqMidSmoother.setTargetValue(eqMidParam->get());
    eqHighSmoother.setTargetValue(eqHighParam->get());

    bypassed = bypassParam->get();
    cabEnabled = cabEnableParam->get();
    lowGain = lowGainParam->get();
    midGain = midGainParam->get();
    highGain = highGainParam->get();
    cabNorm = cabNormParam->get();
}

void Parameters::smoothen() noexcept
{
    gain = gainSmoother.getNextValue();
    output = outputSmoother.getNextValue();
    eqLow = eqLowSmoother.getNextValue();
    eqMid = eqMidSmoother.getNextValue();
    eqHigh = eqHighSmoother.getNextValue();
}
