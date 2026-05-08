#include "PluginEditor.h"

//==============================================================================
// ImageDropZone
//==============================================================================

ImageDropZone::ImageDropZone(ImageSynthProcessor& p) : processor(p)
{
    setSize(380, 380);
}

bool ImageDropZone::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto& f : files)
        if (f.endsWithIgnoreCase(".jpg") ||
            f.endsWithIgnoreCase(".jpeg") ||
            f.endsWithIgnoreCase(".png") ||
            f.endsWithIgnoreCase(".bmp"))
            return true;
    return false;
}

void ImageDropZone::filesDropped(const juce::StringArray& files, int, int)
{
    for (auto& path : files)
    {
        juce::File file(path);
        if (file.existsAsFile())
        {
            processor.loadImage(file);
            previewImage = processor.currentImage;
            repaint();
            return;
        }
    }
}

void ImageDropZone::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(20, 22, 35));

    if (previewImage.isValid())
    {
        auto bounds = getLocalBounds().reduced(8).toFloat();
        g.drawImage(previewImage, bounds, juce::RectanglePlacement::stretchToFit, false);
    }
    else
    {
        g.setColour(juce::Colours::grey.withAlpha(0.4f));
        g.setFont(18.0f);
        g.drawText("Drop Image Here\n(.jpg .png)", getLocalBounds(), juce::Justification::centred);
        
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.drawRect(getLocalBounds().reduced(20), 2);
    }
}

void ImageDropZone::resized() {}

//==============================================================================
// ImageSynthEditor
//==============================================================================

namespace
{
    const juce::Colour BG_COLOR    { 10, 12, 22 };
    const juce::Colour PANEL_COLOR { 25, 28, 45 };
    const juce::Colour ACCENT     { 0, 220, 180 };
}

ImageSynthEditor::ImageSynthEditor(ImageSynthProcessor& p)
    : AudioProcessorEditor(&p), processor(p), dropZone(p)
{
    setSize(920, 620);
    setResizable(true, true);

    addAndMakeVisible(dropZone);

    // Knobs
    setupSlider(gainSlider,   gainLabel,   "Gain");
    setupSlider(tauSlider,    tauLabel,    "Tau");
    setupSlider(zScaleSlider, zScaleLabel, "Z Scale");
    setupSlider(chaosSlider,  chaosLabel,  "Chaos");

    gainAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "gain",   gainSlider);
    tauAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "tau",    tauSlider);
    zScaleAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "zscale", zScaleSlider);
    chaosAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "chaos",  chaosSlider);

    startTimerHz(30);
}

ImageSynthEditor::~ImageSynthEditor()
{
    stopTimer();
}

void ImageSynthEditor::setupSlider(juce::Slider& s, juce::Label& l, const juce::String& name)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    s.setColour(juce::Slider::rotarySliderFillColourId, ACCENT);
    s.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    addAndMakeVisible(s);

    l.setText(name, juce::dontSendNotification);
    l.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    l.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(l);
}

void ImageSynthEditor::paint(juce::Graphics& g)
{
    g.fillAll(BG_COLOR);

    auto title = getLocalBounds().removeFromTop(40);
    g.setColour(PANEL_COLOR);
    g.fillRect(title);

    g.setColour(ACCENT);
    g.setFont(juce::FontOptions(22.0f, juce::Font::bold));  // Fixed deprecated constructor
    g.drawText("Antti Image Synth", title, juce::Justification::centred);
}

void ImageSynthEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(50);

    dropZone.setBounds(area.removeFromLeft(420).reduced(20));

    auto controls = area.reduced(20);
    const int knobW = 110;
    const int spacing = 20;

    int x = controls.getX();
    int y = controls.getY() + 20;

    auto placeKnob = [&](juce::Slider& s, juce::Label& l)
    {
        l.setBounds(x, y, knobW, 20);
        s.setBounds(x, y + 24, knobW, knobW);
        x += knobW + spacing;
    };

    placeKnob(gainSlider, gainLabel);
    placeKnob(tauSlider, tauLabel);
    placeKnob(zScaleSlider, zScaleLabel);
    placeKnob(chaosSlider, chaosLabel);
}

void ImageSynthEditor::timerCallback()
{
    repaint();
}