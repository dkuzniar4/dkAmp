/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DkAmpAudioProcessorEditor::DkAmpAudioProcessorEditor (DkAmpAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    eqGroup.setText("EQ");
    eqGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    eqGroup.addAndMakeVisible(eqLowKnob);
    eqGroup.addChildComponent(eqLowKnob);
    eqGroup.addAndMakeVisible(eqMidKnob);
    eqGroup.addChildComponent(eqMidKnob);
    eqGroup.addAndMakeVisible(eqHighKnob);
    eqGroup.addChildComponent(eqHighKnob);
    addAndMakeVisible(eqGroup);

    cabGroup.setText("Cab");
    cabGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    loadButton.setButtonText("Load IR");
    loadButton.onClick = [this]() {
        loadIRFile();
        };
    loadButton.setLookAndFeel(ButtonLookAndFeel::get());
    previousButton.setButtonText("Prev");
    previousButton.onClick = [this]() {
        prevIR();
        };
    previousButton.setLookAndFeel(ButtonLookAndFeel::get());
    nextButton.setButtonText("Next");
    nextButton.onClick = [this]() {
        nextIR();
        };
    nextButton.setLookAndFeel(ButtonLookAndFeel::get());

    fileComboBox.onChange = [this]() {
        comboBoxChange();
        };
    fileComboBox.setLookAndFeel(ComboBoxLookAndFeel::get());

    cabGroup.addAndMakeVisible(loadButton);
    cabGroup.addChildComponent(loadButton);
    cabGroup.addAndMakeVisible(previousButton);
    cabGroup.addChildComponent(previousButton);
    cabGroup.addAndMakeVisible(nextButton);
    cabGroup.addChildComponent(nextButton);
    cabGroup.addAndMakeVisible(fileComboBox);
    cabGroup.addChildComponent(fileComboBox);
    addAndMakeVisible(cabGroup);

    addAndMakeVisible(gainKnob);

    addAndMakeVisible(outputKnob);

    auto bypassIcon = juce::ImageCache::getFromMemory(BinaryData::Bypass_png,
        BinaryData::Bypass_pngSize);
    bypassButton.setClickingTogglesState(true);
    bypassButton.setBounds(0, 0, 20, 20);
    bypassButton.setImages(
        false, true, true,
        bypassIcon, 1.0f, juce::Colours::white,
        bypassIcon, 1.0f, juce::Colours::white,
        bypassIcon, 1.0f, juce::Colours::grey,
        0.0f);
    addAndMakeVisible(bypassButton);

    restoreIRFile();

    setSize (700, 400);

    setLookAndFeel(&mainLF);
}

DkAmpAudioProcessorEditor::~DkAmpAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void DkAmpAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black);
    
    auto image = juce::ImageCache::getFromMemory(BinaryData::dkAmp_png, BinaryData::dkAmp_pngSize);

    int destWidth = image.getWidth();
    int destHeight = image.getHeight();
    g.drawImage(image,
        (getWidth() / 2) - (destWidth / 2), 10, destWidth, destHeight,
        0, 0, image.getWidth(), image.getHeight());

    image = juce::ImageCache::getFromMemory(BinaryData::dkLogo_png, BinaryData::dkLogo_pngSize);
    g.drawImage(image,
        getWidth() - image.getWidth(), getHeight() - image.getHeight(), image.getWidth(), image.getHeight(),
        0, 0, image.getWidth(), image.getHeight());
}

void DkAmpAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto width = bounds.getWidth();
    auto height = bounds.getHeight();

    auto eqWidth = width * 0.4f;
    auto eqHeight = height * 0.35f;
    auto bigKnobPx = 120;
    auto smallKnobPx = 70;
    auto margin = 0.15f * height;

    eqGroup.setBounds( (width / 2) - (eqWidth / 2), margin, eqWidth, eqHeight);

    eqLowKnob.setTopLeftPosition( (eqWidth * 0.20) - (smallKnobPx / 2), 15);
    eqMidKnob.setTopLeftPosition((eqWidth * 0.5) - (smallKnobPx / 2), 15);
    eqHighKnob.setTopLeftPosition((eqWidth * 0.8) - (smallKnobPx / 2), 15);

    gainKnob.setTopLeftPosition( (0.15 * width) - (bigKnobPx / 2), margin);
    outputKnob.setTopLeftPosition((0.85 * width) - (bigKnobPx / 2), margin);

    bypassButton.setTopLeftPosition(bounds.getRight() - bypassButton.getWidth() - 10, 10);

    cabGroup.setBounds((width / 2) - (eqWidth / 2), height - eqHeight - margin, eqWidth, eqHeight);
    
    auto buttonWidth = 70;
    auto buttonHeight = 25;

    loadButton.setBounds((eqWidth * 0.20) - (buttonWidth / 2), 25, buttonWidth, buttonHeight);
    previousButton.setBounds((eqWidth * 0.5) - (buttonWidth / 2), 25, buttonWidth, buttonHeight);
    nextButton.setBounds((eqWidth * 0.8) - (buttonWidth / 2), 25, buttonWidth, buttonHeight);

    auto comboBoxWidth = eqWidth - 2 * ((eqWidth * 0.20) - (0.5 * buttonWidth));

    fileComboBox.setBounds((eqWidth * 0.20) - (buttonWidth / 2), buttonHeight + 25 + 10, comboBoxWidth, buttonHeight);
}

