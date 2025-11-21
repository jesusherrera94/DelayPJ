/*
  ==============================================================================

    DelayLine.h
    Created: 21 Nov 2025 4:57:14pm
    Author:  Jesus Herrera

  ==============================================================================
*/

#pragma once

#include <memory>

class DelayLine {
    public:
    void setMaximunDelayInSamples(int maxLengthInSamples);
    void reset() noexcept;
    void write(float input) noexcept;
    float read(float delayInSamples) const noexcept;
    int getBufferLength() const noexcept {
        return bufferLength;
    }
    private:
    std::unique_ptr<float[]> buffer;
    int bufferLength = 0;
    int writeIndex = 0; 

};
