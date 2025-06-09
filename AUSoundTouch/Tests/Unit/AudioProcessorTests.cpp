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
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AudioProcessorTests : public juce::UnitTest
{
public:
    AudioProcessorTests() : UnitTest("Audio Processor Tests") {}
    
    void runTest() override
    {
        beginTest("Plugin Creation");
        {
            AUSoundTouchProcessor processor;
            expectEquals(processor.getName(), juce::String("AUSoundTouch"));
            expect(!processor.acceptsMidi());
            expect(!processor.producesMidi());
            expect(!processor.isMidiEffect());
            expect(processor.hasEditor()); // Processor does have an editor
        }
        
        beginTest("Bus Configuration");
        {
            AUSoundTouchProcessor processor;
            auto layout = processor.getBusesLayout();
            
            expect(layout.getMainInputChannelSet() == juce::AudioChannelSet::stereo());
            expect(layout.getMainOutputChannelSet() == juce::AudioChannelSet::stereo());
        }
        
        beginTest("Parameter Creation");
        {
            AUSoundTouchProcessor processor;
            auto& params = processor.getParameters();
            
            auto* pitchParam = params.getParameter("pitch");
            auto* tempoParam = params.getParameter("tempo");
            auto* speedParam = params.getParameter("speed");
            
            expect(pitchParam != nullptr);
            expect(tempoParam != nullptr);
            expect(speedParam != nullptr);
            
            if (pitchParam)
            {
                expectEquals(pitchParam->getName(128), juce::String("Pitch"));
                expectEquals(pitchParam->getValue(), pitchParam->getDefaultValue());
            }
            
            if (tempoParam)
            {
                expectEquals(tempoParam->getName(128), juce::String("Tempo"));
                expectEquals(tempoParam->getValue(), tempoParam->getDefaultValue());
            }
            
            if (speedParam)
            {
                expectEquals(speedParam->getName(128), juce::String("Speed"));
                expectEquals(speedParam->getValue(), speedParam->getDefaultValue());
            }
        }
        
        beginTest("Parameter Ranges");
        {
            AUSoundTouchProcessor processor;
            auto& params = processor.getParameters();
            
            auto* pitchParam = dynamic_cast<juce::AudioParameterFloat*>(params.getParameter("pitch"));
            auto* tempoParam = dynamic_cast<juce::AudioParameterFloat*>(params.getParameter("tempo"));
            auto* speedParam = dynamic_cast<juce::AudioParameterFloat*>(params.getParameter("speed"));
            
            if (pitchParam)
            {
                auto range = pitchParam->getNormalisableRange();
                expectEquals(range.start, -39.8f);
                expectEquals(range.end, 39.8f);
            }
            
            if (tempoParam)
            {
                auto range = tempoParam->getNormalisableRange();
                expectEquals(range.start, -90.0f);
                expectEquals(range.end, 900.0f);
            }
            
            if (speedParam)
            {
                auto range = speedParam->getNormalisableRange();
                expectEquals(range.start, -90.0f);
                expectEquals(range.end, 900.0f);
            }
        }
        
        beginTest("State Save/Restore");
        {
            AUSoundTouchProcessor processor1;
            auto& apvts1 = processor1.getParameters();
            
            // Set some test values (in actual parameter range, not normalized)
            const float pitchValue = 10.0f;  // 10 semitones
            const float tempoValue = 50.0f;  // 50% tempo change
            const float speedValue = -25.0f; // -25% speed change
            
            // Get the actual parameter objects from AudioProcessorValueTreeState
            if (auto* pitchParam = apvts1.getParameter("pitch"))
                pitchParam->setValueNotifyingHost(pitchParam->convertTo0to1(pitchValue));
            if (auto* tempoParam = apvts1.getParameter("tempo"))
                tempoParam->setValueNotifyingHost(tempoParam->convertTo0to1(tempoValue));
            if (auto* speedParam = apvts1.getParameter("speed"))
                speedParam->setValueNotifyingHost(speedParam->convertTo0to1(speedValue));
            
            // Store the actual values before saving
            const float pitch1 = apvts1.getParameter("pitch")->convertFrom0to1(apvts1.getParameter("pitch")->getValue());
            const float tempo1 = apvts1.getParameter("tempo")->convertFrom0to1(apvts1.getParameter("tempo")->getValue());
            const float speed1 = apvts1.getParameter("speed")->convertFrom0to1(apvts1.getParameter("speed")->getValue());
            
            juce::MemoryBlock stateData;
            processor1.getStateInformation(stateData);
            
            AUSoundTouchProcessor processor2;
            processor2.setStateInformation(stateData.getData(), static_cast<int>(stateData.getSize()));
            
            auto& apvts2 = processor2.getParameters();
            const float pitch2 = apvts2.getParameter("pitch")->convertFrom0to1(apvts2.getParameter("pitch")->getValue());
            const float tempo2 = apvts2.getParameter("tempo")->convertFrom0to1(apvts2.getParameter("tempo")->getValue());
            const float speed2 = apvts2.getParameter("speed")->convertFrom0to1(apvts2.getParameter("speed")->getValue());
            
            expectWithinAbsoluteError(pitch2, pitch1, 0.01f);
            expectWithinAbsoluteError(tempo2, tempo1, 0.01f);
            expectWithinAbsoluteError(speed2, speed1, 0.01f);
        }
        
        beginTest("Audio Processing");
        {
            AUSoundTouchProcessor processor;
            processor.prepareToPlay(44100.0, 512);
            
            juce::MidiBuffer midiBuffer;
            
            // Process several blocks to account for SoundTouch's internal buffering
            bool hasOutput = false;
            const float testFrequency = 440.0f;
            const float sampleRate = 44100.0f;
            
            for (int block = 0; block < 4 && !hasOutput; ++block)
            {
                juce::AudioBuffer<float> buffer(2, 512);
                
                // Fill buffer with test signal
                for (int sample = 0; sample < 512; ++sample)
                {
                    const float phase = 2.0f * juce::MathConstants<float>::pi * 
                                      testFrequency * static_cast<float>(block * 512 + sample) / sampleRate;
                    const float value = std::sin(phase);
                    buffer.setSample(0, sample, value);
                    buffer.setSample(1, sample, value);
                }
                
                processor.processBlock(buffer, midiBuffer);
                
                if (buffer.getMagnitude(0, 0, 512) > 0.0f)
                    hasOutput = true;
            }
            
            expect(hasOutput);
            
            processor.releaseResources();
        }
    }
};

static AudioProcessorTests audioProcessorTests;