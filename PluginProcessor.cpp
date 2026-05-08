#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ImageSynthProcessor::ImageSynthProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ImageSynthProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "gain", "Gain", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "tau", "Tau", juce::NormalisableRange<float>(4.0f, 120.0f, 1.0f), 18.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "zscale", "Z Scale", juce::NormalisableRange<float>(0.2f, 5.0f, 0.01f), 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "chaos", "Chaos", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f));

    return layout;
}

void ImageSynthProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    
    // Prime the ADSR envelope and reset global time
    envelope.setSampleRate(sampleRate);
    juce::ADSR::Parameters params;
    params.attack = 0.1f;   // 100ms fade in
    params.decay = 0.1f;
    params.sustain = 0.8f;
    params.release = 1.5f;  // 1.5 second fade out
    envelope.setParameters(params);
    
    globalSampleCount = 0;  // Reset time
}

void ImageSynthProcessor::releaseResources()
{
}

void ImageSynthProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const float gain   = apvts.getRawParameterValue("gain")->load();
    const float tau    = apvts.getRawParameterValue("tau")->load();
    const float zscale = apvts.getRawParameterValue("zscale")->load();
    const float chaos  = apvts.getRawParameterValue("chaos")->load();

    // --- MIDI PARSING ---
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        if (message.isNoteOn())
        {
            // Set the frequency based on the key pressed using the static function
            currentMidiFrequency = (float)juce::MidiMessage::getMidiNoteInHertz(message.getNoteNumber());
            envelope.noteOn();
        }
        else if (message.isNoteOff())
        {
            envelope.noteOff();
        }
    }

    if (!currentImage.isValid())
        return;

    auto* left  = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : left;

    const int w = currentImage.getWidth();
    const int h = currentImage.getHeight();
    const double sampleRate = getSampleRate();

    // --- AUDIO RENDERING WITH GLOBAL TIME & ENVELOPE ---
    for (int s = 0; s < buffer.getNumSamples(); ++s)
    {
        // Get the current volume from the ADSR envelope
        float envValue = envelope.getNextSample();
        float sample = 0.0f;

        // Only do the heavy math if the synth is actually making sound
        if (envValue > 0.0001f) 
        {
            for (int i = 0; i < 24; ++i)
            {
                // USE GLOBAL TIME for continuous movement, scaled down
                float t = (float)globalSampleCount * 0.0001f; 

                // Lissa-jous style image scanning
                int x = (int)(std::sin(t * 120.0f + i) * w * 0.35f + w / 2) % w;
                int y = (int)(std::cos(t * 90.0f + i * tau * 0.015f) * h * 0.35f + h / 2) % h;

                // Safety bounds
                if (x < 0) x = 0; if (x >= w) x = w - 1;
                if (y < 0) y = 0; if (y >= h) y = h - 1;

                juce::Colour c = currentImage.getPixelAt(x, y);
                float brightness = c.getBrightness();

                // Pitch = Base MIDI note + Harmonics shifted by Image Brightness
                float freq = currentMidiFrequency * (1.0f + (i * 0.5f)) + brightness * zscale * 100.0f;
                
                // Proper phase accumulation based on actual sample rate
                float phase = (float)globalSampleCount * freq * (1.0f / (float)sampleRate) * 2.0f * juce::MathConstants<float>::pi;
                
                // Add chaos modulation
                phase *= (1.0f + chaos * i * 0.05f);

                sample += brightness * std::sin(phase) * 0.035f;
            }
        }

        // Apply Master Gain AND the MIDI Envelope volume
        sample *= gain * envValue;
        
        left[s] = sample;
        if (right) right[s] = sample;

        // Advance global time
        globalSampleCount++;
    }
}

void ImageSynthProcessor::loadImage(const juce::File& file)
{
    currentImage = juce::ImageFileFormat::loadFrom(file);
    if (currentImage.isValid())
    {
        DBG("Image loaded: " << currentImage.getWidth() << "x" << currentImage.getHeight());
    }
}

//==============================================================================
// State Management
//==============================================================================

void ImageSynthProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ImageSynthProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* ImageSynthProcessor::createEditor()
{
    return new ImageSynthEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ImageSynthProcessor();
}