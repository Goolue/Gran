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
#include <observable/observable.hpp>
#include "PlayComponent.h"

using namespace std;
using namespace observable;

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public AudioAppComponent {
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
//    void playBtnClicked();
//    void checkForBuffersToFree();
//    void changeState(PlayState newState);
//    void run() override;

    //==============================================================================
    shared_ptr<AudioFormatManager> formatManager;

    ThumbnailComponent thumbnailComponent;
    infinite_subscription subscription;

    PlayComponent playComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
