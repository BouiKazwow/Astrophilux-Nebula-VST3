#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace IDs { constexpr auto attack="attack", decay="decay", sustain="sustain", release="release", tone="tone", drift="drift", echo="echo", space="space"; }

AstrophiluxNebulaAudioProcessor::AstrophiluxNebulaAudioProcessor()
  : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
    state (*this, nullptr, "NEBULA_STATE", createLayout())
{
    for (int i = 0; i < 16; ++i) synth.addVoice (new juce::SamplerVoice());
    juce::WavAudioFormat wav;
    auto* stream = new juce::MemoryInputStream (BinaryData::Nebula_Source_C3_wav, BinaryData::Nebula_Source_C3_wavSize, false);
    if (auto reader = std::unique_ptr<juce::AudioFormatReader> (wav.createReaderFor (stream, true)))
    {
        juce::BigInteger range; range.setRange (24, 61, true);
        synth.addSound (new juce::SamplerSound ("Infinity", *reader, range, 48, 0.02, 1.8, 8.0));
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout AstrophiluxNebulaAudioProcessor::createLayout()
{
    using P = juce::AudioParameterFloat;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back (std::make_unique<P> (IDs::attack, "Attack", juce::NormalisableRange<float>(0.001f,4.f,0.001f,0.35f),0.02f));
    p.push_back (std::make_unique<P> (IDs::decay, "Decay", juce::NormalisableRange<float>(0.01f,8.f,0.001f,0.35f),1.1f));
    p.push_back (std::make_unique<P> (IDs::sustain, "Sustain", 0.f,1.f,0.82f));
    p.push_back (std::make_unique<P> (IDs::release, "Release", juce::NormalisableRange<float>(0.02f,12.f,0.001f,0.35f),1.8f));
    p.push_back (std::make_unique<P> (IDs::tone, "Tone", juce::NormalisableRange<float>(80.f,20000.f,1.f,0.25f),9000.f));
    p.push_back (std::make_unique<P> (IDs::drift, "Drift", 0.f,1.f,0.18f));
    p.push_back (std::make_unique<P> (IDs::echo, "Echo", 0.f,1.f,0.12f));
    p.push_back (std::make_unique<P> (IDs::space, "Space", 0.f,1.f,0.24f));
    return { p.begin(), p.end() };
}

bool AstrophiluxNebulaAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{ return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }

void AstrophiluxNebulaAudioProcessor::prepareToPlay (double sr, int block)
{
    sampleRate = sr; synth.setCurrentPlaybackSampleRate (sr);
    juce::dsp::ProcessSpec spec { sr, (juce::uint32) block, 2 };
    filter.prepare (spec); filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    chorus.prepare (spec); reverb.prepare (spec); delay.prepare (spec);
    chorus.setRate (0.18f); chorus.setDepth (0.22f); chorus.setCentreDelay (7.f); chorus.setFeedback (0.08f);
    updateEnvelope();
}

void AstrophiluxNebulaAudioProcessor::updateEnvelope()
{
    juce::ADSR::Parameters env;
    env.attack=*state.getRawParameterValue(IDs::attack); env.decay=*state.getRawParameterValue(IDs::decay);
    env.sustain=*state.getRawParameterValue(IDs::sustain); env.release=*state.getRawParameterValue(IDs::release);
    for (int i=0; i<synth.getNumVoices(); ++i)
        if (auto* v=dynamic_cast<juce::SamplerVoice*>(synth.getVoice(i))) v->setEnvelopeParameters(env);
}

void AstrophiluxNebulaAudioProcessor::processBlock (juce::AudioBuffer<float>& b, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals; b.clear(); updateEnvelope(); synth.renderNextBlock (b,midi,0,b.getNumSamples());
    juce::dsp::AudioBlock<float> block (b); juce::dsp::ProcessContextReplacing<float> ctx (block);
    filter.setCutoffFrequency (*state.getRawParameterValue(IDs::tone)); filter.process(ctx);
    chorus.setMix (*state.getRawParameterValue(IDs::drift)); chorus.process(ctx);
    const float echo=*state.getRawParameterValue(IDs::echo), delaySamples=(float)(sampleRate*0.375);
    for (int n=0;n<b.getNumSamples();++n) for(int ch=0;ch<2;++ch) {
        const float dry=b.getSample(ch,n), wet=delay.popSample(ch,delaySamples);
        delay.pushSample(ch,dry+wet*0.28f); b.setSample(ch,n,dry+wet*echo);
    }
    juce::dsp::Reverb::Parameters rp; rp.roomSize=0.72f; rp.damping=0.58f; rp.wetLevel=*state.getRawParameterValue(IDs::space); rp.dryLevel=1.f-rp.wetLevel*0.35f;
    reverb.setParameters(rp); reverb.process(ctx);
}

void AstrophiluxNebulaAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{ if (auto xml=state.copyState().createXml()) copyXmlToBinary(*xml,dest); }
void AstrophiluxNebulaAudioProcessor::setStateInformation (const void* d,int s)
{ if(auto xml=getXmlFromBinary(d,s)) if(xml->hasTagName(state.state.getType())) state.replaceState(juce::ValueTree::fromXml(*xml)); }
juce::AudioProcessorEditor* AstrophiluxNebulaAudioProcessor::createEditor() { return new AstrophiluxNebulaAudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new AstrophiluxNebulaAudioProcessor(); }
