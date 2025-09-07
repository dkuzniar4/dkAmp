#include <JuceHeader.h>
#include "RotaryKnob.h"
#include "LookAndFeel.h"

RotaryKnob::RotaryKnob(const juce::String& text,
    juce::AudioProcessorValueTreeState& apvts,
    const juce::ParameterID& parameterID,
    int size,
    bool drawFromMiddle)
    : attachment(apvts, parameterID.getParamID(), slider)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, size, 16);
    slider.setBounds(0, 0, size, size + 16);
    addAndMakeVisible(slider);

    label.setText(text, juce::NotificationType::dontSendNotification);
    label.setJustificationType(juce::Justification::horizontallyCentred);
    label.setBorderSize(juce::BorderSize<int>{ 0, 0, 2, 0 });
    label.attachToComponent(&slider, false);
    addAndMakeVisible(label);

    setSize(size, size + 40);

    setLookAndFeel(RotaryKnobLookAndFeel::get());

    float pi = juce::MathConstants<float>::pi;
    slider.setRotaryParameters(1.25f * pi, 2.75f * pi, true);

    slider.getProperties().set("drawFromMiddle", drawFromMiddle);
}

RotaryKnob::~RotaryKnob()
{
}

void RotaryKnob::resized()
{
    slider.setTopLeftPosition(0, 24);
}
