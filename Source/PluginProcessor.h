#pragma once
#include <JuceHeader.h>

class NebulaSamplerSound final : public juce::SamplerSound
{
public: using juce::SamplerSound::SamplerSound;
};

class NebulaSamplerVoice final : public juce::SamplerVoice
{
public:
    void setEnvelopeParameters(const juce::ADSR::Parameters& p){ envelopeParameters=p; envelope.setParameters(p); }
    void setCurrentPlaybackSampleRate(double r) override { juce::SamplerVoice::setCurrentPlaybackSampleRate(r); envelope.setSampleRate(r); }
    void startNote(int n,float v,juce::SynthesiserSound* s,int wheel) override { juce::SamplerVoice::startNote(n,v,s,wheel); envelope.reset(); envelope.setParameters(envelopeParameters); envelope.noteOn(); }
    void stopNote(float v,bool tail) override { if(tail) envelope.noteOff(); else { envelope.reset(); juce::SamplerVoice::stopNote(v,false); } }
    void renderNextBlock(juce::AudioBuffer<float>& out,int start,int num) override
    {
        if(!isVoiceActive()) return;
        temp.setSize(out.getNumChannels(),num,false,false,true); temp.clear();
        juce::SamplerVoice::renderNextBlock(temp,0,num); envelope.applyEnvelopeToBuffer(temp,0,num);
        for(int ch=0;ch<out.getNumChannels();++ch) out.addFrom(ch,start,temp,juce::jmin(ch,temp.getNumChannels()-1),0,num);
        if(!envelope.isActive()) clearCurrentNote();
    }
private:
    juce::ADSR envelope; juce::ADSR::Parameters envelopeParameters{0.02f,1.1f,0.82f,1.8f}; juce::AudioBuffer<float> temp;
};

class AstrophiluxNebulaAudioProcessor final : public juce::AudioProcessor
{
public:
    AstrophiluxNebulaAudioProcessor();
    ~AstrophiluxNebulaAudioProcessor() override=default;
    void prepareToPlay(double,int) override; void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; } bool producesMidi() const override { return false; } bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 12.0; }
    int getNumPrograms() override { return 12; } int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram(int i) override { loadPreset(i); } const juce::String getProgramName(int i) override; void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
    juce::AudioProcessorValueTreeState state;
    juce::MidiKeyboardState keyboardState;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void loadPreset(int);
    int getPresetIndex() const { return currentPreset; }
private:
    juce::Synthesiser synth; juce::dsp::StateVariableTPTFilter<float> filter; juce::dsp::Chorus<float> chorus; juce::dsp::Reverb reverb; juce::dsp::DelayLine<float> delay{96000};
    double sampleRate=44100.0; int currentPreset=0; void updateEnvelope();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AstrophiluxNebulaAudioProcessor)
};