void DkAmpAudioProcessorEditor::loadIRFile()
{
    auto startFolder = audioProcessor.apvts.state.getProperty("IR_folder").toString();
    juce::File startLocation = startFolder.isNotEmpty()
        ? juce::File(startFolder)
        : juce::File::getSpecialLocation(juce::File::userHomeDirectory);

    chooser = std::make_unique<juce::FileChooser>("Select IR file or folder...",
        startLocation,
        "*.wav");

    auto flags = juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles
        | juce::FileBrowserComponent::canSelectDirectories;

    chooser->launchAsync(flags, [this](const juce::FileChooser& fc)
        {
            auto chosen = fc.getResult();
            if (!chosen.exists())
                return;

            if (chosen.existsAsFile())
            {
                audioProcessor.cabSim[0].loadIR(chosen);
                audioProcessor.cabSim[1].loadIR(chosen);
            }

            juce::File folder;
            if (chosen.isDirectory())
                folder = chosen;
            else
                folder = chosen.getParentDirectory();

            // znajdü wszystkie pliki wav w folderze
            juce::Array<juce::File> wavFiles;
            folder.findChildFiles(wavFiles, juce::File::findFiles, false, "*.wav");

            if (wavFiles.isEmpty())
                return;

            fileComboBox.clear();

            int selectedIndex = 0;
            for (int i = 0; i < wavFiles.size(); ++i)
            {
                const auto& f = wavFiles.getReference(i);
                fileComboBox.addItem(f.getFileName(), i + 1);

                if (f == chosen)
                    selectedIndex = i;
            }

            fileComboBox.setSelectedId(selectedIndex + 1, juce::dontSendNotification);

            audioProcessor.apvts.state.setProperty("IR_file", wavFiles[selectedIndex].getFullPathName(), nullptr);
            audioProcessor.apvts.state.setProperty("IR_folder", folder.getFullPathName(), nullptr);
        });
}

void DkAmpAudioProcessorEditor::restoreIRFile()
{
    auto folderPath = audioProcessor.apvts.state.getProperty("IR_folder").toString();
    auto filePath = audioProcessor.apvts.state.getProperty("IR_file").toString();

    if (folderPath.isNotEmpty() && filePath.isNotEmpty())
    {
        juce::File folder(folderPath);
        juce::File file(filePath);

        if (folder.exists() && file.existsAsFile())
        {
            // WyczyúÊ i wype≥nij combobox plikami z folderu
            fileComboBox.clear();

            juce::Array<juce::File> wavFiles = folder.findChildFiles(
                juce::File::findFiles, false, "*.wav");

            for (auto& f : wavFiles)
                fileComboBox.addItem(f.getFileName(), fileComboBox.getNumItems() + 1);

            // ustaw zaznaczenie na zapamiÍtanym pliku
            fileComboBox.setText(file.getFileName(), juce::dontSendNotification);

            // Za≥aduj do cabSim
            audioProcessor.cabSim[0].loadIR(file);
            audioProcessor.cabSim[1].loadIR(file);
        }
    }
}

void DkAmpAudioProcessorEditor::comboBoxChange()
{
    auto selectedText = fileComboBox.getText();
    if (selectedText.isNotEmpty())
    {
        auto folderPath = audioProcessor.apvts.state.getProperty("IR_folder").toString();
        if (folderPath.isNotEmpty())
        {
            juce::File folder(folderPath);
            juce::File selectedFile = folder.getChildFile(selectedText);

            if (selectedFile.existsAsFile())
            {
                audioProcessor.cabSim[0].loadIR(selectedFile);
                audioProcessor.cabSim[1].loadIR(selectedFile);
                audioProcessor.apvts.state.setProperty("IR_file", selectedFile.getFullPathName(), nullptr);
            }
        }
    }
}

void DkAmpAudioProcessorEditor::nextIR()
{
    int currentIndex = fileComboBox.getSelectedItemIndex();

    if (currentIndex >= 0 && currentIndex < fileComboBox.getNumItems() - 1)
    {
        fileComboBox.setSelectedItemIndex(currentIndex + 1, juce::dontSendNotification);

        auto folderPath = audioProcessor.apvts.state.getProperty("IR_folder").toString();
        auto selectedText = fileComboBox.getText();

        if (folderPath.isNotEmpty() && selectedText.isNotEmpty())
        {
            juce::File folder(folderPath);
            juce::File selectedFile = folder.getChildFile(selectedText);

            if (selectedFile.existsAsFile())
            {
                audioProcessor.cabSim[0].loadIR(selectedFile);
                audioProcessor.cabSim[1].loadIR(selectedFile);
                audioProcessor.apvts.state.setProperty("IR_file", selectedFile.getFullPathName(), nullptr);
            }
        }
    }
}

void DkAmpAudioProcessorEditor::prevIR()
{
    int currentIndex = fileComboBox.getSelectedItemIndex();

    if (currentIndex > 0)
    {
        fileComboBox.setSelectedItemIndex(currentIndex - 1, juce::dontSendNotification);

        auto folderPath = audioProcessor.apvts.state.getProperty("IR_folder").toString();
        auto selectedText = fileComboBox.getText();

        if (folderPath.isNotEmpty() && selectedText.isNotEmpty())
        {
            juce::File folder(folderPath);
            juce::File selectedFile = folder.getChildFile(selectedText);

            if (selectedFile.existsAsFile())
            {
                audioProcessor.cabSim[0].loadIR(selectedFile);
                audioProcessor.cabSim[1].loadIR(selectedFile);
                audioProcessor.apvts.state.setProperty("IR_file", selectedFile.getFullPathName(), nullptr);
            }
        }
    }
}
