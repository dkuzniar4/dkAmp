/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Parameters.h"
#include "RotaryKnob.h"
#include "LookAndFeel.h"

//==============================================================================
/**
*/
class DkAmpAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DkAmpAudioProcessorEditor (DkAmpAudioProcessor&);
    ~DkAmpAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DkAmpAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DkAmpAudioProcessorEditor)

    RotaryKnob gainKnob{ "Gain", audioProcessor.apvts, gainParamID, 120 };
    RotaryKnob outputKnob{ "Output", audioProcessor.apvts, outputParamID, 120, true };
    RotaryKnob eqLowKnob{ "Low", audioProcessor.apvts, eqLowParamID, 70, true };
    RotaryKnob eqMidKnob{ "Mid", audioProcessor.apvts, eqMidParamID, 70, true };
    RotaryKnob eqHighKnob{ "High", audioProcessor.apvts, eqHighParamID, 70, true };

    juce::ImageButton bypassButton;

    juce::AudioProcessorValueTreeState::ButtonAttachment bypassAttachment{
        audioProcessor.apvts, bypassParamID.getParamID(), bypassButton
    };

    juce::TextButton loadButton;
    juce::TextButton previousButton;
    juce::TextButton nextButton;
    juce::TextButton cabEnableButton;

    juce::AudioProcessorValueTreeState::ButtonAttachment cabEnableAttachment{
        audioProcessor.apvts, cabEnableParamID.getParamID(), cabEnableButton
    };

    juce::ComboBox fileComboBox;
    std::unique_ptr<juce::FileChooser> chooser;
    
    juce::GroupComponent eqGroup, cabGroup;

    void loadIRFile();
    void restoreIRFile();
    void comboBoxChange();
    void nextIR();
    void prevIR();

    MainLookAndFeel mainLF;
};
