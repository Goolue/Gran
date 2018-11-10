/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() : thumbnailCache(5), thumbnail(1024, formatManager, thumbnailCache), state(Stop) {
    // Make sure you set the size of the component after
    // you add any child components.
    setSize(800, 600);

    // specify the number of input and output channels that we want to open
    setAudioChannels(0, 2);
    startTimer(40);

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
    transportSource.addChangeListener(this);
    transportSource.setGain(0.5f);
}

void MainComponent::changeListenerCallback(ChangeBroadcaster *source) {
    if (source == &thumbnail) repaint();
    else if (source == &transportSource) {
        if (transportSource.getCurrentPosition() == transportSource.getLengthInSeconds()) {
            transportSource.setPosition(0);
        }
    }
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
//    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

}

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill) {
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)

    if (fileLoaded && state == Play) {
        auto numInputChannels = fileBuffer.getNumChannels();
        auto numOutputChannels = bufferToFill.buffer->getNumChannels();
        auto outputSamplesRemaining = bufferToFill.numSamples;
        auto outputSamplesOffset = bufferToFill.startSample;
        while (outputSamplesRemaining > 0) {
            auto bufferSamplesRemaining = fileBuffer.getNumSamples() - position;
            auto samplesThisTime = jmin(outputSamplesRemaining, bufferSamplesRemaining);
            for (auto channel = 0; channel < numOutputChannels; ++channel) {
                bufferToFill.buffer->copyFrom(channel, outputSamplesOffset, fileBuffer, channel % numInputChannels,
                                              position, samplesThisTime);
            }
            outputSamplesRemaining -= samplesThisTime;
            outputSamplesOffset += samplesThisTime;
            position += samplesThisTime;
            if (position == fileBuffer.getNumSamples()) position = 0;
        }
    }
    else {
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
//    g.setColour(Colours::green);
//    auto audioPosition(position);
//    auto drawPosition = (float) ((audioPosition / audioLength) * thumbnailBounds.getWidth() + thumbnailBounds.getX());
//    g.drawLine(drawPosition, thumbnailBounds.getY(), drawPosition, thumbnailBounds.getBottom(), 2.0f);

}

void MainComponent::resized() {
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    openFileBtn.setBounds(0, 0, 100, 100);

}

void MainComponent::openFileBtnClicked() {
    shutdownAudio();

    playBtn.setEnabled(true);
    FileChooser chooser("Select a Wave file to play...",
                        File::getSpecialLocation(File::currentExecutableFile), "*.wav");
    if (chooser.browseForFileToOpen()) {
        File file(chooser.getResult());
        auto *reader = formatManager.createReaderFor(file);
        if (reader != nullptr) {
            fileBuffer.setSize(reader->numChannels, static_cast<int>(reader->lengthInSamples));
            reader->read(&fileBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
            std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
//            transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
//            transportSource.setLooping(true);
//            std::cout << "source len is: " << transportSource.getLengthInSeconds() << std::endl;
            thumbnail.setSource(new FileInputSource(file));
            readerSource = std::move(newSource);

            position = 0;
            setAudioChannels(0, reader->numChannels); // also starts the audio again
        }
    }
    fileLoaded = true;
}

void MainComponent::playBtnClicked() {
    std::cout << "playBtnClicked " << "state is now " << playBtn.getToggleState() << std::endl;
    if (playBtn.getToggleState()) {
        changeState(Stop);
    } else changeState(Play);
}

void MainComponent::changeState(PlayState newState) {
    if (state != newState) {
        state = newState;
        switch (newState) {
            case Stop:
            case Pause:
                //TODO
//                std::cout << "starting " << transportSource.getCurrentPosition() << std::endl;
//                transportSource.start();
                break;
            case Play:
                //TODO
//                std::cout << "stopping " << transportSource.getCurrentPosition() << std::endl;
//                transportSource.stop();
                break;
        }
    }
}

void MainComponent::timerCallback() {
    repaint();
}
