#include "PluginProcessor.h"

using APVTS = juce::AudioProcessorValueTreeState;

APVTS::ParameterLayout WizahFXProcessor::createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "drive", 1 }, "Drive",
                 NormalisableRange<float> (0.0f, 24.0f, 0.1f), 6.0f, AudioParameterFloatAttributes().withLabel ("dB")));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "tone", 1 }, "Tone",
                 NormalisableRange<float> (200.0f, 20000.0f, 1.0f, 0.3f), 8000.0f, AudioParameterFloatAttributes().withLabel ("Hz")));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "time", 1 }, "Delay Time",
                 NormalisableRange<float> (10.0f, 1000.0f, 1.0f, 0.5f), 350.0f, AudioParameterFloatAttributes().withLabel ("ms")));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "feedback", 1 }, "Feedback",
                 NormalisableRange<float> (0.0f, 0.95f, 0.01f), 0.35f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "mix", 1 }, "Mix",
                 NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.4f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "output", 1 }, "Output",
                 NormalisableRange<float> (-24.0f, 12.0f, 0.1f), 0.0f, AudioParameterFloatAttributes().withLabel ("dB")));

    return { p.begin(), p.end() };
}

WizahFXProcessor::WizahFXProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

bool WizahFXProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto out = l.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo())
           && out == l.getMainInputChannelSet();
}

void WizahFXProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    tone.prepare (spec);
    tone.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    delayBuf.setSize (2, (int) (sampleRate * 1.0) + 4);
    delayBuf.clear();
    writePos = 0;

    const double ramp = 0.05;
    driveSm.reset (sampleRate, ramp);
    mixSm.reset   (sampleRate, ramp);
    delaySm.reset (sampleRate, 0.1);
    outSm.reset   (sampleRate, ramp);

    driveSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("drive")->load()));
    mixSm.setCurrentAndTargetValue   (apvts.getRawParameterValue ("mix")->load());
    delaySm.setCurrentAndTargetValue ((float) (apvts.getRawParameterValue ("time")->load() * 0.001 * sampleRate));
    outSm.setCurrentAndTargetValue   (juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("output")->load()));
}

void WizahFXProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh = juce::jmin (getTotalNumOutputChannels(), 2);
    const int n = buffer.getNumSamples();
    const int bufLen = delayBuf.getNumSamples();

    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear (c, 0, n);

    driveSm.setTargetValue (juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("drive")->load()));
    mixSm.setTargetValue   (apvts.getRawParameterValue ("mix")->load());
    delaySm.setTargetValue ((float) (apvts.getRawParameterValue ("time")->load() * 0.001 * sr));
    outSm.setTargetValue   (juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("output")->load()));
    const float fb = apvts.getRawParameterValue ("feedback")->load();
    tone.setCutoffFrequency (apvts.getRawParameterValue ("tone")->load());

    for (int s = 0; s < n; ++s)
    {
        const float g   = driveSm.getNextValue();
        const float m   = mixSm.getNextValue();
        const float d   = juce::jlimit (1.0f, (float) (bufLen - 2), delaySm.getNextValue());
        const float o   = outSm.getNextValue();
        const float norm = 1.0f / std::tanh (g);

        float readPos = (float) writePos - d;
        if (readPos < 0.0f) readPos += (float) bufLen;
        const int   i0   = (int) readPos;
        const int   i1   = (i0 + 1) % bufLen;
        const float frac = readPos - (float) i0;

        for (int c = 0; c < numCh; ++c)
        {
            auto* io = buffer.getWritePointer (c);
            auto* db = delayBuf.getWritePointer (c);

            const float x = io[s];

            // 1) soft-clip distortion, level-compensated
            float y = std::tanh (g * x) * norm;

            // 2) tone (low-pass)
            y = tone.processSample (c, y);

            // 3) delay with feedback (reads old signal, writes new)
            const float delayed = db[i0] + frac * (db[i1] - db[i0]);
            db[writePos] = y + delayed * fb;

            // 4) dry/wet + output gain
            io[s] = (x * (1.0f - m) + (y + delayed) * m) * o;
        }

        writePos = (writePos + 1) % bufLen;
    }
}

juce::AudioProcessorEditor* WizahFXProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

void WizahFXProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void WizahFXProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WizahFXProcessor();
}
