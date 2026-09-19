#pragma once
#include <JuceHeader.h>

class AstroSound final : public juce::SynthesiserSound
{
public: bool appliesToNote(int) override{return true;} bool appliesToChannel(int) override{return true;}
};

class AstroVoice final : public juce::SynthesiserVoice
{
public:
    bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<AstroSound*>(s)!=nullptr; }
    void setEnvelope(const juce::ADSR::Parameters& p){envParams=p; env.setParameters(p);}
    void setTimbre(int a,int b,float mx,float dt,float drv,float motion){waveA=a;waveB=b;mix=mx;detune=dt;drive=drv;movement=motion;}
    void startNote(int note,float velocity,juce::SynthesiserSound*,int) override { level=velocity; baseHz=(float)juce::MidiMessage::getMidiNoteInHertz(note); phaseA=phaseB=0; driftPhase=0; env.setSampleRate(getSampleRate());env.setParameters(envParams);env.reset();env.noteOn();}
    void stopNote(float,bool tail) override {if(tail)env.noteOff();else{env.reset();clearCurrentNote();}}
    void pitchWheelMoved(int) override{} void controllerMoved(int,int) override{}
    void renderNextBlock(juce::AudioBuffer<float>& out,int start,int num) override
    {
        if(!isVoiceActive())return; const double sr=getSampleRate();
        for(int i=0;i<num;++i){
            driftPhase += 0.17f/(float)sr; if(driftPhase>=1)driftPhase-=1;
            float wob=std::sin(driftPhase*juce::MathConstants<float>::twoPi)*movement*0.004f;
            float hzA=baseHz*(1.f+wob), hzB=baseHz*std::pow(2.f,(detune/1200.f))*(1.f-wob*.7f);
            phaseA += hzA/(float)sr; phaseB += hzB/(float)sr; phaseA-=std::floor(phaseA);phaseB-=std::floor(phaseB);
            float a=wave(phaseA,waveA),b=wave(phaseB,waveB);float s=(a*(1.f-mix)+b*mix)*level;
            s=std::tanh(s*(1.f+drive*5.f))*env.getNextSample()*.42f;
            for(int ch=0;ch<out.getNumChannels();++ch)out.addSample(ch,start+i,s*(ch==0?1.f:.995f));
        }
        if(!env.isActive())clearCurrentNote();
    }
private:
    static float wave(float p,int w){switch(w){case 1:return p<.5f?1.f:-1.f;case 2:return 1.f-4.f*std::abs(p-.5f);case 3:return std::sin(p*juce::MathConstants<float>::twoPi);default:return 2.f*p-1.f;}}
    juce::ADSR env;juce::ADSR::Parameters envParams{.02f,1.1f,.82f,1.8f};float phaseA=0,phaseB=0,driftPhase=0,baseHz=440,level=0,mix=.45f,detune=7,drive=.1f,movement=.1f;int waveA=0,waveB=2;
};

class AstrophiluxNebulaAudioProcessor final : public juce::AudioProcessor
{
public:
    AstrophiluxNebulaAudioProcessor();~AstrophiluxNebulaAudioProcessor() override=default;
    void prepareToPlay(double,int) override;void releaseResources() override{};bool isBusesLayoutSupported(const BusesLayout&) const override;void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;bool hasEditor() const override{return true;}const juce::String getName() const override{return JucePlugin_Name;}
    bool acceptsMidi() const override{return true;}bool producesMidi() const override{return false;}bool isMidiEffect() const override{return false;}double getTailLengthSeconds() const override{return 12.0;}
    int getNumPrograms() override{return 12;}int getCurrentProgram() override{return currentPreset;}void setCurrentProgram(int i) override{loadPreset(i);}const juce::String getProgramName(int i) override;void changeProgramName(int,const juce::String&) override{}
    void getStateInformation(juce::MemoryBlock&) override;void setStateInformation(const void*,int) override;
    juce::AudioProcessorValueTreeState state;juce::MidiKeyboardState keyboardState;static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();void loadPreset(int);int getPresetIndex()const{return currentPreset;}
private:
    juce::Synthesiser synth;juce::dsp::StateVariableTPTFilter<float> filter;juce::dsp::Chorus<float> chorus;juce::dsp::Reverb reverb;juce::dsp::DelayLine<float> delay{96000};
    double sampleRate=44100.0;int currentPreset=0;void updateVoices();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AstrophiluxNebulaAudioProcessor)
};