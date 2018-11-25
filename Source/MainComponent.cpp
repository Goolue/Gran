/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"
#include "future"

using namespace std;

//==============================================================================
MainComponent::MainComponent() : formatManager(new AudioFormatManager),
                                 thumbnailComponent(formatManager),
                                 playComponent(&thumbnailComponent.file, formatManager) {
    // Make sure you set the size of the component after
    // you add any child components.
    setSize(800, 600);

    formatManager->registerBasicFormats();

    addAndMakeVisible(&playComponent);
    playComponent.setBounds(0, 0, 500, 100);

    addAndMakeVisible(&thumbnailComponent);
    thumbnailComponent.setVisible(true);
    thumbnailComponent.setBounds(playComponent.getX(), playComponent.getBottom() + 5, 780, 400);
    thumbnailComponent.subscribeStartAndEnd(playComponent.startVal, playComponent.endVal);

    subscriptions.emplace_back(thumbnailComponent.file.subscribe([&](auto const& file) {
        unique_ptr<AudioFormatReader> reader(formatManager->createReaderFor(file));
        if (reader != nullptr) {
            setAudioChannels(0, reader->numChannels);
        }
    }));
}

//void MainComponent::run() {
//    while (!threadShouldExit()) {
//        checkForBuffersToFree();
//        wait(500);
//    }
//
//}

//void MainComponent::checkForBuffersToFree() {
//    for (int i = buffers.size() - 1; i > 0; --i) {
//        ReferenceCountedBuffer::Ptr buffer(buffers.getUnchecked(i));
//        if (buffer->getReferenceCount() == 2) {
//            buffers.remove(i);
//        }
//    }
//}

MainComponent::~MainComponent() {
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
//    stopThread(1000);

    subscriptions.clear();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()

    playComponent.prepareToPlay(samplesPerBlockExpected, sampleRate);
    thumbnailComponent.setSampleRate(sampleRate);

}

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) {
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)

    playComponent.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources() {
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint(Graphics& g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    const Colour& backgroundColour = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    g.fillAll(backgroundColour);

    // You can add your drawing code here!
}

void MainComponent::resized() {
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

//void MainComponent::playBtnClicked() {
//    cout << "playBtnClicked " << "state is now " << playBtn.getToggleState() << endl;
//    if (playBtn.getToggleState()) {
//        changeState(Play);
//    } else changeState(Stop);
//}

//void MainComponent::changeState(PlayState newState) {
//    cout << "state is " << state << " changing state to " << newState << endl;
//    state = newState;
//}
