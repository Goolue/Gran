/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() : Thread("MainThread", 0), thumbnailCache(5),
                                 thumbnail(1024, formatManager, thumbnailCache), state(Stop), buffers() {
    // Make sure you set the size of the component after
    // you add any child components.
    setSize(800, 600);

    formatManager.registerBasicFormats();

    addAndMakeVisible(&openFileBtn);
    openFileBtn.setButtonText("Open file");
    openFileBtn.setVisible(true);
    openFileBtn.onClick = [this] { openFileBtnClicked(); };

    addAndMakeVisible(&playBtn);
    playBtn.setButtonText("Play / Pause");
    playBtn.setVisible(true);
    playBtn.setColour(ToggleButton::tickColourId, Colours::green);
    playBtn.setColour(ToggleButton::tickDisabledColourId, Colours::red);
    playBtn.setEnabled(false);
    playBtn.onClick = [this] { playBtnClicked(); };
    playBtn.setBounds(openFileBtn.getRight() + 5, openFileBtn.getY(), 100, 100);

    thumbnail.addChangeListener(this);
}

void MainComponent::run() {
    while (!threadShouldExit()) {
        checkForBuffersToFree();
        wait(500);
    }

}

void MainComponent::checkForBuffersToFree() {
    for (int i = buffers.size() - 1; i > 0; --i) {
        ReferenceCountedBuffer::Ptr buffer(buffers.getUnchecked(i));
        if (buffer->getReferenceCount() == 2) {
            buffers.remove(i);
        }
    }
}

void MainComponent::changeListenerCallback(ChangeBroadcaster *source) {
    if (source == &thumbnail) repaint();
}

MainComponent::~MainComponent() {
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()

}

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill) {
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)

    if (fileLoaded && state == Play) {
        ReferenceCountedBuffer::Ptr retainedCurrentBuffer(currentBuffer);
        if (retainedCurrentBuffer == nullptr) {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
        auto *currentAudioSampleBuffer = retainedCurrentBuffer->getAudioSampleBuffer();
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

void MainComponent::releaseResources() {
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint(Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    const Colour &backgroundColour = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    g.fillAll(backgroundColour);

    // You can add your drawing code here!

    Rectangle<int> thumbnailBounds(10, 100, getWidth() - 20, getHeight() - 120);
    if (thumbnail.getNumChannels() > 0) {
        paintThumbnailIfFileWasLoaded(g, backgroundColour, thumbnailBounds);
    } else {
        g.drawFittedText("No file was loaded!", thumbnailBounds, Justification::centred, 2);
    }
}

void MainComponent::paintThumbnailIfFileWasLoaded(Graphics &g, const Colour &backgroundColour,
                                                  const Rectangle<int> &thumbnailBounds) {
    g.setColour(backgroundColour);
    auto audioLength(thumbnail.getTotalLength());
    g.setColour(Colours::red);
    thumbnail.drawChannels(g, thumbnailBounds, 0.0, audioLength, 1.0f);
}

void MainComponent::resized() {
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    openFileBtn.setBounds(0, 0, 100, 100);

}

void MainComponent::openFileBtnClicked() {
    playBtn.setEnabled(true);
    FileChooser chooser("Select a Wave file to play...",
                        File::getSpecialLocation(File::currentExecutableFile), "*.wav");
    if (chooser.browseForFileToOpen()) {
        File file(chooser.getResult());
        std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor (file));
        if (reader != nullptr) {
            ReferenceCountedBuffer::Ptr newBuffer = new ReferenceCountedBuffer(file.getFileName(),
                                                                               reader->numChannels,
                                                                               (int) reader->lengthInSamples);
            reader->read(newBuffer->getAudioSampleBuffer(), 0, (int) reader->lengthInSamples, 0, true, true);
            currentBuffer = newBuffer;
            buffers.add(newBuffer);
            thumbnail.setSource(new FileInputSource(file));
            fileLoaded = true;
            setAudioChannels(0, reader->numChannels);
        }
    }
    fileLoaded = true;
}

void MainComponent::playBtnClicked() {
    std::cout << "playBtnClicked " << "state is now " << playBtn.getToggleState() << std::endl;
    if (playBtn.getToggleState()) {
        changeState(Play);
    } else changeState(Stop);
}

void MainComponent::changeState(PlayState newState) {
    std::cout << "state is " << state << " changing state to " << newState << std::endl;
    state = newState;
}
