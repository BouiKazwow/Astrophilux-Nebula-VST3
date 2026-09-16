#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AstrophiluxNebulaAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AstrophiluxNebulaAudioProcessorEditor (AstrophiluxNebulaAudioProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    AstrophiluxNebulaAudioProcessor& processor;
    juce::Image hero;
    juce::LookAndFeel_V4 look;
    std::array<juce::Slider,8> knobs;
    std::array<juce::Label,8> labels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    const std::array<const char*,8> ids {"attack","decay","sustain","release","tone","drift","echo","space"};
    const std::array<const char*,8> names {"ATTACK","DECAY","SUSTAIN","RELEASE","TONE","DRIFT","ECHO","SPACE"};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AstrophiluxNebulaAudioProcessorEditor)
};
