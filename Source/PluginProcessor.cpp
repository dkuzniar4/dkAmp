/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ampProfiles.h"

//==============================================================================
DkAmpAudioProcessor::DkAmpAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
    , params(apvts)
{
}

DkAmpAudioProcessor::~DkAmpAudioProcessor()
{
}

//==============================================================================
const juce::String DkAmpAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DkAmpAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DkAmpAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DkAmpAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DkAmpAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DkAmpAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DkAmpAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DkAmpAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DkAmpAudioProcessor::getProgramName (int index)
{
    return {};
}

void DkAmpAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DkAmpAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    params.prepareToPlay(sampleRate);
    params.reset();

    eq[0].initialise(sampleRate, 200.0f, 1000.0f, 8000.0f);
    eq[1].initialise(sampleRate, 200.0f, 1000.0f, 8000.0f);

    auto filePath = apvts.state.getProperty("IR_file").toString();
    juce::File file(filePath);

    auto* cabEnableParam = apvts.getRawParameterValue(cabEnableParamID.getParamID());;

    bool cabEnabled = (cabEnableParam->load() > 0.5f);

    cabSim[0].setEnable(cabEnabled);
    cabSim[1].setEnable(cabEnabled);

    if (file.existsAsFile())
    {
        cabSim[0].loadIR(file);
        cabSim[1].loadIR(file);
    }

    //tran[0].load(&dkAmpProfile[0][0]);
    tran[0].load(&_57customChamp[0][0]);
    tran[0].init(100, 200.0f, 1000.0f, 8000.0f, sampleRate);

    //tran[1].load(&dkAmpProfile[0][0]);
    tran[1].load(&_57customChamp[0][0]);
    tran[1].init(100, 200.0f, 1000.0f, 8000.0f, sampleRate);
}

void DkAmpAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DkAmpAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mono = juce::AudioChannelSet::mono();
    const auto stereo = juce::AudioChannelSet::stereo();
    const auto mainIn = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();

    //DBG("isBusesLayoutSupported, in: " << mainIn.getDescription()
    //        << ", out: " << mainOut.getDescription());

    if (mainIn == mono && mainOut == mono) { return true; }
    if (mainIn == mono && mainOut == stereo) { return true; }

    return false;
}
#endif

void DkAmpAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    params.update();

    auto mainOutput = getBusBuffer(buffer, false, 0);
    auto mainOutputChannels = mainOutput.getNumChannels();
    auto isMainOutputStereo = mainOutputChannels > 1;
    float* outputDataL = mainOutput.getWritePointer(0);
    float* outputDataR = mainOutput.getWritePointer(isMainOutputStereo ? 1 : 0);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const float* inputData = buffer.getReadPointer(channel);
        float* outputData = mainOutput.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            params.smoothen();

            float input = inputData[sample];

            if (!params.bypassed)
            {
                float signal;

                // alternative non-linear function
                //signal = softClipWaveShaper(input, params.gain);

                // transient non-linear function
                signal = tran[channel].process(input * (params.gain / 10.0f));

                // to do lastEqLow - is common for both channels !!!
                if (params.eqLow != lastEqLow)
                {
                    eq[channel].setLowGain(params.eqLow);
                    lastEqLow = params.eqLow;
                }

                if (params.eqMid != lastEqMid)
                {
                    eq[channel].setMidGain(params.eqMid);
                    lastEqMid = params.eqMid;
                }

                if (params.eqHigh != lastEqHigh)
                {
                    eq[channel].setHighGain(params.eqHigh);
                    lastEqHigh = params.eqHigh;
                }

                signal = eq[channel].processSample(signal);

                signal = cabSim[channel].process(signal);

                signal = signal * params.output;

                outputData[sample] = signal;
            }
            else
            {
                outputData[sample] = input;
            }
        }
    }
}

//==============================================================================
bool DkAmpAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DkAmpAudioProcessor::createEditor()
{
    return new DkAmpAudioProcessorEditor (*this);
}

//==============================================================================
void DkAmpAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    copyXmlToBinary(*apvts.copyState().createXml(), destData);
}

void DkAmpAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType())) {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DkAmpAudioProcessor();
}

juce::AudioProcessorParameter* DkAmpAudioProcessor::getBypassParameter() const
{
    return params.bypassParam;
}
