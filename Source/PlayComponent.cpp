#include <utility>

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

    setupSlider(gainSlider, playBtn, "Gain", currGain, [&]() {
        currGain = gainSlider.getValue();
    });

    setupSlider(startSlider, gainSlider, "Start", 0, [&]() {
        startVal = static_cast<int>(startSlider.getValue());
        if (endVal.get() < startVal.get() + MIN_LEN_GAP) {
            endVal = startVal.get() + MIN_LEN_GAP;
            endSlider.setValue(endVal.get());
        }
    });

    setupSlider(endSlider, startSlider, "End", 1, [&]() {
        endVal = static_cast<int>(endSlider.getValue());
        if (endVal.get() < startVal.get() + MIN_LEN_GAP) {
            startVal = endVal.get() - MIN_LEN_GAP;
            startSlider.setValue(startVal.get());
        }
    });

    setupSlider(grainSizeSlider, endSlider, "Grain size", grainSize, [&]() {
       grainSize = static_cast<int>(grainSizeSlider.getValue());
       splitFileToGrains();
    });
    grainSizeSlider.setRange(MIN_GRAIN_SIZE, MAX_GRAIN_SIZE, 10);
    grainSizeSlider.setValue(grainSize);


    file->subscribe([&](const auto& file) {
        const String& fileName = file.getFileName();
        cout << "in file subscribe callback, file: " << fileName << endl;
        unique_ptr<AudioFormatReader> reader(formatManager->createReaderFor(file));
        if (reader != nullptr) {
            playBtn.setEnabled(true);
            gainSlider.setEnabled(true);
            startSlider.setEnabled(true);
            endSlider.setEnabled(true);
            startSlider.setRange(0, reader->lengthInSamples - MIN_LEN_GAP, 1);
            startVal = (int) startSlider.getValue();
            endSlider.setRange(MIN_LEN_GAP, reader->lengthInSamples, 1);
            endSlider.setValue(reader->lengthInSamples);
            endVal = (int) endSlider.getValue();
            grainSizeSlider.setEnabled(true);

            numOfChannels = reader->numChannels;
            fileNumSamples = reader->lengthInSamples;
            ReferenceCountedBuffer::Ptr newBuffer = new ReferenceCountedBuffer(fileName,
                                                                               reader->numChannels,
                                                                               (int) reader->lengthInSamples);
            reader->read(newBuffer->getAudioSampleBuffer(), 0, (int) reader->lengthInSamples, 0, true, true);
            currentBuffer = newBuffer;
            buffers.add(newBuffer);

            splitFileToGrains();
            fileLoaded = true;
        }
    });

    setSize(playBtn.getWidth() + 5 * 4 + 4 * SLIDER_WIDTH , playBtn.getHeight());

    startThread();
}

void PlayComponent::setupSlider(Slider& slider, Component& toPutNextTo, const string& name, const double value,
                                function<void()> onValueChange) {
    addAndMakeVisible(&slider);
    slider.setEnabled(false);
    slider.setName(name);
    slider.setBounds(toPutNextTo.getRight() + GAP_SIZE, toPutNextTo.getY(), SLIDER_WIDTH, SLIDER_HIGHT);
    slider.setTextBoxIsEditable(false);
    slider.setPopupDisplayEnabled(true, true, this);
    slider.setRange(0, 1, 0.001);
    slider.setValue(value);
    slider.onValueChange = std::move(onValueChange);
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
        auto retainedCurrentBuffer = getAudioBufferFromQueue();
        if (retainedCurrentBuffer == nullptr) {
            cerr << "queue is empty!" << endl;
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
                buffersQueue.pop_front();
                retainedCurrentBuffer = getAudioBufferFromQueue();
                if (retainedCurrentBuffer == nullptr) {
                    bufferToFill.clearActiveBufferRegion();
                    return;
                }
                currentAudioSampleBuffer = retainedCurrentBuffer->getAudioSampleBuffer();
                position = retainedCurrentBuffer->position;
            }
        }
        cout << "setting position " << position << endl;
        retainedCurrentBuffer->position = position;

    } else {
        bufferToFill.clearActiveBufferRegion();
    }
}

