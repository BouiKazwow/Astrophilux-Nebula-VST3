#pragma once
#include <JuceHeader.h>

class NebulaSamplerSound final : public juce::SamplerSound
{
public:
    using juce::SamplerSound::SamplerSound;
};

class NebulaSamplerVoice final : public juce::SamplerVoice
{
public:
    void setEnvelopeParameters(const juce::ADSR::Parameters& p) { envelopeParameters = p; envelope.setParameters(p); }
    void setCurrentPlaybackSampleRate(double newRate) override
    {
        juce::SamplerVoice::setCurrentPlaybackSampleRate(newRate);
        envelope.setSampleRate(newRate);
    }

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int pitchWheel) override
    {
        juce::SamplerVoice::startNote(midiNoteNumber, velocity, sound, pitchWheel);
        envelope.reset();
        envelope.setParameters(envelopeParameters);
        envelope.noteOn();
    }

    void stopNote(float velocity, bool allowTailOff) override
    {
        if (allowTailOff) envelope.noteOff();
        else { envelope.reset(); juce::SamplerVoice::stopNote(velocity, false); }
    }

    void renderNextBlock(juce::AudioBuffer<float>& output, int startSample, int numSamples) override
    {
        if (! isVoiceActive()) return;
        juce::AudioBuffer<float> temp(output.getNumChannels(), numSamples);
        temp.clear();
        juce::SamplerVoice::renderNextBlock(temp, 0, numSamples);
        envelope.applyEnvelopeToBuffer(temp, 0, numSamples);
        for (int ch=0; ch<output.getNumChannels(); ++ch)
            output.addFrom(ch, startSample, temp, juce::jmin(ch, temp.getNumChannels()-1), 0, numSamples);
        if (! envelope.isActive()) clearCurrentNote();
    }
private:
    juce::ADSR envelope;
    juce::ADSR::Parameters envelopeParameters { 0.02f, 1.1f, 0.82f, 1.8f };
};

class AstrophiluxNebulaAudioProcessor final : public juce::AudioProcessor
{
public:
    AstrophiluxNebulaAudioProcessor();
    ~AstrophiluxNebulaAudioProcessor() override = default;
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 12.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Synth Pad 3"; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState state;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
private:
    juce::Synthesiser synth;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::dsp::Chorus<float> chorus;
    juce::dsp::Reverb reverb;
    juce::dsp::DelayLine<float> delay { 96000 };
    double sampleRate = 44100.0;
    void updateEnvelope();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AstrophiluxNebulaAudioProcessor)
};
