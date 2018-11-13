#include <utility>

//
// Created by goolue on 11/12/18.
//

#include "PlayComponent.h"

PlayComponent::PlayComponent(value<File>* file, shared_ptr<AudioFormatManager> formatManagerArg) :
        Thread("Play Component Thread", 0), formatManager(move(formatManagerArg)) {
    addAndMakeVisible(&playBtn);
    playBtn.setEnabled(false);
    playBtn.setBounds(0, 0, 100, 100);
    playBtn.setButtonText("Play");
    playBtn.setColour(TextButton::buttonColourId, Colours::green);
    playBtn.onClick = [&]{playBtnClicked();};

    file->subscribe([&](const auto& file) {
        const String& fileName = file.getFileName();
        cout << "in file subscribe callback, file: " << fileName << endl;
        unique_ptr<AudioFormatReader> reader(formatManager->createReaderFor(file));
        if (reader != nullptr) {
            fileLoaded = true;
            playBtn.setEnabled(true);
            ReferenceCountedBuffer::Ptr newBuffer = new ReferenceCountedBuffer(fileName,
                                                                               reader->numChannels,
                                                                               (int) reader->lengthInSamples);
            reader->read(newBuffer->getAudioSampleBuffer(), 0, (int) reader->lengthInSamples, 0, true, true);
            currentBuffer = newBuffer;
            buffers.add(newBuffer);
//            setAudioChannels(0, reader->numChannels);
        }
    });
}

PlayComponent::~PlayComponent() {
    state = Stop;
    stopThread(1000);

    checkForBuffersToFree();
}

void PlayComponent::playBtnClicked() {
    cout << "playBtnClicked " << "state is now " << playBtn.getToggleState() << endl;
    if (state == Stop || state == Pause) {
        changeState(Play);
    } else changeState(Pause);
}

void PlayComponent::changeState(PlayState newState) {
    state = newState;

    switch (newState) {
        case Play:
            playBtn.setColour(TextButton::buttonColourId, Colours::red);
            playBtn.setButtonText("Pause");
            break;
        case Pause:
            playBtn.setColour(TextButton::buttonColourId, Colours::green);
            playBtn.setButtonText("Play");
            break;
        default:
            cerr << "change state called with" << newState << endl;
            break;
    }

}

void PlayComponent::run() {
    while (!threadShouldExit()) {
        checkForBuffersToFree();
        wait(500);
    }
}

void PlayComponent::checkForBuffersToFree() {
    for (int i = buffers.size() - 1; i > 0; --i) {
        ReferenceCountedBuffer::Ptr buffer(buffers.getUnchecked(i));
        if (buffer->getReferenceCount() == 2) {
            buffers.remove(i);
        }
    }
}

void PlayComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) {
    if (fileLoaded && state == Play) {
        ReferenceCountedBuffer::Ptr retainedCurrentBuffer(currentBuffer);
        if (retainedCurrentBuffer == nullptr) {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
        auto* currentAudioSampleBuffer = retainedCurrentBuffer->getAudioSampleBuffer();
        auto position = retainedCurrentBuffer->position;
        auto numInputChannels = currentAudioSampleBuffer->getNumChannels();
        auto numOutputChannels = bufferToFill.buffer->getNumChannels();
        auto outputSamplesRemaining = bufferToFill.numSamples;
        auto outputSamplesOffset = 0;
        while (outputSamplesRemaining > 0) {
            auto bufferSamplesRemaining = currentAudioSampleBuffer->getNumSamples() - position;
            auto samplesThisTime = jmin(outputSamplesRemaining, bufferSamplesRemaining);
            for (auto channel = 0; channel < numOutputChannels; ++channel) {
                bufferToFill.buffer->copyFrom(channel,
                                              bufferToFill.startSample + outputSamplesOffset,
                                              *currentAudioSampleBuffer,
                                              channel % numInputChannels,
                                              position,
                                              samplesThisTime);
            }
            outputSamplesRemaining -= samplesThisTime;
            outputSamplesOffset += samplesThisTime;
            position += samplesThisTime;
            if (position == currentAudioSampleBuffer->getNumSamples()) {
                position = 0;
            }
        }
        retainedCurrentBuffer->position = position;
    } else {
        bufferToFill.clearActiveBufferRegion();
    }
}
