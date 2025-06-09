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
#include "SoundTouchWrapper.h"

class AUSoundTouchProcessor : public juce::AudioProcessor
{
public:
    AUSoundTouchProcessor();
    ~AUSoundTouchProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Buffering modes
    enum BufferingMode
    {
        Minimal = 1,
        Normal = 2,
        Extra = 3
    };
    
    void setBufferingMode(int mode);
    int getBufferingMode() const { return bufferingMode.load(); }

    static constexpr float MIN_PITCH_SEMITONES = -39.8f;
    static constexpr float MAX_PITCH_SEMITONES = 39.8f;
    static constexpr float DEFAULT_PITCH_SEMITONES = 0.0f;
    
    static constexpr float MIN_TEMPO_PERCENT = -90.0f;
    static constexpr float MAX_TEMPO_PERCENT = 900.0f;
    static constexpr float DEFAULT_TEMPO_PERCENT = 0.0f;
    
    static constexpr float MIN_SPEED_PERCENT = -90.0f;
    static constexpr float MAX_SPEED_PERCENT = 900.0f;
    static constexpr float DEFAULT_SPEED_PERCENT = 0.0f;

private:
    juce::AudioProcessorValueTreeState parameters;
    SoundTouchWrapper soundTouch;
    
    std::atomic<float>* pitchParameter = nullptr;
    std::atomic<float>* tempoParameter = nullptr;
    std::atomic<float>* speedParameter = nullptr;
    
    std::atomic<int> bufferingMode { Normal };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AUSoundTouchProcessor)
};