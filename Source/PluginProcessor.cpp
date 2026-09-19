#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace IDs { constexpr auto attack="attack"; constexpr auto decay="decay"; constexpr auto sustain="sustain"; constexpr auto release="release"; constexpr auto tone="tone"; constexpr auto drift="drift"; constexpr auto echo="echo"; constexpr auto space="space"; constexpr auto master="master"; }

AstrophiluxNebulaAudioProcessor::AstrophiluxNebulaAudioProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"NEBULA_STATE",createLayout())
{
    for(int i=0;i<16;++i) synth.addVoice(new NebulaSamplerVoice());
    juce::WavAudioFormat wav;
    auto* stream=new juce::MemoryInputStream(BinaryData::Nebula_Source_C3_wav,BinaryData::Nebula_Source_C3_wavSize,false);
    if(auto reader=std::unique_ptr<juce::AudioFormatReader>(wav.createReaderFor(stream,true))){
        juce::BigInteger range; range.setRange(0,128,true);
        synth.addSound(new NebulaSamplerSound("Astrophilux Source",*reader,range,60,0.02,1.8,8.0));
    }
}
juce::AudioProcessorValueTreeState::ParameterLayout AstrophiluxNebulaAudioProcessor::createLayout()
{
    using P=juce::AudioParameterFloat; std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<P>(IDs::attack,"Attack",juce::NormalisableRange<float>(.001f,4.f,.001f,.35f),.02f));
    p.push_back(std::make_unique<P>(IDs::decay,"Decay",juce::NormalisableRange<float>(.01f,8.f,.001f,.35f),1.1f));
    p.push_back(std::make_unique<P>(IDs::sustain,"Sustain",0.f,1.f,.82f));
    p.push_back(std::make_unique<P>(IDs::release,"Release",juce::NormalisableRange<float>(.02f,12.f,.001f,.35f),1.8f));
    p.push_back(std::make_unique<P>(IDs::tone,"Tone",juce::NormalisableRange<float>(80.f,20000.f,1.f,.25f),9000.f));
    p.push_back(std::make_unique<P>(IDs::drift,"Drift",0.f,1.f,.18f));
    p.push_back(std::make_unique<P>(IDs::echo,"Echo",0.f,1.f,.12f));
    p.push_back(std::make_unique<P>(IDs::space,"Space",0.f,1.f,.24f));
    p.push_back(std::make_unique<P>(IDs::master,"Master",0.f,1.25f,.8f));
    return {p.begin(),p.end()};
}
bool AstrophiluxNebulaAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const { return l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo(); }
void AstrophiluxNebulaAudioProcessor::prepareToPlay(double sr,int block)
{
    sampleRate=sr; synth.setCurrentPlaybackSampleRate(sr); keyboardState.reset();
    juce::dsp::ProcessSpec spec{sr,(juce::uint32)block,2}; filter.prepare(spec); filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass); chorus.prepare(spec); reverb.prepare(spec); delay.prepare(spec);
    chorus.setRate(.18f); chorus.setDepth(.22f); chorus.setCentreDelay(7.f); chorus.setFeedback(.08f);
}
void AstrophiluxNebulaAudioProcessor::updateEnvelope()
{
    juce::ADSR::Parameters p; p.attack=*state.getRawParameterValue(IDs::attack);p.decay=*state.getRawParameterValue(IDs::decay);p.sustain=*state.getRawParameterValue(IDs::sustain);p.release=*state.getRawParameterValue(IDs::release);
    for(int i=0;i<synth.getNumVoices();++i) if(auto* v=dynamic_cast<NebulaSamplerVoice*>(synth.getVoice(i))) v->setEnvelopeParameters(p);
}
void AstrophiluxNebulaAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals n; b.clear(); updateEnvelope(); keyboardState.processNextMidiBuffer(midi,0,b.getNumSamples(),true); synth.renderNextBlock(b,midi,0,b.getNumSamples());
    juce::dsp::AudioBlock<float> block(b); juce::dsp::ProcessContextReplacing<float> ctx(block);
    filter.setCutoffFrequency(*state.getRawParameterValue(IDs::tone)); filter.process(ctx);
    chorus.setMix(*state.getRawParameterValue(IDs::drift)); chorus.process(ctx);
    float ea=*state.getRawParameterValue(IDs::echo), ds=(float)(sampleRate*.375);
    for(int s=0;s<b.getNumSamples();++s) for(int ch=0;ch<b.getNumChannels();++ch){float dry=b.getSample(ch,s),d=delay.popSample(ch,ds);delay.pushSample(ch,dry+d*.28f);b.setSample(ch,s,dry+d*ea);}
    juce::dsp::Reverb::Parameters rp; rp.roomSize=.72f;rp.damping=.58f;rp.wetLevel=*state.getRawParameterValue(IDs::space);rp.dryLevel=1.f-rp.wetLevel*.35f;reverb.setParameters(rp);reverb.process(ctx);
    b.applyGain(*state.getRawParameterValue(IDs::master));
}
const juce::String AstrophiluxNebulaAudioProcessor::getProgramName(int i)
{
    static const char* names[]={"Synth Pad 3","Infinite Haze","Soft Orbit","Dreamglass","Green Aurora","Afterglow","Neon Highway","Midnight Arcade","Chrome Horizon","Cassette Sunset","Night Drive","Digital Hearts"};
    return names[juce::jlimit(0,11,i)];
}
void AstrophiluxNebulaAudioProcessor::loadPreset(int i)
{
    struct V{float a,d,s,r,t,dr,e,sp,m;}; static const V v[]={
        {.02f,1.1f,.82f,1.8f,9000,.18f,.12f,.24f,.80f},
        {.55f,2.4f,.72f,5.8f,4300,.42f,.25f,.62f,.78f},
        {.18f,1.8f,.88f,3.7f,6900,.28f,.08f,.48f,.80f},
        {.03f,.75f,.66f,2.8f,11800,.12f,.31f,.40f,.76f},
        {1.25f,3.2f,.76f,7.4f,3600,.55f,.18f,.70f,.78f},
        {.08f,1.25f,.58f,4.5f,7600,.34f,.36f,.58f,.77f},
        {.015f,.45f,.72f,1.5f,12500,.10f,.22f,.28f,.80f},
        {.005f,.28f,.64f,.75f,9800,.06f,.14f,.18f,.82f},
        {.09f,.90f,.80f,2.9f,6200,.26f,.30f,.52f,.78f},
        {.32f,1.8f,.70f,4.8f,5100,.38f,.18f,.64f,.76f},
        {.012f,.62f,.74f,1.9f,11200,.16f,.34f,.38f,.80f},
        {.06f,1.4f,.86f,3.6f,7400,.30f,.26f,.56f,.78f}};
    i=juce::jlimit(0,11,i); currentPreset=i; const auto& x=v[i];
    const char* ids[]={IDs::attack,IDs::decay,IDs::sustain,IDs::release,IDs::tone,IDs::drift,IDs::echo,IDs::space,IDs::master}; float vals[]={x.a,x.d,x.s,x.r,x.t,x.dr,x.e,x.sp,x.m};
    for(int k=0;k<9;++k) if(auto* p=state.getParameter(ids[k])) p->setValueNotifyingHost(p->convertTo0to1(vals[k]));
}
void AstrophiluxNebulaAudioProcessor::getStateInformation(juce::MemoryBlock& d){if(auto xml=state.copyState().createXml()) copyXmlToBinary(*xml,d);}
void AstrophiluxNebulaAudioProcessor::setStateInformation(const void* d,int n){if(auto xml=getXmlFromBinary(d,n))if(xml->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessorEditor* AstrophiluxNebulaAudioProcessor::createEditor(){return new AstrophiluxNebulaAudioProcessorEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new AstrophiluxNebulaAudioProcessor();}