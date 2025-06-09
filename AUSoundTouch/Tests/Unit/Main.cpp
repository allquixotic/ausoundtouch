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

int main()
{
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure(false);
    runner.setPassesAreLogged(true);
    
    runner.runAllTests();
    
    int numFailures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        if (runner.getResult(i)->failures > 0)
            numFailures += runner.getResult(i)->failures;
    }
    
    if (numFailures > 0)
    {
        std::cout << "\n*** " << numFailures << " test(s) failed ***\n";
        return 1;
    }
    
    std::cout << "\nAll tests passed!\n";
    return 0;
}