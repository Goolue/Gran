/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
enum PlayState {
    Play,
    Stop,
    Pause
};

/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public AudioAppComponent, public ChangeListener, private Timer {
public:
    //==============================================================================
    MainComponent();

    ~MainComponent();

    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    void getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill) override;

    void releaseResources() override;

    //==============================================================================
    void paint(Graphics &g) override;

    void resized() override;

    void changeListenerCallback(ChangeBroadcaster *source) override;

private:
    // Your private member variables go here...

    //==============================================================================
    void openFileBtnClicked();
    void playBtnClicked();
    void changeState(PlayState newState);

    void paintThumbnailIfFileWasLoaded(Graphics &g, const Colour &backgroundColour, const Rectangle<int> &thumbnailBounds);

    //==============================================================================
    TextButton openFileBtn;
    ToggleButton playBtn;

    AudioFormatManager formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    PlayState state;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)

    void timerCallback() override;
};