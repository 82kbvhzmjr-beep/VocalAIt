#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr auto paramMix = "mix";
constexpr auto paramInput = "input";
constexpr auto paramOutput = "output";
constexpr auto paramHP = "highpass";
constexpr auto paramBright = "bright";
constexpr auto paramComp = "comp";
constexpr auto paramDrive = "drive";
constexpr auto paramReverb = "reverb";
constexpr auto paramTune = "tune";
constexpr auto paramDoubler = "doubler";
constexpr auto paramBypass = "bypass";
constexpr auto paramAB = "ab";
}

VocalAIStudioProcessor::VocalAIStudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Entrada", juce::AudioChannelSet::stereo(), true)
                                      .withOutput ("Salida", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETROS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout VocalAIStudioProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { paramMix, 1 }, "Intensidad", NormalisableRange<float> (0.0f, 1.0f), 1.0f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { paramInput, 1 }, "Entrada", NormalisableRange<float> (-18.0f, 18.0f, 0.1f), 0.0f, "dB"));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { paramOutput, 1 }, "Salida", NormalisableRange<float> (-18.0f, 18.0f, 0.1f), 0.0f, "dB"));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { paramHP, 1 }, "Filtro grave", NormalisableRange<float> (20.0f, 350.0f, 1.0f, 0.5f), 75.0f, "Hz"));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { paramBright, 1 }, "Brillo", NormalisableRange<float> (-9.0f, 9.0f, 0.1f), 0.0f, "dB"));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { paramComp, 1 }, "Compresión", NormalisableRange<float> (0.0f, 1.0f), 0.25f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { paramDrive, 1 }, "Saturación", NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { paramReverb, 1 }, "Reverb", NormalisableRange<float> (0.0f, 1.0f), 0.08f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { paramTune, 1 }, "Afinación", NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { paramDoubler, 1 }, "Segunda voz grave", NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { paramBypass, 1 }, "Procesamiento", false));
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { paramAB, 1 }, "Comparar A/B", false));
    return { p.begin(), p.end() };
}

void VocalAIStudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), static_cast<juce::uint32> (getTotalNumOutputChannels()) };
    highPass.prepare (spec);
    lowPass.prepare (spec);
    highPass.reset(); lowPass.reset(); reverb.reset();
    highPass.state = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, 75.0f);
    lowPass.state = juce::dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate, 4200.0f, 0.7f, juce::Decibels::decibelsToGain (0.0f));
    juce::dsp::Reverb::Parameters rp; rp.roomSize = 0.35f; rp.damping = 0.55f; rp.wetLevel = 0.08f; rp.dryLevel = 1.0f; rp.width = 0.7f; reverb.setParameters (rp);
    dryBuffer.setSize (getTotalNumOutputChannels(), samplesPerBlock);
}

void VocalAIStudioProcessor::releaseResources() {}

bool VocalAIStudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo()) && out == layouts.getMainInputChannelSet();
}

void VocalAIStudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = juce::jmin (buffer.getNumChannels(), getTotalNumInputChannels());
    const int samples = buffer.getNumSamples();
    if (dryBuffer.getNumSamples() < samples || dryBuffer.getNumChannels() < channels) dryBuffer.setSize (channels, samples, false, false, true);
    for (int c = 0; c < channels; ++c) dryBuffer.copyFrom (c, 0, buffer, c, 0, samples);

    const auto get = [this] (const char* id) { return parameters.getRawParameterValue (id)->load(); };
    if (get (paramBypass) > 0.5f || get (paramAB) > 0.5f) return;

    const float inputGain = juce::Decibels::decibelsToGain (get (paramInput));
    const float outputGain = juce::Decibels::decibelsToGain (get (paramOutput));
    const float mix = get (paramMix);
    const float drive = get (paramDrive) * 8.0f;
    const float compAmount = get (paramComp);
    const float wet = get (paramReverb);
    const float hp = get (paramHP);
    const float bright = get (paramBright);
    highPass.state = juce::dsp::IIR::Coefficients<float>::makeHighPass (currentSampleRate, hp);
    lowPass.state = juce::dsp::IIR::Coefficients<float>::makeHighShelf (currentSampleRate, 4200.0f, 0.7f, juce::Decibels::decibelsToGain (bright));
    juce::dsp::Reverb::Parameters rp; rp.roomSize = 0.35f; rp.damping = 0.55f; rp.wetLevel = wet * 0.45f; rp.dryLevel = 1.0f; rp.width = 0.7f; reverb.setParameters (rp);

    for (int c = 0; c < channels; ++c)
    {
        auto* x = buffer.getWritePointer (c);
        for (int n = 0; n < samples; ++n) x[n] *= inputGain;
        juce::dsp::AudioBlock<float> block (buffer);
        auto channelBlock = block.getSingleChannelBlock (static_cast<size_t> (c));
        juce::dsp::ProcessContextReplacing<float> context (channelBlock);
        highPass.process (context);
        lowPass.process (context);
        for (int n = 0; n < samples; ++n)
        {
            float v = x[n];
            if (compAmount > 0.001f)
            {
                const float threshold = 1.0f - 0.72f * compAmount;
                const float magnitude = std::abs (v);
                if (magnitude > threshold) v = std::copysign (threshold + (magnitude - threshold) * (1.0f - 0.7f * compAmount), v);
            }
            if (drive > 0.001f) v = std::tanh (v * (1.0f + drive)) / std::tanh (1.0f + drive);
            x[n] = v;
        }
    }
    if (channels == 2 && wet > 0.001f) reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), samples);
    for (int c = 0; c < channels; ++c)
    {
        auto* x = buffer.getWritePointer (c); const auto* dry = dryBuffer.getReadPointer (c);
        for (int n = 0; n < samples; ++n) x[n] = (dry[n] * (1.0f - mix) + x[n] * mix) * outputGain;
    }
}

void VocalAIStudioProcessor::loadPreset (int index)
{
    const struct Preset { float hp, bright, comp, drive, rev; } presets[] = {
        { 75.0f, 0.0f, 0.22f, 0.0f, 0.05f }, { 95.0f, 2.5f, 0.58f, 0.18f, 0.04f },
        { 85.0f, 1.0f, 0.48f, 0.12f, 0.12f }, { 110.0f, 3.5f, 0.42f, 0.08f, 0.06f },
        { 65.0f, -2.5f, 0.38f, 0.12f, 0.15f }, { 80.0f, 0.5f, 0.30f, 0.05f, 0.38f },
        { 100.0f, 2.0f, 0.65f, 0.10f, 0.03f }, { 55.0f, 0.0f, 0.12f, 0.0f, 0.03f },
        { 85.0f, 2.0f, 0.44f, 0.04f, 0.08f }, { 75.0f, 1.0f, 0.30f, 0.0f, 0.20f }
    };
    const auto& p = presets[juce::jlimit (0, 9, index)];
    const std::pair<const char*, float> values[] = { { paramHP, p.hp }, { paramBright, p.bright }, { paramComp, p.comp }, { paramDrive, p.drive }, { paramReverb, p.rev } };
    for (const auto& item : values) if (auto* v = parameters.getParameter (item.first)) v->setValueNotifyingHost (v->convertTo0to1 (item.second));
}

void VocalAIStudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState(); std::unique_ptr<juce::XmlElement> xml (state.createXml()); copyXmlToBinary (*xml, destData);
}
void VocalAIStudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType())) parameters.replaceState (juce::ValueTree::fromXml (*xml));
}
juce::AudioProcessorEditor* VocalAIStudioProcessor::createEditor() { return new VocalAIStudioEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new VocalAIStudioProcessor(); }
