#include <utility>

//
// Created by goolue on 11/10/18.
//

#ifndef GRAN_REFERENCECOUNTEDBUFFER_H
#define GRAN_REFERENCECOUNTEDBUFFER_H

#endif //GRAN_REFERENCECOUNTEDBUFFER_H
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class ReferenceCountedBuffer : public ReferenceCountedObject {
public:
    typedef ReferenceCountedObjectPtr<ReferenceCountedBuffer> Ptr;

    ReferenceCountedBuffer(String nameToUse, int numChannels, int numSamples)
            : name(std::move(nameToUse)), buffer(numChannels, numSamples) {
        DBG (String("Buffer named '") + name + "' constructed. numChannels = " + String(numChannels) +
             ", numSamples = " + String(numSamples));
    }

    ~ReferenceCountedBuffer() {
        DBG (String("Buffer named '") + name + "' destroyed");
    }

    AudioSampleBuffer *getAudioSampleBuffer() {
        return &buffer;
    }

    int position = 0;
private:
    String name;
    AudioSampleBuffer buffer;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReferenceCountedBuffer)
};
