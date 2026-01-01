#pragma once

#include <JuceHeader.h>
#include "BinaryData.h"
#include "Core/AudioEngine.h"
#include "Core/Transport.h"
#include "Core/CVOutputRouter.h"

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

class MainComponent : public juce::Component,
                      public juce::AudioIODeviceCallback,
                      public juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    // AudioIODeviceCallback
    void audioDeviceIOCallbackWithContext(
        const float* const* inputChannelData,
        int numInputChannels,
        float* const* outputChannelData,
        int numOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext& context) override;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void timerCallback() override;

    // Access for settings window
    juce::AudioDeviceManager& getDeviceManager() { return deviceManager_; }
    TechnoMachine::CVOutputRouter& getCVRouter() { return cvRouter_; }

private:
    // Audio device management
    juce::AudioDeviceManager deviceManager_;
    TechnoMachine::CVOutputRouter cvRouter_;

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

    // Fill Intensity
    juce::Slider fillIntensitySlider_;
    juce::Label fillIntensityLabel_{"", "Fill"};

    // 4 Role density sliders with labels
    juce::Slider timelineDensitySlider_;
    juce::Slider foundationDensitySlider_;
    juce::Slider grooveDensitySlider_;
    juce::Slider leadDensitySlider_;

    juce::Label timelineDensityLabel_{"", "Timeline"};
    juce::Label foundationDensityLabel_{"", "Foundation"};
    juce::Label grooveDensityLabel_{"", "Groove"};
    juce::Label leadDensityLabel_{"", "Lead"};

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

    // Inline CV Routing (4 roles × 2 voices × 3 signals = 24 ComboBoxes)
    juce::ComboBox cvRouteBoxes_[24];
    juce::Label cvRoleLabels_[4];  // TIMELINE, FOUNDATION, GROOVE, LEAD
    juce::Label cvColHeaders_[6];  // Trigger, Pitch, Velocity × 2
    juce::Label cvVoiceGroupLabels_[2];  // Primary, Secondary

    // Audio Device selector
    juce::ComboBox audioDeviceSelector_;

    // Application settings persistence
    juce::ApplicationProperties appProperties_;

    void updateUI();
    void updateDJInfo();
    void cycleSwing();
    void applyGlobalDensity();
    void initializeAudio();
    void loadSettings();
    void saveSettings();
    void syncSwingFromStyle();

    int swingLevel_ = 1;  // default swing level 1
    float globalDensityOffset_ = 0.0f;
    float baseDensities_[4] = {0.5f, 0.5f, 0.5f, 0.5f};

    // Colors for button flash - vibrant pink
    juce::Colour btnBgColor_{0xff201a1a};
    juce::Colour btnFlashColor_{0xffff9eb0};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
