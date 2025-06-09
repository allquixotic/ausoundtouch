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
#include "SoundTouchWrapper.h"
#include <cmath>

SoundTouchWrapper::SoundTouchWrapper()
    : processor(std::make_unique<soundtouch::SoundTouch>())
{
    // Configure for best quality
    processor->setSetting(SETTING_USE_QUICKSEEK, 0); // Disable quickseek for better quality
    processor->setSetting(SETTING_USE_AA_FILTER, 1); // Enable anti-alias filter
    
    // Increase anti-alias filter length for better quality (default is 32)
    processor->setSetting(SETTING_AA_FILTER_LENGTH, 64); // Higher = better quality but more CPU
    
    // Use default processing settings for better quality
    // These are the SoundTouch defaults that produce high-quality output:
    processor->setSetting(SETTING_SEQUENCE_MS, 40); // Default is 40ms
    processor->setSetting(SETTING_SEEKWINDOW_MS, 15); // Default is 15ms  
    processor->setSetting(SETTING_OVERLAP_MS, 8); // Default is 8ms
    
    // Note: We're already using float samples (SOUNDTOUCH_FLOAT_SAMPLES) which is
    // compiled into the Homebrew version of SoundTouch for best quality
}

SoundTouchWrapper::~SoundTouchWrapper() = default;

void SoundTouchWrapper::prepare(double sampleRate, int blockSize, int numChannels)
{
    currentSampleRate = sampleRate;
    currentBlockSize = blockSize;
    currentNumChannels = numChannels;
    
    processor->setSampleRate(static_cast<uint>(sampleRate));
    processor->setChannels(static_cast<uint>(numChannels));
    
    interleavedBuffer.resize(static_cast<size_t>(blockSize * numChannels * 2));
    
    // Initialize FIFO buffer based on current buffering mode
    int fifoSize;
    switch (bufferingMode)
    {
        case 1: // Minimal
            fifoSize = std::max(4096, blockSize * 8);
            break;
        case 2: // Normal
            fifoSize = std::max(16384, blockSize * 32);
            break;
        case 3: // Extra
            fifoSize = std::max(32768, blockSize * 64);
            break;
        default:
            fifoSize = std::max(16384, blockSize * 32);
            break;
    }
    
    outputFifo = std::make_unique<juce::AbstractFifo>(fifoSize);
    fifoBuffer.resize(static_cast<size_t>(fifoSize * numChannels));
    
    processor->clear();
}

void SoundTouchWrapper::setPitch(float semitones)
{
    const float nativeValue = semitonesToNative(semitones);
    processor->setPitch(nativeValue);
    DBG("Set pitch: " << semitones << " semitones -> native: " << nativeValue);
}

void SoundTouchWrapper::setTempo(float percentage)
{
    const float nativeValue = percentageToNative(percentage);
    processor->setTempo(nativeValue);
}

void SoundTouchWrapper::setRate(float percentage)
{
    const float nativeValue = percentageToNative(percentage);
    processor->setRate(nativeValue);
}

