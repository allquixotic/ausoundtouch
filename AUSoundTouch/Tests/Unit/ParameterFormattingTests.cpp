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

class ParameterFormattingTests : public juce::UnitTest
{
public:
    ParameterFormattingTests() : UnitTest("Parameter Formatting Tests") {}
    
    void runTest() override
    {
        beginTest("Pitch parameter formatting");
        {
            AUSoundTouchProcessor processor;
            auto& params = processor.getParameters();
            auto* pitchParam = params.getParameter("pitch");
            
            // Test positive values with + sign
            pitchParam->setValueNotifyingHost(pitchParam->convertTo0to1(10.5f));
            auto text = pitchParam->getText(pitchParam->getValue(), 1024);
            expect(text == "+10.50 st", "Positive pitch should show + sign");
            
            // Test negative values with - sign
            pitchParam->setValueNotifyingHost(pitchParam->convertTo0to1(-5.25f));
            text = pitchParam->getText(pitchParam->getValue(), 1024);
            expect(text == "-5.25 st", "Negative pitch should show - sign");
            
            // Test zero without sign
            pitchParam->setValueNotifyingHost(pitchParam->convertTo0to1(0.0f));
            text = pitchParam->getText(pitchParam->getValue(), 1024);
            expect(text == "0.00 st", "Zero pitch should not show sign");
        }
        
        beginTest("Tempo parameter formatting");
        {
            AUSoundTouchProcessor processor;
            auto& params = processor.getParameters();
            auto* tempoParam = params.getParameter("tempo");
            
            // Test positive values with + sign
            tempoParam->setValueNotifyingHost(tempoParam->convertTo0to1(50.0f));
            auto text = tempoParam->getText(tempoParam->getValue(), 1024);
            expect(text == "+50.0%", "Positive tempo should show + sign");
            
            // Test negative values with - sign
            tempoParam->setValueNotifyingHost(tempoParam->convertTo0to1(-30.0f));
            text = tempoParam->getText(tempoParam->getValue(), 1024);
            expect(text == "-30.0%", "Negative tempo should show - sign");
            
            // Test zero without sign
            tempoParam->setValueNotifyingHost(tempoParam->convertTo0to1(0.0f));
            text = tempoParam->getText(tempoParam->getValue(), 1024);
            expect(text == "0.0%", "Zero tempo should not show sign");
        }
        
        beginTest("Speed parameter formatting");
        {
            AUSoundTouchProcessor processor;
            auto& params = processor.getParameters();
            auto* speedParam = params.getParameter("speed");
            
            // Test positive values with + sign
            speedParam->setValueNotifyingHost(speedParam->convertTo0to1(100.0f));
            auto text = speedParam->getText(speedParam->getValue(), 1024);
            expect(text == "+100.0%", "Positive speed should show + sign");
            
            // Test negative values with - sign
            speedParam->setValueNotifyingHost(speedParam->convertTo0to1(-45.0f));
            text = speedParam->getText(speedParam->getValue(), 1024);
            expect(text == "-45.0%", "Negative speed should show - sign");
            
            // Test zero without sign
            speedParam->setValueNotifyingHost(speedParam->convertTo0to1(0.0f));
            text = speedParam->getText(speedParam->getValue(), 1024);
            expect(text == "0.0%", "Zero speed should not show sign");
        }
        
        beginTest("Parameter reset functionality");
        {
            AUSoundTouchProcessor processor;
            auto& params = processor.getParameters();
            
            // Test pitch reset
            auto* pitchParam = params.getParameter("pitch");
            pitchParam->setValueNotifyingHost(pitchParam->convertTo0to1(15.0f));
            expectWithinAbsoluteError(pitchParam->convertFrom0to1(pitchParam->getValue()), 15.0f, 0.01f);
            
            // Reset to default
            pitchParam->setValueNotifyingHost(pitchParam->getDefaultValue());
            expectWithinAbsoluteError(pitchParam->convertFrom0to1(pitchParam->getValue()), 0.0f, 0.01f);
            
            // Test tempo reset
            auto* tempoParam = params.getParameter("tempo");
            tempoParam->setValueNotifyingHost(tempoParam->convertTo0to1(75.0f));
            expectWithinAbsoluteError(tempoParam->convertFrom0to1(tempoParam->getValue()), 75.0f, 0.1f);
            
            // Reset to default
            tempoParam->setValueNotifyingHost(tempoParam->getDefaultValue());
            expectWithinAbsoluteError(tempoParam->convertFrom0to1(tempoParam->getValue()), 0.0f, 0.1f);
            
            // Test speed reset
            auto* speedParam = params.getParameter("speed");
            speedParam->setValueNotifyingHost(speedParam->convertTo0to1(-50.0f));
            expectWithinAbsoluteError(speedParam->convertFrom0to1(speedParam->getValue()), -50.0f, 0.1f);
            
            // Reset to default
            speedParam->setValueNotifyingHost(speedParam->getDefaultValue());
            expectWithinAbsoluteError(speedParam->convertFrom0to1(speedParam->getValue()), 0.0f, 0.1f);
        }
    }
};

static ParameterFormattingTests parameterFormattingTests;