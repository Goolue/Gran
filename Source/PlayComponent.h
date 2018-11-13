#ifndef GRAN_PLAYCOMPONENT_H
#define GRAN_PLAYCOMPONENT_H

#include "JuceHeader.h"
#include "PlayState.h"
#include "ReferenceCountedBuffer.h"
#include <observable/observable.hpp>

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

private:
    TextButton playBtn;
    PlayState state{Stop};
    bool fileLoaded = false;

    shared_ptr<AudioFormatManager> formatManager;
    ReferenceCountedArray<ReferenceCountedBuffer> buffers{};
    ReferenceCountedBuffer::Ptr currentBuffer;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayComponent)
};


#endif //GRAN_PLAYCOMPONENT_H
