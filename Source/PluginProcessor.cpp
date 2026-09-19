#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace IDs
{
    constexpr auto attack  = "attack";
    constexpr auto decay   = "decay";
    constexpr auto sustain = "sustain";
    constexpr auto release = "release";
    constexpr auto tone    = "tone";
    constexpr auto drift   = "drift";
    constexpr auto echo    = "echo";
    constexpr auto space   = "space";
}

AstrophiluxNebulaAudioProcessor::AstrophiluxNebulaAudioProcessor()
    : AudioProcessor(
          BusesProperties().withOutput(
              "Output",
              juce::AudioChannelSet::stereo(),
              true
          )
      ),
      state(*this, nullptr, "NEBULA_STATE", createLayout())
{
    for (int i = 0; i < 16; ++i)
        synth.addVoice(new NebulaSamplerVoice());

    juce::WavAudioFormat wav;

    auto* stream = new juce::MemoryInputStream(
        BinaryData::Nebula_Source_C3_wav,
        BinaryData::Nebula_Source_C3_wavSize,
        false
    );

    if (auto reader = std::unique_ptr<juce::AudioFormatReader>(
            wav.createReaderFor(stream, true)))
    {
        juce::BigInteger playableRange;
        playableRange.setRange(24, 61, true);

        synth.addSound(
            new NebulaSamplerSound(
                "Synth Pad 3",
                *reader,
                playableRange,
                48,
                0.02,
                1.8,
                8.0
            )
        );
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout
AstrophiluxNebulaAudioProcessor::createLayout()
{
    using Parameter = juce::AudioParameterFloat;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    parameters.push_back(
        std::make_unique<Parameter>(
            IDs::attack,
            "Attack",
            juce::NormalisableRange<float>(0.001f, 4.0f, 0.001f, 0.35f),
            0.02f
        )
    );

    parameters.push_back(
        std::make_unique<Parameter>(
            IDs::decay,
            "Decay",
            juce::NormalisableRange<float>(0.01f, 8.0f, 0.001f, 0.35f),
            1.1f
        )
    );

    parameters.push_back(
        std::make_unique<Parameter>(
            IDs::sustain,
            "Sustain",
            0.0f,
            1.0f,
            0.82f
        )
    );

    parameters.push_back(
        std::make_unique<Parameter>(
            IDs::release,
            "Release",
            juce::NormalisableRange<float>(0.02f, 12.0f, 0.001f, 0.35f),
            1.8f
        )
    );

    parameters.push_back(
        std::make_unique<Parameter>(
            IDs::tone,
            "Tone",
            juce::NormalisableRange<float>(80.0f, 20000.0f, 1.0f, 0.25f),
            9000.0f
        )
    );

    parameters.push_back(
        std::make_unique<Parameter>(
            IDs::drift,
            "Drift",
            0.0f,
            1.0f,
            0.18f
        )
    );

    parameters.push_back(
        std::make_unique<Parameter>(
            IDs::echo,
            "Echo",
            0.0f,
            1.0f,
            0.12f
        )
    );

    parameters.push_back(
        std::make_unique<Parameter>(
            IDs::space,
            "Space",
            0.0f,
            1.0f,
            0.24f
        )
    );

    return { parameters.begin(), parameters.end() };
}

bool AstrophiluxNebulaAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet()
        == juce::AudioChannelSet::stereo();
}

void AstrophiluxNebulaAudioProcessor::prepareToPlay(
    double newSampleRate,
    int samplesPerBlock)
{
    sampleRate = newSampleRate;
    synth.setCurrentPlaybackSampleRate(newSampleRate);

    juce::dsp::ProcessSpec specification
    {
        newSampleRate,
        static_cast<juce::uint32>(samplesPerBlock),
        2
    };

    filter.prepare(specification);
    filter.setType(
        juce::dsp::StateVariableTPTFilterType::lowpass
    );

    chorus.prepare(specification);
    reverb.prepare(specification);
    delay.prepare(specification);

    chorus.setRate(0.18f);
    chorus.setDepth(0.22f);
    chorus.setCentreDelay(7.0f);
    chorus.setFeedback(0.08f);
}

void AstrophiluxNebulaAudioProcessor::updateEnvelope()
{
    juce::ADSR::Parameters p;
    p.attack  = *state.getRawParameterValue(IDs::attack);
    p.decay   = *state.getRawParameterValue(IDs::decay);
    p.sustain = *state.getRawParameterValue(IDs::sustain);
    p.release = *state.getRawParameterValue(IDs::release);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<NebulaSamplerVoice*>(synth.getVoice(i)))
            voice->setEnvelopeParameters(p);
}

void AstrophiluxNebulaAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();
    updateEnvelope();

    synth.renderNextBlock(
        buffer,
        midiMessages,
        0,
        buffer.getNumSamples()
    );

    juce::dsp::AudioBlock<float> audioBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> context(audioBlock);

    filter.setCutoffFrequency(
        *state.getRawParameterValue(IDs::tone)
    );

    filter.process(context);

    chorus.setMix(
        *state.getRawParameterValue(IDs::drift)
    );

    chorus.process(context);

    const float echoAmount =
        *state.getRawParameterValue(IDs::echo);

    const float delayInSamples =
        static_cast<float>(sampleRate * 0.375);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            const float drySignal =
                buffer.getSample(channel, sample);

            const float delayedSignal =
                delay.popSample(channel, delayInSamples);

            delay.pushSample(
                channel,
                drySignal + delayedSignal * 0.28f
            );

            buffer.setSample(
                channel,
                sample,
                drySignal + delayedSignal * echoAmount
            );
        }
    }

    juce::dsp::Reverb::Parameters reverbParameters;

    reverbParameters.roomSize = 0.72f;
    reverbParameters.damping = 0.58f;
    reverbParameters.wetLevel =
        *state.getRawParameterValue(IDs::space);

    reverbParameters.dryLevel =
        1.0f - reverbParameters.wetLevel * 0.35f;

    reverb.setParameters(reverbParameters);
    reverb.process(context);
}

void AstrophiluxNebulaAudioProcessor::getStateInformation(
    juce::MemoryBlock& destinationData)
{
    if (auto xml = state.copyState().createXml())
        copyXmlToBinary(*xml, destinationData);
}

void AstrophiluxNebulaAudioProcessor::setStateInformation(
    const void* data,
    int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (xml->hasTagName(state.state.getType()))
        {
            state.replaceState(
                juce::ValueTree::fromXml(*xml)
            );
        }
    }
}

juce::AudioProcessorEditor*
AstrophiluxNebulaAudioProcessor::createEditor()
{
    return new AstrophiluxNebulaAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AstrophiluxNebulaAudioProcessor();
}
