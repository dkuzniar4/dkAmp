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
                       .withInput  ("Input",  juce::AudioChannelSet::mono(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::mono(), true)
                     #endif
                       )
#endif
    , params(apvts)
{
#if LOGGER_ENABLE
    auto logFile = juce::File::getSpecialLocation((juce::File::userHomeDirectory))
        .getChildFile("myLogs.txt");


            logger.reset(juce::FileLogger::createDefaultAppLogger(
                logFile.getParentDirectory().getFullPathName(),
                logFile.getFileName(),
                "==== Start Plugin Log ===="));

            juce::Logger::setCurrentLogger(logger.get());
#endif
}

DkAmpAudioProcessor::~DkAmpAudioProcessor()
{
#if LOGGER_ENABLE
    juce::Logger::writeToLog("Plugin closing...");
    juce::Logger::setCurrentLogger(nullptr);
    logger = nullptr;
#endif
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
    this->sampleRate = sampleRate;
    this->samplesPerBlock = samplesPerBlock;

    params.prepareToPlay(sampleRate);
    params.reset();
    params.update();

    eq.initialise(sampleRate, 250.0f, 800.0f, 3000.0f);
    
    auto filePath = apvts.state.getProperty("IR_file").toString();

    if (filePath.isNotEmpty())
    {
        juce::File file(filePath);
        
        if (file.existsAsFile())
        {
            cabSim.loadIR(file);
        }
    }

    tran.load(&deluxeRev[0][0]);
    //tran.load(&_57customChamp[0][0]);
    tran.init(100, FREQ_1, FREQ_2, sampleRate);

    comp.setRatio(4.0f);
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
    
    cabSim.setEnable(params.cabEnabled);

    tran.enableLow(params.lowGain);
    tran.enableMid(params.midGain);
    tran.enableHigh(params.highGain);

    const float* inputData = buffer.getReadPointer(0);
    float* outputData = buffer.getWritePointer(0);
 

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        params.smoothen();

        float signal = inputData[sample];

        if (!params.bypassed)
        {
            if (params.eqLow != lastEqLow)
            {
                eq.setLowGain(params.eqLow);
                lastEqLow = params.eqLow;
            }

            if (params.eqMid != lastEqMid)
            {
                eq.setMidGain(params.eqMid);
                lastEqMid = params.eqMid;
            }

            if (params.eqHigh != lastEqHigh)
            {
                eq.setHighGain(params.eqHigh);
                lastEqHigh = params.eqHigh;
            }

            signal = eq.processSample(signal);

            // alternative non-linear function
            //signal = softClipWaveShaper(signal, params.gain);

            // transistion non-linear function
            signal = tran.process(signal * (params.gain / 10.0f));

            signal = cabSim.process(signal);

            signal = comp.process(signal, 0.6f, true);

            signal = signal * params.output;

            outputData[sample] = signal;
        }
        else
        {
            outputData[sample] = signal;
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
    
    prepareToPlay(this->sampleRate, this->samplesPerBlock);
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
