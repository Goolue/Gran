#include <utility>

#include "PlayComponent.h"

PlayComponent::PlayComponent(value<File>* file, shared_ptr<AudioFormatManager> formatManagerArg) :
        Thread("Play Component Thread", 0), formatManager(move(formatManagerArg)) {
    addAndMakeVisible(&playBtn);
    playBtn.setEnabled(false);
    playBtn.setBounds(0, 0, 100, 100);
    playBtn.setButtonText("Play");
    playBtn.setColour(TextButton::buttonColourId, Colours::green);
    playBtn.onClick = [&] { playBtnClicked(); };

    file->subscribe([&](const auto& file) {
        const String& fileName = file.getFileName();
        cout << "in file subscribe callback, file: " << fileName << endl;
        unique_ptr<AudioFormatReader> reader(formatManager->createReaderFor(file));
        if (reader != nullptr) {
            playBtn.setEnabled(true);
            numOfChannels = reader->numChannels;
            ReferenceCountedBuffer::Ptr newBuffer = new ReferenceCountedBuffer(fileName,
                                                                               reader->numChannels,
                                                                               (int) reader->lengthInSamples);
            reader->read(newBuffer->getAudioSampleBuffer(), 0, (int) reader->lengthInSamples, 0, true, true);
            currentBuffer = newBuffer;
            buffers.add(newBuffer);

            fileLoaded = true;
        }
    });

    startThread();
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
        if (fileLoaded) {
            addBuffersToQueue();
        }
        checkForBuffersToFree();
//        wait(100);
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
    if (fileLoaded && state == Play && !buffersQueue.empty()) {
//        ReferenceCountedBuffer::Ptr retainedCurrentBuffer(currentBuffer);
//        if (retainedCurrentBuffer == nullptr) {
//            bufferToFill.clearActiveBufferRegion();
//            return;
//        }
//        auto* currentAudioSampleBuffer = retainedCurrentBuffer->getAudioSampleBuffer();
//        auto position = retainedCurrentBuffer->position;
//        auto numInputChannels = currentAudioSampleBuffer->getNumChannels();
//        auto numOutputChannels = bufferToFill.buffer->getNumChannels();
//        auto outputSamplesRemaining = bufferToFill.numSamples;
//        auto outputSamplesOffset = 0;
//        while (outputSamplesRemaining > 0) {
//            auto bufferSamplesRemaining = currentAudioSampleBuffer->getNumSamples() - position;
//            auto samplesThisTime = jmin(outputSamplesRemaining, bufferSamplesRemaining);
//            for (auto channel = 0; channel < numOutputChannels; ++channel) {
//                bufferToFill.buffer->copyFrom(channel,
//                                              bufferToFill.startSample + outputSamplesOffset,
//                                              *currentAudioSampleBuffer,
//                                              channel % numInputChannels,
//                                              position,
//                                              samplesThisTime);
//            }
//            outputSamplesRemaining -= samplesThisTime;
//            outputSamplesOffset += samplesThisTime;
//            position += samplesThisTime;
//            if (position == currentAudioSampleBuffer->getNumSamples()) {
//                position = 0;
//            }
//        }
//        retainedCurrentBuffer->position = position;


        auto retainedCurrentBuffer = getAudioBufferFromQueue();
        if (retainedCurrentBuffer == nullptr) {
            cerr << "queue is empty!" << endl;
            bufferToFill.clearActiveBufferRegion();
            return;
        }
        auto* currentAudioSampleBuffer = retainedCurrentBuffer->getAudioSampleBuffer();
        auto position = retainedCurrentBuffer->position;
        cout << "buff from queue, position now " << position << endl;
        auto numInputChannels = currentAudioSampleBuffer->getNumChannels();
        auto numOutputChannels = bufferToFill.buffer->getNumChannels();
        auto outputSamplesRemaining = bufferToFill.numSamples;
        cout << "bufferToFill size " << outputSamplesRemaining << endl;
        auto outputSamplesOffset = 0;
        while (outputSamplesRemaining > 0) {
            auto bufferSamplesRemaining = currentAudioSampleBuffer->getNumSamples() - position;
            auto samplesThisTime = jmin(outputSamplesRemaining, bufferSamplesRemaining);
            cout << "samples this time " << samplesThisTime << endl;
            for (auto channel = 0; channel < numOutputChannels; ++channel) {
                cout << "copy: dest start " << bufferToFill.startSample + outputSamplesOffset <<
                     ", dest end " << bufferToFill.startSample + outputSamplesOffset + samplesThisTime <<
                     ", dest size " << bufferToFill.buffer->getNumSamples() <<
                     ", outputSamplesRemaining " << outputSamplesRemaining << endl;
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
                cout << "popping new arr, position now " << position << " ref count now " <<
                     retainedCurrentBuffer->getReferenceCount() << endl;
                buffersQueue.pop_front();
                cout << "after pop ref count now " << retainedCurrentBuffer->getReferenceCount() << endl;
                retainedCurrentBuffer = getAudioBufferFromQueue();
                if (retainedCurrentBuffer == nullptr) {
                    cerr << "queue is empty! in while" << endl;
                    bufferToFill.clearActiveBufferRegion();
                    return;
                }
                currentAudioSampleBuffer = retainedCurrentBuffer->getAudioSampleBuffer();
                position = retainedCurrentBuffer->position;
                cout << "new position now " << position << endl;
            }
        }
        cout << "setting position " << position << endl;
        retainedCurrentBuffer->position = position;

//        auto* currentAudioSampleBuffer = getAudioBufferFromQueue();
//        if (currentAudioSampleBuffer == nullptr) {
//            cerr << "queue is empty!" << endl;
//            bufferToFill.clearActiveBufferRegion();
//            return;
//        }
//        int position = 0;
//        auto numOutputChannels = bufferToFill.buffer->getNumChannels();
//        auto outputSamplesRemaining = bufferToFill.numSamples;
//        auto numInputChannels = currentAudioSampleBuffer->getNumChannels();
//        auto outputSamplesOffset = 0;
//        while (outputSamplesRemaining > 0) {
//            auto bufferSamplesRemaining = currentAudioSampleBuffer->getNumSamples() - position;
//            auto samplesThisTime = jmin(outputSamplesRemaining, bufferSamplesRemaining);
//            cout << "samples this time " << samplesThisTime << " bufferSamplesRemaining " << bufferSamplesRemaining
//                 << " outputSamplesRemaining " << outputSamplesRemaining << " num samples " <<
//                 currentAudioSampleBuffer->getNumSamples() << endl;
//            for (auto channel = 0; channel < numOutputChannels; ++channel) {
//                cout << "copying channel " << channel << endl;
//                bufferToFill.buffer->copyFrom(channel,
//                                              bufferToFill.startSample + outputSamplesOffset,
//                                              *currentAudioSampleBuffer,
//                                              channel % numInputChannels,
//                                              position,
//                                              samplesThisTime);
//                cout << "copied channel " << channel << endl;
//            }
//            outputSamplesRemaining -= samplesThisTime;
//            outputSamplesOffset += samplesThisTime;
//            position += samplesThisTime;
//            if (position == currentAudioSampleBuffer->getNumSamples()) {
//                cout << "reseting position" << endl;
//                position = 0;
//                outputSamplesOffset = 0;
//                currentAudioSampleBuffer = getAudioBufferFromQueue();
//                if (currentAudioSampleBuffer == nullptr) {
//                    cerr << "queue is empty! inside while" << endl;
//                    bufferToFill.clearActiveBufferRegion();
//                    return;
//                }
//            }
//        }
    } else {
        bufferToFill.clearActiveBufferRegion();
    }
}

