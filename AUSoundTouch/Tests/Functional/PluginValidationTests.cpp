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

class PluginValidationTests : public juce::UnitTest
{
public:
    PluginValidationTests() : UnitTest("Plugin Validation Tests") {}
    
    void runTest() override
    {
        beginTest("Complete Plugin Lifecycle");
        {
            AUSoundTouchProcessor processor;
            
            processor.prepareToPlay(44100.0, 512);
            
            juce::AudioBuffer<float> buffer(2, 512);
            juce::MidiBuffer midiBuffer;
            
            for (int i = 0; i < 100; ++i)
            {
                buffer.clear();
                generateTestSignal(buffer, 440.0f, 44100.0f);
                processor.processBlock(buffer, midiBuffer);
            }
            
            processor.releaseResources();
        }
        
        beginTest("Parameter Automation");
        {
            AUSoundTouchProcessor processor;
            processor.prepareToPlay(44100.0, 512);
            
            auto& params = processor.getParameters();
            auto* pitchParam = params.getParameter("pitch");
            auto* tempoParam = params.getParameter("tempo");
            
            juce::AudioBuffer<float> buffer(2, 512);
            juce::MidiBuffer midiBuffer;
            
            for (float normValue = 0.0f; normValue <= 1.0f; normValue += 0.1f)
            {
                pitchParam->setValue(normValue);
                tempoParam->setValue(1.0f - normValue);
                
                buffer.clear();
                generateTestSignal(buffer, 440.0f, 44100.0f);
                processor.processBlock(buffer, midiBuffer);
            }
            
            processor.releaseResources();
        }
        
        beginTest("Latency Compensation");
        {
            AUSoundTouchProcessor processor;
            processor.prepareToPlay(44100.0, 512);
            
            const int reportedLatency = processor.getLatencySamples();
            expectGreaterOrEqual(reportedLatency, 0);
            expectLessThan(reportedLatency, 44100);
            
            processor.releaseResources();
        }
        
        beginTest("Thread Safety");
        {
            AUSoundTouchProcessor processor;
            processor.prepareToPlay(44100.0, 512);
            
            std::atomic<bool> shouldStop(false);
            
            auto audioThread = std::thread([&processor, &shouldStop]()
            {
                juce::AudioBuffer<float> buffer(2, 512);
                juce::MidiBuffer midiBuffer;
                
                while (!shouldStop)
                {
                    buffer.clear();
                    processor.processBlock(buffer, midiBuffer);
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                }
            });
            
            auto& params = processor.getParameters();
            for (int i = 0; i < 100; ++i)
            {
                params.getParameter("pitch")->setValue(juce::Random::getSystemRandom().nextFloat());
                params.getParameter("tempo")->setValue(juce::Random::getSystemRandom().nextFloat());
                params.getParameter("speed")->setValue(juce::Random::getSystemRandom().nextFloat());
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            
            shouldStop = true;
            audioThread.join();
            
            processor.releaseResources();
        }
    }
    
private:
    void generateTestSignal(juce::AudioBuffer<float>& buffer, float frequency, float sampleRate)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float value = std::sin(2.0f * juce::MathConstants<float>::pi * 
                                       frequency * static_cast<float>(sample) / sampleRate);
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                buffer.setSample(channel, sample, value * 0.5f);
            }
        }
    }
};

static PluginValidationTests pluginValidationTests;