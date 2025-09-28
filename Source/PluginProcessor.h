/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Parameters.h"
#include "Preamp.h"
#include "Nonlinear.h"
#include "ParamEq.h"
#include "CabSim.h"
#include "ShapeProcessor.h"


//==============================================================================
/**
*/
class DkAmpAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DkAmpAudioProcessor();
    ~DkAmpAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorParameter* getBypassParameter() const override;

    juce::AudioProcessorValueTreeState apvts{
        *this, nullptr, "Parameters", Parameters::createParameterLayout()
    };

#if LOGGER_ENABLE
    std::unique_ptr<juce::FileLogger> logger;
#endif

    Parameters params;
    Convolver cabSim;

private:
    double sampleRate;
    int samplesPerBlock;

    SimpleEQ eq;
    NonlinearProcessor tran;
    ShapeProcessor shP;


    float lastEqLow = 0.0f;
    float lastEqMid = 0.0f;
    float lastEqHigh = 0.0f;

    juce::File lastOpenedFolder;
    juce::File lastOpenedFile;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DkAmpAudioProcessor)
};
