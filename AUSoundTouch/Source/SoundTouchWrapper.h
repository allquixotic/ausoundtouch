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
#pragma once

#include <JuceHeader.h>
// Handle both system and fetched SoundTouch includes
#if __has_include(<soundtouch/SoundTouch.h>)
    #include <soundtouch/SoundTouch.h>
#else
    #include <SoundTouch.h>
#endif
#include <memory>

class SoundTouchWrapper
{
public:
    SoundTouchWrapper();
    ~SoundTouchWrapper();
    
    void prepare(double sampleRate, int blockSize, int numChannels);
    
    void setPitch(float semitones);
    void setTempo(float percentage);
    void setRate(float percentage);
    
    void processBlock(juce::AudioBuffer<float>& buffer);
    
    int getLatencyInSamples() const;
    
    static float semitonesToNative(float semitones);
    static float percentageToNative(float percentage);
    
    // Buffering mode: 1=Minimal, 2=Normal, 3=Extra
    void setBufferingMode(int mode);
    
private:
    std::unique_ptr<soundtouch::SoundTouch> processor;
    
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    int currentNumChannels = 2;
    
    std::vector<float> interleavedBuffer;
    
    // FIFO buffer for output samples
    std::unique_ptr<juce::AbstractFifo> outputFifo;
    std::vector<float> fifoBuffer;
    
    int bufferingMode = 2; // Default to Normal
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundTouchWrapper)
};