//
// Created by goolue on 11/10/18.
//

#include <future>
#include "ThumbnailComponent.h"

ThumbnailComponent::ThumbnailComponent(shared_ptr<AudioFormatManager> formatManager) : thumbnailCache(5),
                                           thumbnail(THUMBNAIL_NUM_SAMPLES, *formatManager, thumbnailCache) {
    setSize(800, 300);

    addAndMakeVisible(&openFileBtn);
    openFileBtn.setButtonText("Open file");
    openFileBtn.setVisible(true);
    openFileBtn.onClick = [this] { openFileBtnClicked(); };
    openFileBtn.setBounds(0, 0, 100, 100);

    thumbnail.addChangeListener(this);
}

ThumbnailComponent::~ThumbnailComponent() {}

void ThumbnailComponent::changeListenerCallback(ChangeBroadcaster* source) {
    if (source == &thumbnail) repaint();
}

void ThumbnailComponent::paint(Graphics& g) {
    const Colour& backgroundColour = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    g.fillAll(backgroundColour);

    Rectangle<int> thumbnailBounds(10, 100, getWidth() - 20, getHeight() - 120);
    if (thumbnail.getNumChannels() > 0) {
        paintThumbnailIfFileWasLoaded(g, backgroundColour, thumbnailBounds);
    } else {
        g.drawFittedText("No file was loaded!", thumbnailBounds, Justification::centred, 2);
    }
}

void ThumbnailComponent::openFileBtnClicked() {
    FileChooser chooser("Select a Wave file to play...",
                        File::getSpecialLocation(File::currentExecutableFile), "*.wav");
    if (chooser.browseForFileToOpen()) {
        File openedFile(chooser.getResult());
        auto future = std::async([&] {
            file = openedFile; // notifies observers
            thumbnail.setSource(new FileInputSource(openedFile));
//            std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor(openedFile));
//            if (reader != nullptr) {
//                ReferenceCountedBuffer::Ptr newBuffer = new ReferenceCountedBuffer(openedFile.getFileName(),
//                                                                                   reader->numChannels,
//                                                                                   (int) reader->lengthInSamples);
//                reader->read(newBuffer->getAudioSampleBuffer(), 0, (int) reader->lengthInSamples, 0, true, true);
//                currentBuffer = newBuffer;
//                buffers.add(newBuffer);
//                fileLoaded = true;
//                setAudioChannels(0, reader->numChannels);
//            }
        });
    }

}

void ThumbnailComponent::paintThumbnailIfFileWasLoaded(Graphics& g, const Colour& backgroundColour,
                                                       const Rectangle<int>& thumbnailBounds) {
    g.setColour(backgroundColour);
    auto audioLength(thumbnail.getTotalLength());
    g.setColour(Colours::red);
    thumbnail.drawChannels(g, thumbnailBounds, 0.0, audioLength, 1.0f);
}