void SoundTouchWrapper::processBlock(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    if (numChannels != currentNumChannels)
    {
        jassertfalse;
        return;
    }
    
    // Resize buffers if needed
    const size_t requiredSize = static_cast<size_t>(numSamples * numChannels);
    if (interleavedBuffer.size() < requiredSize * 2)
    {
        interleavedBuffer.resize(requiredSize * 2);
    }
    
    // Step 1: Interleave and feed input samples to SoundTouch
    for (int sample = 0; sample < numSamples; ++sample)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            interleavedBuffer[static_cast<size_t>(sample * numChannels + channel)] = 
                buffer.getSample(channel, sample);
        }
    }
    
    processor->putSamples(interleavedBuffer.data(), static_cast<uint>(numSamples));
    
    // Step 2: Receive all available samples from SoundTouch and add to FIFO
    std::vector<float> tempBuffer(requiredSize * 2); // Temporary buffer for receiving
    
    while (processor->numSamples() > 0)
    {
        const int received = static_cast<int>(
            processor->receiveSamples(tempBuffer.data(), static_cast<uint>(numSamples * 2))
        );
        
        if (received == 0)
            break;
            
        // Add received samples to FIFO
        const int samplesInFifo = outputFifo->getFreeSpace() / numChannels;
        const int samplesToWrite = std::min(received, samplesInFifo);
        
        if (samplesToWrite > 0)
        {
            int start1, size1, start2, size2;
            outputFifo->prepareToWrite(samplesToWrite * numChannels, start1, size1, start2, size2);
            
            // Copy samples to FIFO buffer
            if (size1 > 0)
            {
                std::copy(tempBuffer.begin(), 
                         tempBuffer.begin() + size1, 
                         fifoBuffer.begin() + start1);
            }
            if (size2 > 0)
            {
                std::copy(tempBuffer.begin() + size1,
                         tempBuffer.begin() + size1 + size2,
                         fifoBuffer.begin() + start2);
            }
            
            outputFifo->finishedWrite(samplesToWrite * numChannels);
        }
    }
    
    // Step 3: Read samples from FIFO to output buffer
    // IMPORTANT: Only process if we have enough samples to fill the entire buffer
    // Otherwise, pass through the dry signal to avoid dropouts
    const int availableInFifo = outputFifo->getNumReady() / numChannels;
    
    if (availableInFifo >= numSamples)
    {
        // We have enough samples - clear buffer and fill with processed audio
        buffer.clear();
        
        int start1, size1, start2, size2;
        outputFifo->prepareToRead(numSamples * numChannels, start1, size1, start2, size2);
        
        // De-interleave samples from FIFO to output buffer
        int sampleIndex = 0;
        
        // Process first block
        for (int i = 0; i < size1; i += numChannels)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                buffer.setSample(ch, sampleIndex, fifoBuffer[static_cast<size_t>(start1 + i + ch)]);
            }
            sampleIndex++;
        }
        
        // Process second block (if wrapped)
        for (int i = 0; i < size2; i += numChannels)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                buffer.setSample(ch, sampleIndex, fifoBuffer[static_cast<size_t>(start2 + i + ch)]);
            }
            sampleIndex++;
        }
        
        outputFifo->finishedRead(numSamples * numChannels);
    }
    // else: keep the input buffer unchanged (dry signal passes through)
    
    // Debug output
    static int blockCount = 0;
    blockCount++;
    if (blockCount <= 30)
    {
        DBG("Block " << blockCount << ": Put " << numSamples 
            << " samples, FIFO has " << availableInFifo << " samples"
            << ", " << (availableInFifo >= numSamples ? "PROCESSED" : "PASSTHROUGH"));
    }
}

int SoundTouchWrapper::getLatencyInSamples() const
{
    // Total latency includes:
    // 1. Samples waiting in SoundTouch's input buffer
    // 2. Samples waiting in our output FIFO
    const int unprocessedSamples = static_cast<int>(processor->numUnprocessedSamples());
    const int fifoSamples = outputFifo ? (outputFifo->getNumReady() / currentNumChannels) : 0;
    
    return unprocessedSamples + fifoSamples;
}

float SoundTouchWrapper::semitonesToNative(float semitones)
{
    return std::exp(semitones * std::log(2.0f) / 12.0f);
}

float SoundTouchWrapper::percentageToNative(float percentage)
{
    return 1.0f + (percentage / 100.0f);
}

void SoundTouchWrapper::setBufferingMode(int mode)
{
    if (mode < 1 || mode > 3 || mode == bufferingMode)
        return;
        
    bufferingMode = mode;
    
    // Re-initialize FIFO buffer with new size based on buffering mode
    if (currentBlockSize > 0 && currentNumChannels > 0)
    {
        int fifoSize;
        
        switch (bufferingMode)
        {
            case 1: // Minimal - smaller buffer for lower latency
                fifoSize = std::max(4096, currentBlockSize * 8);
                break;
                
            case 2: // Normal - current default
                fifoSize = std::max(16384, currentBlockSize * 32);
                break;
                
            case 3: // Extra - double the normal buffer
                fifoSize = std::max(32768, currentBlockSize * 64);
                break;
                
            default:
                fifoSize = std::max(16384, currentBlockSize * 32);
                break;
        }
        
        outputFifo = std::make_unique<juce::AbstractFifo>(fifoSize);
        fifoBuffer.resize(static_cast<size_t>(fifoSize * currentNumChannels));
        
        // Clear the processor to ensure clean state
        processor->clear();
        
        DBG("Buffering mode changed to " << mode << ", FIFO size: " << fifoSize);
    }
}