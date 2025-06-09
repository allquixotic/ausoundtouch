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

AUSoundTouchEditor::AUSoundTouchEditor (AUSoundTouchProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Minimal setup - just set size and optimization flags
    setSize (680, 320); // Wider to accommodate unit labels
    
    // Option 4: Rendering optimizations
    setOpaque(true); // No transparency needed - improves performance
    setBufferedToImage(true); // Cache rendered content
    
    // Option 1: Defer UI creation to avoid CPU spike on load
    startTimer(1); // Start timer with 1ms delay
}

AUSoundTouchEditor::~AUSoundTouchEditor()
{
}

void AUSoundTouchEditor::timerCallback()
{
    stopTimer(); // Only run once
    setupUI();
    resized(); // Trigger layout after UI is created
}

void AUSoundTouchEditor::setupUI()
{
    if (uiInitialized)
        return;
    
    titleLabel.setText("AUSoundTouch", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    setupParameterControl(tempoControl, "Tempo", "tempo");
    setupParameterControl(pitchControl, "Pitch", "pitch");
    setupParameterControl(speedControl, "Speed", "speed");
    
    // Setup buffering control
    bufferingLabel.setText("Buffering:", juce::dontSendNotification);
    bufferingLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(bufferingLabel);
    
    bufferingComboBox.addItem("Minimal", 1);
    bufferingComboBox.addItem("Normal", 2);
    bufferingComboBox.addItem("Extra", 3);
    bufferingComboBox.setSelectedId(audioProcessor.getBufferingMode()); // Get current mode from processor
    addAndMakeVisible(bufferingComboBox);
    
    // Handle buffering changes
    bufferingComboBox.onChange = [this]() {
        const int bufferingMode = bufferingComboBox.getSelectedId();
        audioProcessor.setBufferingMode(bufferingMode);
    };
    
    uiInitialized = true;
}

void AUSoundTouchEditor::setupParameterControl(ParameterControl& control,
                                              const juce::String& name,
                                              const juce::String& paramId,
                                              const juce::String& resetLabel)
{
    control.label.setText(name, juce::dontSendNotification);
    control.label.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(control.label);
    
    control.slider.setSliderStyle(juce::Slider::LinearHorizontal);
    control.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(control.slider);
    
    // Configure TextEditor
    control.textDisplay.setJustification(juce::Justification::centred);
    control.textDisplay.setSelectAllWhenFocused(true);
    control.textDisplay.setInputRestrictions(8, "0123456789.-+"); // Allow numbers, decimal, minus, plus
    addAndMakeVisible(control.textDisplay);
    
    control.resetButton.setButtonText(resetLabel);
    addAndMakeVisible(control.resetButton);
    
    // Setup unit label
    juce::String unitText;
    if (paramId == "pitch")
        unitText = "semitones";
    else // tempo or speed
        unitText = "%";
    
    control.unitLabel.setText(unitText, juce::dontSendNotification);
    control.unitLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(control.unitLabel);
    
    control.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), paramId, control.slider
    );
    
    auto* param = audioProcessor.getParameters().getParameter(paramId);
    
    // Update text display when slider changes
    control.slider.onValueChange = [&control, param, paramId]() {
        const auto value = control.slider.getValue();
        // Display just the numeric value without units
        if (paramId == "pitch")
        {
            control.textDisplay.setText(juce::String(value, 2), juce::dontSendNotification);
        }
        else // tempo or speed
        {
            control.textDisplay.setText(juce::String(value, 1), juce::dontSendNotification);
        }
    };
    
    // Helper function to apply text input
    auto applyTextInput = [&control, param, paramId]() {
        const auto text = control.textDisplay.getText();
        const float numericValue = text.getFloatValue();
        
        // Clamp to parameter range
        const auto range = param->getNormalisableRange();
        const float clampedValue = juce::jlimit(range.start, range.end, numericValue);
        
        control.slider.setValue(clampedValue, juce::sendNotification);
    };
    
    // Handle Enter key
    control.textDisplay.onReturnKey = applyTextInput;
    
    // Handle focus loss
    control.textDisplay.onFocusLost = applyTextInput;
    
    control.resetButton.onClick = [&control, param]() {
        const auto defaultValue = param->convertFrom0to1(param->getDefaultValue());
        control.slider.setValue(defaultValue, juce::sendNotification);
    };
    
    // Initialize values
    control.slider.setValue(param->convertFrom0to1(param->getValue()), juce::dontSendNotification);
    if (paramId == "pitch")
    {
        control.textDisplay.setText(juce::String(control.slider.getValue(), 2), juce::dontSendNotification);
    }
    else
    {
        control.textDisplay.setText(juce::String(control.slider.getValue(), 1), juce::dontSendNotification);
    }
}

void AUSoundTouchEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void AUSoundTouchEditor::resized()
{
    // Don't try to layout components if they haven't been created yet
    if (!uiInitialized)
        return;
        
    auto bounds = getLocalBounds();
    
    titleLabel.setBounds(bounds.removeFromTop(40));
    
    bounds.reduce(15, 5); // Tighter margins to accommodate unit labels
    
    const int controlHeight = 60; // Height for each parameter section
    const int labelWidth = 70;
    const int sliderHeight = 25;
    const int textBoxWidth = 60;
    const int unitLabelWidth = 70;
    const int buttonWidth = 50;
    const int spacing = 8;
    
    auto setupControl = [&](ParameterControl& control, juce::Rectangle<int> area) {
        // Label on the left
        control.label.setBounds(area.removeFromLeft(labelWidth));
        
        // Text display, unit label, and reset button on the right
        auto rightSection = area.removeFromRight(textBoxWidth + unitLabelWidth + buttonWidth + spacing * 2);
        control.textDisplay.setBounds(rightSection.removeFromLeft(textBoxWidth));
        rightSection.removeFromLeft(spacing);
        control.unitLabel.setBounds(rightSection.removeFromLeft(unitLabelWidth));
        rightSection.removeFromLeft(spacing);
        control.resetButton.setBounds(rightSection);
        
        // Slider takes the remaining space in the middle
        area.reduce(spacing, (area.getHeight() - sliderHeight) / 2);
        control.slider.setBounds(area);
    };
    
    // Stack controls vertically: Tempo, Pitch, Speed
    setupControl(tempoControl, bounds.removeFromTop(controlHeight));
    bounds.removeFromTop(5); // Small gap between controls
    setupControl(pitchControl, bounds.removeFromTop(controlHeight));
    bounds.removeFromTop(5);
    setupControl(speedControl, bounds.removeFromTop(controlHeight));
    bounds.removeFromTop(10); // Slightly larger gap before buffering control
    
    // Buffering control
    auto bufferingBounds = bounds.removeFromTop(30);
    bufferingLabel.setBounds(bufferingBounds.removeFromLeft(labelWidth));
    bufferingBounds.removeFromLeft(spacing);
    bufferingComboBox.setBounds(bufferingBounds.removeFromLeft(150));
}