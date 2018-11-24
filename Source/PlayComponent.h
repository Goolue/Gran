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
    OBSERVABLE_PROPERTIES(Component)

public:
    explicit PlayComponent(value<File>* file, shared_ptr<AudioFormatManager> formatManager);
    ~PlayComponent() override;


    void playBtnClicked();
    void checkForBuffersToFree();
    void changeState(PlayState newState);
    void run() override;

    void getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill);
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);

private:
    // methods
    void addBuffersToQueue();
    ReferenceCountedBuffer::Ptr getAudioBufferFromQueue();

    // vars
    const int MAX_QUEUE_SIZE = 5;
    deque<ReferenceCountedBuffer::Ptr> buffersQueue{};
    int currBuffIndex = 0;

    TextButton playBtn;
    Slider gainSlider;
    double currGain = 0.5;

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