ReferenceCountedBuffer::Ptr PlayComponent::getAudioBufferFromQueue() {
    auto buffFromQueue = buffersQueue.front();
    if (buffFromQueue == nullptr) {
        return nullptr;
    }
//    buffersQueue.pop_front();
    cout << "popped from queue. queue size is now " << buffersQueue.size() << endl;
    return buffFromQueue;
//    auto* currentAudioSampleBuffer = buffFromQueue->getAudioSampleBuffer();
//    return currentAudioSampleBuffer;
}

void PlayComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    this->samplesPerBlockExpected = samplesPerBlockExpected;
    this->sampleRate = sampleRate;

}

void PlayComponent::addBuffersToQueue() {
    if (fileLoaded && buffersQueue.size() < MAX_QUEUE_SIZE) {
        cout << "starting add to queue" << endl;
        ReferenceCountedBuffer::Ptr retainedCurrentBuffer(currentBuffer);
        if (retainedCurrentBuffer == nullptr) {
            cout << "retained buffer is nullptr" << endl;
            return;
        }
        auto currentAudioSampleBuffer = retainedCurrentBuffer->getAudioSampleBuffer();
        auto position = retainedCurrentBuffer->position;

        while (buffersQueue.size() < MAX_QUEUE_SIZE) {
            cout << "starting push to queue. queue size is now " << buffersQueue.size() << endl;

            ReferenceCountedBuffer::Ptr buffToPush = new ReferenceCountedBuffer("buffToPush" + String(currBuffIndex),
                                                                                numOfChannels,
                                                                                samplesPerBlockExpected);
            auto audioSampleBufferToPush = buffToPush->getAudioSampleBuffer();

            currBuffIndex++;
            auto outputSamplesOffset = 0;
            while (outputSamplesOffset < audioSampleBufferToPush->getNumSamples()) {
                auto bufferSamplesRemaining = currentAudioSampleBuffer->getNumSamples() - position;
                auto samplesThisTime = jmin(samplesPerBlockExpected - outputSamplesOffset, bufferSamplesRemaining);
                cout << "push samples this time " << samplesThisTime <<
                    " push bufferSamplesRemaining " << bufferSamplesRemaining
                     << " push outputSamplesOffset " << outputSamplesOffset << " push num samples " <<
                     currentAudioSampleBuffer->getNumSamples() << endl;
                for (int channel = 0; channel < numOfChannels; channel++) {
                    cout << "push copy: dest start " << outputSamplesOffset <<
                         ", dest end " << samplesThisTime + outputSamplesOffset  <<
                         ", dest size " << audioSampleBufferToPush->getNumSamples() << endl;
                    audioSampleBufferToPush->copyFrom(channel, outputSamplesOffset, *currentAudioSampleBuffer,
                                                      channel, position, samplesThisTime);
                }
                outputSamplesOffset += samplesThisTime;
                position += samplesThisTime;
                if (position >= currentAudioSampleBuffer->getNumSamples()) {
                    position = 0;
                }
            }
            retainedCurrentBuffer->position = position;
            buffersQueue.push_back(buffToPush);

            cout << "pushed to queue. queue size is now " << buffersQueue.size() << " pushed buff size " <<
                 audioSampleBufferToPush->getNumSamples() << endl;
        }
    }
}
