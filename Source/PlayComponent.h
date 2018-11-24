#ifndef GRAN_PLAYCOMPONENT_H
#define GRAN_PLAYCOMPONENT_H

#include "JuceHeader.h"
#include "PlayState.h"
#include "ReferenceCountedBuffer.h"
#include <observable/observable.hpp>
#include <queue>

using namespace std;
using namespace observable;

class PlayComponent : public Component, private Thread {
    OBSERVABLE_PROPERTIES(PlayComponent)

public:
    explicit PlayComponent(value<File>* file, shared_ptr<AudioFormatManager> formatManager);
    ~PlayComponent() override;


    void playBtnClicked();
    void checkForBuffersToFree();
    void changeState(PlayState newState);
    void run() override;

    void getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill);
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);

    observable_property<int> startVal;
    observable_property<int> endVal;

private:
    // methods
    void addBuffersToQueue();
    ReferenceCountedBuffer::Ptr getAudioBufferFromQueue();

    // vars
    const int MAX_QUEUE_SIZE = 5;
    deque<ReferenceCountedBuffer::Ptr> buffersQueue{};
    int currBuffIndex = 0;

    TextButton playBtn;
    Slider gainSlider{Slider::SliderStyle::Rotary, Slider::NoTextBox};
    double currGain = 0.5;
    Slider startSlider{Slider::SliderStyle::Rotary, Slider::NoTextBox};
    Slider endSlider{Slider::SliderStyle::Rotary, Slider::NoTextBox};
    const int MIN_LEN_GAP = 100;

    PlayState state{Stop};
    bool fileLoaded = false;

    shared_ptr<AudioFormatManager> formatManager;
    ReferenceCountedArray<ReferenceCountedBuffer> buffers{};
    ReferenceCountedBuffer::Ptr currentBuffer;

    int samplesPerBlockExpected;
    double sampleRate;
    int numOfChannels;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayComponent)
};


#endif //GRAN_PLAYCOMPONENT_H
