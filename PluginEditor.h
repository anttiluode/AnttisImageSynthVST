#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// Image Drop Zone + Preview
//==============================================================================
class ImageDropZone : public juce::Component,
                      public juce::FileDragAndDropTarget
{
public:
    ImageDropZone(ImageSynthProcessor& p);

    void paint(juce::Graphics& g) override;
    void resized() override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    ImageSynthProcessor& processor;
    juce::Image previewImage;
};

//==============================================================================
// Main Editor
//==============================================================================
class ImageSynthEditor : public juce::AudioProcessorEditor,
                         private juce::Timer
{
public:
    explicit ImageSynthEditor(ImageSynthProcessor&);
    ~ImageSynthEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    ImageSynthProcessor& processor;

    ImageDropZone dropZone;

    // Knobs
    juce::Slider gainSlider, tauSlider, zScaleSlider, chaosSlider;

    juce::Label gainLabel, tauLabel, zScaleLabel, chaosLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        gainAtt, tauAtt, zScaleAtt, chaosAtt;

    void setupSlider(juce::Slider& s, juce::Label& l, const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImageSynthEditor)
};