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
    void setEnvelope(const juce::ADSR::Parameters& p){envParams=p;env.setParameters(p);}
    void setTimbre(int a,int b,float mx,float dt,float drv,float motion){waveA=a;waveB=b;mix=mx;detune=dt;drive=drv;movement=motion;}
    void setPadSynthMode(bool enabled){padSynthMode=enabled;}
    void startNote(int note,float velocity,juce::SynthesiserSound*,int) override
    {
        level=velocity;baseHz=(float)juce::MidiMessage::getMidiNoteInHertz(note);phaseA=phaseB=driftPhase=padLfoPhase=0.f;
        env.setSampleRate(getSampleRate());
        if(padSynthMode){juce::ADSR::Parameters z;z.attack=.008f;z.decay=.75f;z.sustain=1.f;z.release=4.8f;env.setParameters(z);buildPadTables(note);}
        else env.setParameters(envParams);
        env.reset();env.noteOn();
    }
    void stopNote(float,bool tail) override {if(tail)env.noteOff();else{env.reset();clearCurrentNote();}}
    void pitchWheelMoved(int) override{} void controllerMoved(int,int) override{}
    void renderNextBlock(juce::AudioBuffer<float>& out,int start,int num) override
    {
        if(!isVoiceActive())return;const double sr=getSampleRate();
        for(int i=0;i<num;++i){
            const float e=env.getNextSample();
            if(padSynthMode){
                padLfoPhase+=0.621181f/(float)sr;if(padLfoPhase>=1.f)padLfoPhase-=1.f;
                const float vibrato=std::sin(padLfoPhase*juce::MathConstants<float>::twoPi)*0.0018f;
                const float step=(baseHz/440.f)*(1.f+vibrato);
                auto read=[&](const std::vector<float>& t,float p){int i0=(int)p&(padTableSize-1),i1=(i0+1)&(padTableSize-1);float f=p-std::floor(p);return t[(size_t)i0]+(t[(size_t)i1]-t[(size_t)i0])*f;};
                float l=read(padTableL,padReadPos),r=read(padTableR,padReadPos);padReadPos+=step;while(padReadPos>=padTableSize)padReadPos-=padTableSize;
                if(out.getNumChannels()>0)out.addSample(0,start+i,l*level*e*.82f);
                if(out.getNumChannels()>1)out.addSample(1,start+i,r*level*e*.82f);
            }else{
                driftPhase+=.17f/(float)sr;if(driftPhase>=1)driftPhase-=1;float wob=std::sin(driftPhase*juce::MathConstants<float>::twoPi)*movement*.004f;
                float hzA=baseHz*(1.f+wob),hzB=baseHz*std::pow(2.f,detune/1200.f)*(1.f-wob*.7f);phaseA+=hzA/(float)sr;phaseB+=hzB/(float)sr;phaseA-=std::floor(phaseA);phaseB-=std::floor(phaseB);
                float a=wave(phaseA,waveA),b=wave(phaseB,waveB),v=(a*(1.f-mix)+b*mix)*level;v=std::tanh(v*(1.f+drive*5.f))*e*.42f;
                for(int ch=0;ch<out.getNumChannels();++ch)out.addSample(ch,start+i,v*(ch==0?1.f:.995f));
            }
        }
        if(!env.isActive())clearCurrentNote();
    }
private:
    static constexpr int padOrder=16,padTableSize=1<<padOrder;
    static float wave(float p,int w){switch(w){case 1:return p<.5f?1.f:-1.f;case 2:return 1.f-4.f*std::abs(p-.5f);case 3:return std::sin(p*juce::MathConstants<float>::twoPi);default:return 2.f*p-1.f;}}
    void buildPadTables(int note)
    {
        padTableL.assign(padTableSize,0.f);padTableR.assign(padTableSize,0.f);
        std::vector<float> specL((size_t)padTableSize*2,0.f),specR((size_t)padTableSize*2,0.f);
        const float fundamental=440.f*std::pow(2.f,(note-69)/12.f),binHz=(float)getSampleRate()/padTableSize;
        juce::Random rng(0x535033+note*97);
        for(int h=1;h<=128;++h){
            const float hf=fundamental*h;if(hf>getSampleRate()*.47f)break;
            // Synth Pad 3 .xiz: bandwidth=625, width=127, base_function=4/base_par=51, rand=64.
            const float harmonicAmp=std::exp(-.050f*(h-1))*std::pow(.5f+.5f*std::cos((h-1)*.115f),.35f);
            const float bwHz=juce::jmax(1.5f,hf*(.018f+.045f*(625.f/1000.f)));
            const int centre=(int)std::round(hf/binHz),radius=juce::jlimit(2,220,(int)std::ceil(bwHz/binHz*3.f));
            for(int k=-radius;k<=radius;++k){int bin=centre+k;if(bin<=0||bin>=padTableSize/2)continue;float x=(k*binHz)/bwHz,profile=std::exp(-x*x*2.2f),mag=harmonicAmp*profile;
                float phL=rng.nextFloat()*juce::MathConstants<float>::twoPi,phR=rng.nextFloat()*juce::MathConstants<float>::twoPi;
                specL[(size_t)2*bin]+=mag*std::cos(phL);specL[(size_t)2*bin+1]+=mag*std::sin(phL);specR[(size_t)2*bin]+=mag*std::cos(phR);specR[(size_t)2*bin+1]+=mag*std::sin(phR);}
        }
        juce::dsp::FFT fft(padOrder);fft.perform(specL.data(),specL.data(),true);fft.perform(specR.data(),specR.data(),true);
        float peak=.0001f;for(int i=0;i<padTableSize;++i){padTableL[(size_t)i]=specL[(size_t)2*i];padTableR[(size_t)i]=specR[(size_t)2*i];peak=juce::jmax(peak,std::abs(padTableL[(size_t)i]),std::abs(padTableR[(size_t)i]));}
        const float g=.78f/peak;for(int i=0;i<padTableSize;++i){padTableL[(size_t)i]*=g;padTableR[(size_t)i]*=g;}padReadPos=0.f;
    }
    juce::ADSR env;juce::ADSR::Parameters envParams{.02f,1.1f,.82f,1.8f};
    std::vector<float> padTableL,padTableR;float padReadPos=0,padLfoPhase=0;bool padSynthMode=false;
    float phaseA=0,phaseB=0,driftPhase=0,baseHz=440,level=0,mix=.45f,detune=7,drive=.1f,movement=.1f;int waveA=0,waveB=2;
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