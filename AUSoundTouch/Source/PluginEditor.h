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
#include "PluginProcessor.h"

class AUSoundTouchEditor : public juce::AudioProcessorEditor,
                           private juce::Timer
{
public:
    AUSoundTouchEditor (AUSoundTouchProcessor&);
    ~AUSoundTouchEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AUSoundTouchProcessor& audioProcessor;
    
    juce::Label titleLabel;
    
    struct ParameterControl
    {
        juce::Label label;
        juce::Slider slider;
        juce::TextEditor textDisplay;
        juce::Label unitLabel;
        juce::TextButton resetButton;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };
    
    ParameterControl tempoControl;
    ParameterControl pitchControl;
    ParameterControl speedControl;
    
    // Buffering control
    juce::Label bufferingLabel;
    juce::ComboBox bufferingComboBox;
    
    bool uiInitialized = false;
    
    void timerCallback() override;
    void setupUI();
    void setupParameterControl(ParameterControl& control, 
                             const juce::String& name,
                             const juce::String& paramId,
                             const juce::String& resetLabel = "Reset");
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AUSoundTouchEditor)
};