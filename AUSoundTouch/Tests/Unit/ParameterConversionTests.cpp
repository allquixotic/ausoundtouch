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

class ParameterConversionTests : public juce::UnitTest
{
public:
    ParameterConversionTests() : UnitTest("Parameter Conversion Tests") {}
    
    void runTest() override
    {
        beginTest("Semitones to Native Conversion");
        {
            expectWithinAbsoluteError(SoundTouchWrapper::semitonesToNative(0.0f), 1.0f, 0.0001f);
            expectWithinAbsoluteError(SoundTouchWrapper::semitonesToNative(12.0f), 2.0f, 0.0001f);
            expectWithinAbsoluteError(SoundTouchWrapper::semitonesToNative(-12.0f), 0.5f, 0.0001f);
            expectWithinAbsoluteError(SoundTouchWrapper::semitonesToNative(1.0f), 1.0594630943592953f, 0.0001f);
            expectWithinAbsoluteError(SoundTouchWrapper::semitonesToNative(-1.0f), 0.9438743126816935f, 0.0001f);
        }
        
        beginTest("Percentage to Native Conversion");
        {
            expectWithinAbsoluteError(SoundTouchWrapper::percentageToNative(0.0f), 1.0f, 0.0001f);
            expectWithinAbsoluteError(SoundTouchWrapper::percentageToNative(100.0f), 2.0f, 0.0001f);
            expectWithinAbsoluteError(SoundTouchWrapper::percentageToNative(-50.0f), 0.5f, 0.0001f);
            expectWithinAbsoluteError(SoundTouchWrapper::percentageToNative(200.0f), 3.0f, 0.0001f);
            expectWithinAbsoluteError(SoundTouchWrapper::percentageToNative(-90.0f), 0.1f, 0.0001f);
            expectWithinAbsoluteError(SoundTouchWrapper::percentageToNative(900.0f), 10.0f, 0.0001f);
        }
        
        beginTest("Range Boundary Tests");
        {
            const float minPitchNative = SoundTouchWrapper::semitonesToNative(-39.8f);
            const float maxPitchNative = SoundTouchWrapper::semitonesToNative(39.8f);
            expect(minPitchNative > 0.0f && minPitchNative < 1.0f);
            expect(maxPitchNative > 1.0f && maxPitchNative < 100.0f);
            
            const float minTempoNative = SoundTouchWrapper::percentageToNative(-90.0f);
            const float maxTempoNative = SoundTouchWrapper::percentageToNative(900.0f);
            expectWithinAbsoluteError(minTempoNative, 0.1f, 0.0001f);
            expectWithinAbsoluteError(maxTempoNative, 10.0f, 0.0001f);
        }
    }
};

static ParameterConversionTests parameterConversionTests;