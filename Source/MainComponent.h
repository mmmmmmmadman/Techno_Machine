#pragma once

#include <JuceHeader.h>
#include "Core/AudioEngine.h"
#include "Core/Transport.h"

class MainComponent : public juce::AudioAppComponent,
                      public juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void timerCallback() override;

private:
    AudioEngine audioEngine_;
    Transport transport_;

    // Transport controls
    juce::TextButton playButton_{"Play"};
    juce::TextButton stopButton_{"Stop"};
    juce::TextButton regenerateButton_{"Regenerate"};

    // Tempo
    juce::Slider tempoSlider_;
    juce::Label tempoLabel_{"", "BPM"};

    // Variation
    juce::Slider variationSlider_;
    juce::Label variationLabel_{"", "Variation"};

    // 4 Role levels
    juce::Slider timelineSlider_;   // Hi-Hat
    juce::Slider foundationSlider_; // Kick
    juce::Slider grooveSlider_;     // Clap
    juce::Slider leadSlider_;       // Rim

    juce::Label timelineLabel_{"", "HiHat"};
    juce::Label foundationLabel_{"", "Kick"};
    juce::Label grooveLabel_{"", "Clap"};
    juce::Label leadLabel_{"", "Rim"};

    // Status
    juce::Label statusLabel_;

    void updateUI();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
