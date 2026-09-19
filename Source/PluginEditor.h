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
    std::array<juce::Slider,12> knobs; std::array<juce::Label,12> labels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    const std::array<const char*,12> ids{"attack","decay","sustain","release","tone","drift","echo","space","master","oscMix","detune","drive"};
    const std::array<const char*,12> names{"ATTACK","DECAY","SUSTAIN","RELEASE","TONE","DRIFT","ECHO","SPACE","MASTER","A/B MIX","DETUNE","DRIVE"};
    juce::MidiKeyboardComponent keyboard;
    juce::ComboBox presetBox, waveABox, waveBBox; juce::TextButton previous{"<"},next{">"}; std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveAAttachment,waveBAttachment;
    float animation=0.f; void timerCallback() override;
    void drawPanel(juce::Graphics&,juce::Rectangle<float>,const juce::String&,juce::Colour); void drawWave(juce::Graphics&,juce::Rectangle<float>,juce::Colour,float);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AstrophiluxNebulaAudioProcessorEditor)
};