#pragma once

#include <JuceHeader.h>
#include "BinaryData.h"
#include "Core/AudioEngine.h"
#include "Core/Transport.h"

// Simple LookAndFeel to use embedded font
class ThinFontLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void setTypeface(juce::Typeface::Ptr tf) { typeface_ = tf; }

    juce::Font getTextButtonFont(juce::TextButton&, int) override
    {
        return juce::Font(typeface_).withHeight(16.0f);
    }

    juce::Font getLabelFont(juce::Label&) override
    {
        return juce::Font(typeface_).withHeight(16.0f);
    }

private:
    juce::Typeface::Ptr typeface_;
};

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
    // Embedded thin font for cross-platform
    juce::Typeface::Ptr thinTypeface_;
    ThinFontLookAndFeel thinLookAndFeel_;

    AudioEngine audioEngine_;
    Transport transport_;

    // Transport controls
    juce::TextButton playButton_{"Play"};
    juce::TextButton stopButton_{"Stop"};
    juce::TextButton swingButton_{"Swing: Off"};

    // Tempo
    juce::Slider tempoSlider_;
    juce::Label tempoLabel_{"", "BPM"};

    // Global Density (offsets all 4 densities)
    juce::Slider globalDensitySlider_;
    juce::Label globalDensityLabel_{"", "Global"};

    // 4 Role levels
    juce::Slider timelineSlider_;   // Hi-Hat
    juce::Slider foundationSlider_; // Kick
    juce::Slider grooveSlider_;     // Clap
    juce::Slider leadSlider_;       // Rim

    juce::Label timelineLabel_{"", "Timeline"};
    juce::Label foundationLabel_{"", "Foundation"};
    juce::Label grooveLabel_{"", "Groove"};
    juce::Label leadLabel_{"", "Lead"};

    // 4 Role density sliders
    juce::Slider timelineDensitySlider_;
    juce::Slider foundationDensitySlider_;
    juce::Slider grooveDensitySlider_;
    juce::Slider leadDensitySlider_;

    juce::Label densityHeaderLabel_{"", "DENSITY"};

    // DJ Set controls
    juce::TextButton loadAButton_{"Load A"};
    juce::TextButton loadBButton_{"Load B"};

    // Crossfader
    juce::Slider crossfaderSlider_;
    juce::Label crossfaderLabel_{"", "CROSSFADER"};

    // Role style display (4 labels)
    juce::Label roleStyleLabels_[4];

    // Status
    juce::Label statusLabel_;

    void updateUI();
    void updateDJInfo();
    void cycleSwing();
    void applyGlobalDensity();

    int swingLevel_ = 1;  // default swing level 1
    float globalDensityOffset_ = 0.0f;
    float baseDensities_[4] = {0.5f, 0.5f, 0.5f, 0.5f};

    // Colors for button flash - vibrant pink
    juce::Colour btnBgColor_{0xff201a1a};
    juce::Colour btnFlashColor_{0xffff9eb0};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
