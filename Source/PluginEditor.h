#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class NebulaLookAndFeel final:public juce::LookAndFeel_V4
{
public:void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
};

class AstrophiluxNebulaAudioProcessorEditor final:public juce::AudioProcessorEditor,private juce::Timer
{
public:
    explicit AstrophiluxNebulaAudioProcessorEditor(AstrophiluxNebulaAudioProcessor&);
    ~AstrophiluxNebulaAudioProcessorEditor() override;
    void paint(juce::Graphics&) override; void resized() override;
private:
    AstrophiluxNebulaAudioProcessor& processor; juce::Image hero; NebulaLookAndFeel look;
    std::array<juce::Slider,9> knobs; std::array<juce::Label,9> labels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    const std::array<const char*,9> ids{"attack","decay","sustain","release","tone","drift","echo","space","master"};
    const std::array<const char*,9> names{"ATTACK","DECAY","SUSTAIN","RELEASE","TONE","DRIFT","ECHO","SPACE","MASTER"};
    juce::MidiKeyboardComponent keyboard;
    juce::ComboBox presetBox; juce::TextButton previous{"<"},next{">"};
    float animation=0.f; void timerCallback() override;
    void drawPanel(juce::Graphics&,juce::Rectangle<float>,const juce::String&,juce::Colour); void drawWave(juce::Graphics&,juce::Rectangle<float>,juce::Colour,float);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AstrophiluxNebulaAudioProcessorEditor)
};