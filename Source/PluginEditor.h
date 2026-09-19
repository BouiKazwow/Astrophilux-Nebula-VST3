#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class NebulaLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
};

class AstrophiluxNebulaAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AstrophiluxNebulaAudioProcessorEditor(AstrophiluxNebulaAudioProcessor&);
    ~AstrophiluxNebulaAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    AstrophiluxNebulaAudioProcessor& processor;
    juce::Image hero;
    NebulaLookAndFeel look;
    std::array<juce::Slider,8> knobs;
    std::array<juce::Label,8> labels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    const std::array<const char*,8> ids {"attack","decay","sustain","release","tone","drift","echo","space"};
    const std::array<const char*,8> names {"ATTACK","DECAY","SUSTAIN","RELEASE","TONE","DRIFT","ECHO","SPACE"};
    void drawPanel(juce::Graphics&, juce::Rectangle<float>, const juce::String&, juce::Colour);
    void drawWave(juce::Graphics&, juce::Rectangle<float>, juce::Colour, float);
    void drawKeyboard(juce::Graphics&, juce::Rectangle<int>);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AstrophiluxNebulaAudioProcessorEditor)
};