ReferenceCountedBuffer::Ptr PlayComponent::getAudioBufferFromQueue() {
    auto buffFromQueue = buffersQueue.front();
    if (buffFromQueue == nullptr) {
        return nullptr;
    }
    return buffFromQueue;
}

void PlayComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    this->samplesPerBlockExpected = samplesPerBlockExpected;
    this->sampleRate = sampleRate;

}

void PlayComponent::addBuffersToQueue() {
    if (fileLoaded && !grainVec.empty() && buffersQueue.size() < MAX_QUEUE_SIZE) {
        ReferenceCountedBuffer::Ptr retainedCurrentBuffer(currentBuffer);
        if (retainedCurrentBuffer == nullptr) {
            cout << "retained buffer is nullptr" << endl;
            return;
        }
        auto currentAudioSampleBuffer = retainedCurrentBuffer->getAudioSampleBuffer();
        auto position = jmax(retainedCurrentBuffer->position, startVal.get());

        while (buffersQueue.size() < MAX_QUEUE_SIZE) {

            ReferenceCountedBuffer::Ptr buffToPush = new ReferenceCountedBuffer("buffToPush" + String(currBuffNum),
                                                                                numOfChannels,
                                                                                grainSize);
            auto audioSampleBufferToPush = buffToPush->getAudioSampleBuffer();

            currBuffNum++;
            auto outputSamplesOffset = 0;
            auto maxSample = jmin(endVal.get(), currentAudioSampleBuffer->getNumSamples());
            while (outputSamplesOffset < audioSampleBufferToPush->getNumSamples()) {
                auto bufferSamplesRemaining = currentAudioSampleBuffer->getNumSamples() - position;
                auto samplesThisTime = jmin(grainSize - outputSamplesOffset, bufferSamplesRemaining);
                cout << "samples this time " << samplesThisTime << endl;
                for (int channel = 0; channel < numOfChannels; channel++) {
                    audioSampleBufferToPush->copyFrom(channel, outputSamplesOffset, *currentAudioSampleBuffer,
                                                      channel, position, samplesThisTime);
                    audioSampleBufferToPush->applyGain(static_cast<float>(currGain));
                }
                outputSamplesOffset += samplesThisTime;
                position += samplesThisTime;
                if (position >= maxSample) {
                    position = startVal.get();
                }
            }
            retainedCurrentBuffer->position = position;
            buffersQueue.push_back(buffToPush);

            cout << "pushed to queue. queue size is now " << buffersQueue.size() << " pushed buff size " <<
                 audioSampleBufferToPush->getNumSamples() << endl;
        }
    }
}

void PlayComponent::splitFileToGrains() {
    vector<ReferenceCountedBuffer::Ptr> vec{};
    ReferenceCountedBuffer::Ptr retainedCurrentBuffer(currentBuffer);
    if (retainedCurrentBuffer == nullptr) {
        cout << "retained buffer is nullptr" << endl;
        return;
    }
    auto currentAudioSampleBuffer = retainedCurrentBuffer->getAudioSampleBuffer();
    int index = 0;
    int samplesLeft = currentAudioSampleBuffer->getNumSamples();
    int buffersCount = 0;
    while (samplesLeft > 0) {
        int samplesThisTime = jmin(grainSize, samplesLeft);
        ReferenceCountedBuffer::Ptr buffToPush = new ReferenceCountedBuffer("grainBuff" + String(buffersCount),
                                                                            numOfChannels,
                                                                            samplesThisTime);
        auto audioSampleBufferToPush = buffToPush->getAudioSampleBuffer();
        for (int channel = 0; channel < numOfChannels; channel++) {
            audioSampleBufferToPush->copyFrom(channel, 0, *currentAudioSampleBuffer,
                                              channel, index, samplesThisTime);
            vec.push_back(buffToPush);
            samplesLeft -= samplesThisTime;
            index += samplesThisTime;
        }
    }
    swap(vec, grainVec);

}
