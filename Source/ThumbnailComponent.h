#ifndef GRAN_THUMBNAILCOMPONENT_H
#define GRAN_THUMBNAILCOMPONENT_H
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PlayState.h"
#include <observable/observable.hpp>

using namespace std;
using namespace observable;

class ThumbnailComponent : public Component, public ChangeListener {

    OBSERVABLE_PROPERTIES(ThumbnailComponent)

public:
    // methods:
    explicit ThumbnailComponent(shared_ptr<AudioFormatManager> formatManager);
    ~ThumbnailComponent() override;

    void paint(Graphics &g) override;
    void changeListenerCallback(ChangeBroadcaster *source) override;
    void subscribeStartAndEnd(value<int>& startVal, value<int>& endVal);
    void setSampleRate(double rate);

    observable_property<File> file; // cannot be changed from outside this class

private:

    // methods:
    void openFileBtnClicked();
    void paintThumbnailIfFileWasLoaded(Graphics &g, const Colour &backgroundColour, const Rectangle<int> &thumbnailBounds);

    // vars:
    const int THUMBNAIL_NUM_SAMPLES = 512;

    TextButton openFileBtn;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;

    double sampleRate;

    double start;
    double end;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThumbnailComponent)
};


#endif //GRAN_THUMBNAILCOMPONENT_H
