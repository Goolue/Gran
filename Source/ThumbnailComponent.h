#ifndef GRAN_THUMBNAILCOMPONENT_H
#define GRAN_THUMBNAILCOMPONENT_H
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PlayState.h"

class ThumbnailComponent : public Component, public ChangeListener {
public:
    // methods:
    ThumbnailComponent();
    ~ThumbnailComponent();

    void paint(Graphics &g) override;
    void changeListenerCallback(ChangeBroadcaster *source) override;

    bool wasFileLoaded() const;

private:

    // methods:
    void openFileBtnClicked();
    void paintThumbnailIfFileWasLoaded(Graphics &g, const Colour &backgroundColour, const Rectangle<int> &thumbnailBounds);

    // vars:
    const int THUMBNAIL_NUM_SAMPLES = 512;
    bool fileLoaded = false;

    TextButton openFileBtn;
    AudioFormatManager formatManager;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;

};


#endif //GRAN_THUMBNAILCOMPONENT_H
