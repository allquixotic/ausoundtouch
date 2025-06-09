/*
  ==============================================================================

    This file is part of AUSoundTouch.
    Copyright (c) 2025 - Sean McNamara <smcnam@gmail.com>

    AUSoundTouch is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AUSoundTouch is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with AUSoundTouch.  If not, see <https://www.gnu.org/licenses/>.

  ==============================================================================
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"

AUSoundTouchProcessor::AUSoundTouchProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       parameters(*this, nullptr, juce::Identifier("AUSoundTouchParameters"), createParameterLayout())
{
    pitchParameter = parameters.getRawParameterValue("pitch");
    tempoParameter = parameters.getRawParameterValue("tempo");
    speedParameter = parameters.getRawParameterValue("speed");
}

AUSoundTouchProcessor::~AUSoundTouchProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout AUSoundTouchProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "pitch",
        "Pitch",
        juce::NormalisableRange<float>(MIN_PITCH_SEMITONES, MAX_PITCH_SEMITONES, 0.01f),
        DEFAULT_PITCH_SEMITONES,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { 
            if (std::abs(value) < 0.005f) return juce::String("0.00 st");
            return juce::String(value > 0 ? "+" : "") + juce::String(value, 2) + " st"; 
        },
        [](const juce::String& text) { return text.getFloatValue(); }
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "tempo",
        "Tempo",
        juce::NormalisableRange<float>(MIN_TEMPO_PERCENT, MAX_TEMPO_PERCENT, 0.1f),
        DEFAULT_TEMPO_PERCENT,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { 
            if (std::abs(value) < 0.05f) return juce::String("0.0%");
            return juce::String(value > 0 ? "+" : "") + juce::String(value, 1) + "%"; 
        },
        [](const juce::String& text) { return text.getFloatValue(); }
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "speed",
        "Speed",
        juce::NormalisableRange<float>(MIN_SPEED_PERCENT, MAX_SPEED_PERCENT, 0.1f),
        DEFAULT_SPEED_PERCENT,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { 
            if (std::abs(value) < 0.05f) return juce::String("0.0%");
            return juce::String(value > 0 ? "+" : "") + juce::String(value, 1) + "%"; 
        },
        [](const juce::String& text) { return text.getFloatValue(); }
    ));
    
    return { params.begin(), params.end() };
}

const juce::String AUSoundTouchProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AUSoundTouchProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AUSoundTouchProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AUSoundTouchProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AUSoundTouchProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AUSoundTouchProcessor::getNumPrograms()
{
    return 1;
}

int AUSoundTouchProcessor::getCurrentProgram()
{
    return 0;
}

void AUSoundTouchProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AUSoundTouchProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AUSoundTouchProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void AUSoundTouchProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    soundTouch.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
}

void AUSoundTouchProcessor::releaseResources()
{
}

void AUSoundTouchProcessor::setBufferingMode(int mode)
{
    if (mode >= Minimal && mode <= Extra)
    {
        bufferingMode.store(mode);
        soundTouch.setBufferingMode(mode);
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AUSoundTouchProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AUSoundTouchProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    soundTouch.setPitch(*pitchParameter);
    soundTouch.setTempo(*tempoParameter);
    soundTouch.setRate(*speedParameter);
    
    soundTouch.processBlock(buffer);
}

bool AUSoundTouchProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AUSoundTouchProcessor::createEditor()
{
    return new AUSoundTouchEditor (*this);
}

void AUSoundTouchProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    
    // Add buffering mode as an attribute
    xml->setAttribute("bufferingMode", bufferingMode.load());
    
    copyXmlToBinary (*xml, destData);
}

void AUSoundTouchProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
            
        // Restore buffering mode
        const int savedBufferingMode = xmlState->getIntAttribute("bufferingMode", Normal);
        setBufferingMode(savedBufferingMode);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AUSoundTouchProcessor();
}