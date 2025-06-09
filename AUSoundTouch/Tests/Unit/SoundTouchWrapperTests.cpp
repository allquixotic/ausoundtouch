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
#include "SoundTouchWrapper.h"

class SoundTouchWrapperTests : public juce::UnitTest
{
public:
    SoundTouchWrapperTests() : UnitTest("SoundTouch Wrapper Tests") {}
    
    void runTest() override
    {
        beginTest("Initialization");
        {
            SoundTouchWrapper wrapper;
            expect(wrapper.getLatencyInSamples() == 0);
        }
        
        beginTest("Prepare Method");
        {
            SoundTouchWrapper wrapper;
            wrapper.prepare(44100.0, 512, 2);
            expect(wrapper.getLatencyInSamples() >= 0);
        }
        
        beginTest("Parameter Setting");
        {
            SoundTouchWrapper wrapper;
            wrapper.prepare(44100.0, 512, 2);
            
            wrapper.setPitch(0.0f);
            wrapper.setTempo(0.0f);
            wrapper.setRate(0.0f);
            
            wrapper.setPitch(12.0f);
            wrapper.setTempo(50.0f);
            wrapper.setRate(-25.0f);
            
            wrapper.setPitch(-39.8f);
            wrapper.setTempo(900.0f);
            wrapper.setRate(-90.0f);
        }
        
        beginTest("Basic Audio Processing");
        {
            SoundTouchWrapper wrapper;
            wrapper.prepare(44100.0, 512, 2);
            
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
                
                wrapper.processBlock(buffer);
                
                if (buffer.getMagnitude(0, 0, 512) > 0.0f)
                    hasOutput = true;
            }
            
            expect(hasOutput);
        }
        
        beginTest("Latency Reporting");
        {
            SoundTouchWrapper wrapper;
            wrapper.prepare(44100.0, 512, 2);
            
            juce::AudioBuffer<float> buffer(2, 512);
            buffer.clear();
            
            const int initialLatency = wrapper.getLatencyInSamples();
            expectGreaterOrEqual(initialLatency, 0);
            
            wrapper.setPitch(12.0f);
            wrapper.processBlock(buffer);
            
            const int afterProcessLatency = wrapper.getLatencyInSamples();
            expectGreaterOrEqual(afterProcessLatency, 0);
        }
    }
};

static SoundTouchWrapperTests soundTouchWrapperTests;