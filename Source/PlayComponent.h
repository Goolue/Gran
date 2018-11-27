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
    void setupSlider(Slider& slider, Component& toPutNextTo, const string& name, double value,
                         function<void()> onValueChange);
    void splitFileToGrains();

    // vars
    const int MAX_QUEUE_SIZE = 10;
    deque<ReferenceCountedBuffer::Ptr> buffersQueue{};
    int currBuffNum = 0;

    const int SLIDER_WIDTH = 100;
    const int SLIDER_HIGHT = 100;
    const int GAP_SIZE = 5;
    TextButton playBtn;
    Slider gainSlider{Slider::SliderStyle::Rotary, Slider::NoTextBox};
    double currGain = 0.5;
    Slider startSlider{Slider::SliderStyle::Rotary, Slider::NoTextBox};
    Slider endSlider{Slider::SliderStyle::Rotary, Slider::NoTextBox};
    const int MIN_LEN_GAP = 100;
    Slider grainSizeSlider{Slider::SliderStyle::Rotary, Slider::NoTextBox};
    const int MAX_GRAIN_SIZE = 1000;
    const int MIN_GRAIN_SIZE = 100;
    int grainSize = (MAX_GRAIN_SIZE + MIN_GRAIN_SIZE) / 2;
    int currGrainIndex = 0;
    vector<ReferenceCountedBuffer::Ptr> grainVec{};


    PlayState state{Stop};
    bool fileLoaded = false;

    shared_ptr<AudioFormatManager> formatManager;
    ReferenceCountedArray<ReferenceCountedBuffer> buffers{};
    ReferenceCountedBuffer::Ptr currentBuffer;

    int samplesPerBlockExpected;
    double sampleRate;
    int numOfChannels;
    int64 fileNumSamples;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayComponent)
};


#endif //GRAN_PLAYCOMPONENT_H
