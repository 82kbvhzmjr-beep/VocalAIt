#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class VocalAIStudioEditor final : public juce::AudioProcessorEditor
{
public:
    explicit VocalAIStudioEditor (VocalAIStudioProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    VocalAIStudioProcessor& processor;
    juce::Label title, chainLabel, statusLabel, promptLabel;
    juce::ComboBox presetBox, sectionBox;
    juce::TextButton analyseButton { "Analizar voz" }, talkButton { "TALK" }, compareButton { "A/B" }, advancedButton { "Ajustes avanzados" };
    juce::TextEditor prompt;
    juce::Slider intensity, bright, comp, reverb;
    juce::Label intensityLabel, brightLabel, compLabel, reverbLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment, brightAttachment, compAttachment, reverbAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalAIStudioEditor)
};
