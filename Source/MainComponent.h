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

    // Mouse events for Build button
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

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

    // Build-up control
    juce::TextButton buildButton_{"Build"};
    juce::ComboBox buildBarsSelector_;

    // Role style display (4 labels)
    juce::Label roleStyleLabels_[4];

    // Status
    juce::Label statusLabel_;

    // Inline CV Routing (4 roles × 3 signals = 12 ComboBoxes)
    juce::ComboBox cvRouteBoxes_[12];
    juce::Label cvRoleLabels_[4];  // TIMELINE, FOUNDATION, GROOVE, LEAD
    juce::Label cvColHeaders_[3];  // Trigger, Pitch, Velocity

    // Audio Device selector
    juce::ComboBox audioDeviceSelector_;

    // Sample panel (bottom-right) - 4 voices (1 per role)
    juce::Label samplePanelLabel_{"", "SAMPLES"};
    juce::Label sampleRoleLabels_[4];        // Timeline, Foundation, Groove, Lead
    juce::Label sampleNameLabels_[4];        // Display loaded sample names (4 voices)
    juce::TextButton sampleLoadButtons_[4];  // Load buttons (4 voices)

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
    void loadSampleForVoice(int voiceIdx);
    void updateSampleDisplay();

    int swingLevel_ = 1;  // default swing level 1
    float globalDensityOffset_ = 0.0f;
    float baseDensities_[4] = {0.5f, 0.5f, 0.5f, 0.5f};

    // Build-up state
    bool buildupActive_ = false;
    int buildupStartBar_ = 0;
    int buildupDurationBars_ = 8;
    float preBuildupFillIntensity_ = 0.5f;
    float preBuildupGlobalDensity_ = 0.0f;
    int preBuildupFillInterval_ = 4;

    void startBuildup();
    void stopBuildup();
    void updateBuildup();

    // Colors for button flash - vibrant pink
    juce::Colour btnBgColor_{0xff201a1a};
    juce::Colour btnFlashColor_{0xffff9eb0};

    // File chooser for sample loading (must persist for async operation)
    std::unique_ptr<juce::FileChooser> sampleFileChooser_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
