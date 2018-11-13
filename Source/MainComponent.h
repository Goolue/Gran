/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ReferenceCountedBuffer.h"
#include "PlayState.h"
#include "ThumbnailComponent.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public AudioAppComponent, private Thread {
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

private:
    // Your private member variables go here...

    //==============================================================================
//    void changeState(PlayState newState);
    void playBtnClicked();
    void checkForBuffersToFree();
    void changeState(PlayState newState);
    void run() override;

    //==============================================================================
    ToggleButton playBtn;
    PlayState state;
    bool fileLoaded = false;

    shared_ptr<AudioFormatManager> formatManager;
    ReferenceCountedArray<ReferenceCountedBuffer> buffers;
    ReferenceCountedBuffer::Ptr currentBuffer;

    ThumbnailComponent thumbnailComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
