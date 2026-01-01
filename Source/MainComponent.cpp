#include "MainComponent.h"

MainComponent::MainComponent()
{
    // Load embedded Inter Thin font (cross-platform)
    thinTypeface_ = juce::Typeface::createSystemTypefaceFor(
        BinaryData::InterThin_ttf,
        BinaryData::InterThin_ttfSize
    );

    // Apply thin font to all components
    thinLookAndFeel_.setTypeface(thinTypeface_);
    setLookAndFeel(&thinLookAndFeel_);

    setSize(1000, 380);

    // Initialize application properties for settings persistence
    juce::PropertiesFile::Options options;
    options.applicationName = "TechnoMachine";
    options.folderName = "MADZINE";
    options.filenameSuffix = ".settings";
    appProperties_.setStorageParameters(options);

    // Initialize audio device
    initializeAudio();

    // Load saved settings
    loadSettings();

    // Vibrant pink palette - high visibility
    auto bgDark = juce::Colour(0xff0e0c0c);      // dark warm black
    auto bgMid = juce::Colour(0xff201a1a);       // dark warm gray
    auto accent = juce::Colour(0xffff9eb0);      // vibrant pink
    auto accentDim = juce::Colour(0xffc08090);   // bright muted pink
    auto textLight = juce::Colour(0xffffffff);   // pure white
    auto textDim = juce::Colour(0xffc8b8b8);     // bright pink-gray

    // Apply button styling - minimal, flat
    auto styleButton = [&](juce::TextButton& btn, juce::Colour text) {
        btn.setColour(juce::TextButton::buttonColourId, bgMid);
        btn.setColour(juce::TextButton::buttonOnColourId, bgMid.brighter(0.1f));
        btn.setColour(juce::TextButton::textColourOffId, text);
        btn.setColour(juce::TextButton::textColourOnId, text.brighter(0.2f));
    };

    // Play button with flash effect
    playButton_.onClick = [this] {
        transport_.start();
        updateUI();
        // Flash effect
        playButton_.setColour(juce::TextButton::buttonColourId, btnFlashColor_);
        juce::Timer::callAfterDelay(150, [this] {
            playButton_.setColour(juce::TextButton::buttonColourId, btnBgColor_);
        });
    };
    styleButton(playButton_, accent);
    addAndMakeVisible(playButton_);

    // Stop button
    stopButton_.onClick = [this] {
        transport_.stop();
        transport_.reset();
        updateUI();
    };
    styleButton(stopButton_, textLight);
    addAndMakeVisible(stopButton_);

    // Swing button (cycles 0/1/2/3)
    swingButton_.onClick = [this] {
        cycleSwing();
    };
    styleButton(swingButton_, textDim);
    addAndMakeVisible(swingButton_);

    // Settings button - removed (CV routing now inline)

    // Slider styling helper
    auto styleSlider = [&](juce::Slider& slider) {
        slider.setColour(juce::Slider::backgroundColourId, bgMid);
        slider.setColour(juce::Slider::trackColourId, accentDim);
        slider.setColour(juce::Slider::thumbColourId, accent);
        slider.setColour(juce::Slider::textBoxTextColourId, textLight);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, bgDark);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    };

    // Label styling helper - embedded thin font
    auto styleLabel = [this, textDim](juce::Label& label) {
        label.setColour(juce::Label::textColourId, textDim);
        label.setFont(juce::Font(thinTypeface_).withHeight(16.0f));
    };

    // Tempo slider - default 132 BPM
    tempoSlider_.setRange(80.0, 180.0, 0.1);
    tempoSlider_.setValue(132.0);
    tempoSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 18);
    tempoSlider_.onValueChange = [this] {
        transport_.setTempo(tempoSlider_.getValue());
    };
    styleSlider(tempoSlider_);
    addAndMakeVisible(tempoSlider_);
    styleLabel(tempoLabel_);
    tempoLabel_.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(tempoLabel_);

    // Global Density slider (offsets all 4 densities)
    globalDensitySlider_.setRange(-0.5, 0.5, 0.01);
    globalDensitySlider_.setValue(0.0);
    globalDensitySlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 18);
    globalDensitySlider_.onValueChange = [this] {
        globalDensityOffset_ = static_cast<float>(globalDensitySlider_.getValue());
        applyGlobalDensity();
    };
    styleSlider(globalDensitySlider_);
    addAndMakeVisible(globalDensitySlider_);
    styleLabel(globalDensityLabel_);
    globalDensityLabel_.attachToComponent(&globalDensitySlider_, true);
    addAndMakeVisible(globalDensityLabel_);

    // Fill Intensity slider (controls fill length/complexity/density)
    fillIntensitySlider_.setRange(0.0, 1.0, 0.01);
    fillIntensitySlider_.setValue(0.5);
    fillIntensitySlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 18);
    fillIntensitySlider_.onValueChange = [this] {
        audioEngine_.setFillIntensity(static_cast<float>(fillIntensitySlider_.getValue()));
    };
    styleSlider(fillIntensitySlider_);
    addAndMakeVisible(fillIntensitySlider_);
    styleLabel(fillIntensityLabel_);
    fillIntensityLabel_.attachToComponent(&fillIntensitySlider_, true);
    addAndMakeVisible(fillIntensityLabel_);

    using Role = TechnoMachine::Role;

    // Fixed levels at 80% (removed level sliders)
    audioEngine_.drums().setLevel(Role::TIMELINE, 0.8f);
    audioEngine_.drums().setLevel(Role::FOUNDATION, 0.8f);
    audioEngine_.drums().setLevel(Role::GROOVE, 0.8f);
    audioEngine_.drums().setLevel(Role::LEAD, 0.8f);

    // === Density sliders with labels ===

    // Timeline density - default 50%
    timelineDensitySlider_.setRange(0.0, 1.0, 0.01);
    timelineDensitySlider_.setValue(0.5);
    baseDensities_[0] = 0.5f;
    timelineDensitySlider_.setSliderStyle(juce::Slider::LinearVertical);
    timelineDensitySlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 36, 16);
    timelineDensitySlider_.onValueChange = [this] {
        baseDensities_[0] = static_cast<float>(timelineDensitySlider_.getValue());
        applyGlobalDensity();
    };
    styleSlider(timelineDensitySlider_);
    addAndMakeVisible(timelineDensitySlider_);
    styleLabel(timelineDensityLabel_);
    timelineDensityLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(timelineDensityLabel_);

    // Foundation density - default 50%
    foundationDensitySlider_.setRange(0.0, 1.0, 0.01);
    foundationDensitySlider_.setValue(0.5);
    baseDensities_[1] = 0.5f;
    foundationDensitySlider_.setSliderStyle(juce::Slider::LinearVertical);
    foundationDensitySlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 36, 16);
    foundationDensitySlider_.onValueChange = [this] {
        baseDensities_[1] = static_cast<float>(foundationDensitySlider_.getValue());
        applyGlobalDensity();
    };
    styleSlider(foundationDensitySlider_);
    addAndMakeVisible(foundationDensitySlider_);
    styleLabel(foundationDensityLabel_);
    foundationDensityLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(foundationDensityLabel_);

    // Groove density - default 50%
    grooveDensitySlider_.setRange(0.0, 1.0, 0.01);
    grooveDensitySlider_.setValue(0.5);
    baseDensities_[2] = 0.5f;
    grooveDensitySlider_.setSliderStyle(juce::Slider::LinearVertical);
    grooveDensitySlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 36, 16);
    grooveDensitySlider_.onValueChange = [this] {
        baseDensities_[2] = static_cast<float>(grooveDensitySlider_.getValue());
        applyGlobalDensity();
    };
    styleSlider(grooveDensitySlider_);
    addAndMakeVisible(grooveDensitySlider_);
    styleLabel(grooveDensityLabel_);
    grooveDensityLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(grooveDensityLabel_);

    // Lead density - default 50%
    leadDensitySlider_.setRange(0.0, 1.0, 0.01);
    leadDensitySlider_.setValue(0.5);
    baseDensities_[3] = 0.5f;
    leadDensitySlider_.setSliderStyle(juce::Slider::LinearVertical);
    leadDensitySlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 36, 16);
    leadDensitySlider_.onValueChange = [this] {
        baseDensities_[3] = static_cast<float>(leadDensitySlider_.getValue());
        applyGlobalDensity();
    };
    styleSlider(leadDensitySlider_);
    addAndMakeVisible(leadDensitySlider_);
    styleLabel(leadDensityLabel_);
    leadDensityLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(leadDensityLabel_);

    // === DJ Set controls ===

    // Store colors for flash effect
    btnBgColor_ = bgMid;
    btnFlashColor_ = accent;

    // Load A button with flash effect
    loadAButton_.onClick = [this] {
        audioEngine_.loadToDeck(0);
        syncSwingFromStyle();
        updateDJInfo();
        // Flash effect
        loadAButton_.setColour(juce::TextButton::buttonColourId, btnFlashColor_);
        juce::Timer::callAfterDelay(150, [this] {
            loadAButton_.setColour(juce::TextButton::buttonColourId, btnBgColor_);
        });
    };
    styleButton(loadAButton_, textDim);
    addAndMakeVisible(loadAButton_);

    // Load B button with flash effect
    loadBButton_.onClick = [this] {
        audioEngine_.loadToDeck(1);
        syncSwingFromStyle();
        updateDJInfo();
        // Flash effect
        loadBButton_.setColour(juce::TextButton::buttonColourId, btnFlashColor_);
        juce::Timer::callAfterDelay(150, [this] {
            loadBButton_.setColour(juce::TextButton::buttonColourId, btnBgColor_);
        });
    };
    styleButton(loadBButton_, textDim);
    addAndMakeVisible(loadBButton_);

    // === Crossfader ===
    crossfaderSlider_.setRange(0.0, 1.0, 0.01);
    crossfaderSlider_.setValue(0.0, juce::dontSendNotification);
    crossfaderSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    crossfaderSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    crossfaderSlider_.onValueChange = [this] {
        audioEngine_.setCrossfader(static_cast<float>(crossfaderSlider_.getValue()));
        syncSwingFromStyle();
        updateDJInfo();
    };
    styleSlider(crossfaderSlider_);
    addAndMakeVisible(crossfaderSlider_);

    crossfaderLabel_.setFont(juce::Font(thinTypeface_).withHeight(16.0f));
    crossfaderLabel_.setColour(juce::Label::textColourId, textDim);
    crossfaderLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(crossfaderLabel_);

    // === Build-up control ===
    // Build button - hold to activate, release to drop
    buildButton_.setClickingTogglesState(false);
    buildButton_.addMouseListener(this, false);
    styleButton(buildButton_, accent);
    addAndMakeVisible(buildButton_);

    // Bar duration selector
    buildBarsSelector_.addItem("4", 4);
    buildBarsSelector_.addItem("8", 8);
    buildBarsSelector_.addItem("16", 16);
    buildBarsSelector_.addItem("32", 32);
    buildBarsSelector_.setSelectedId(8);  // Default 8 bars
    buildBarsSelector_.onChange = [this] {
        buildupDurationBars_ = buildBarsSelector_.getSelectedId();
    };
    buildBarsSelector_.setColour(juce::ComboBox::backgroundColourId, bgMid);
    buildBarsSelector_.setColour(juce::ComboBox::textColourId, textLight);
    buildBarsSelector_.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    buildBarsSelector_.setColour(juce::ComboBox::arrowColourId, accentDim);
    addAndMakeVisible(buildBarsSelector_);

    // Role style labels
    for (int i = 0; i < 4; i++) {
        roleStyleLabels_[i].setFont(juce::Font(thinTypeface_).withHeight(16.0f));
        roleStyleLabels_[i].setColour(juce::Label::textColourId, textLight);
        addAndMakeVisible(roleStyleLabels_[i]);
    }

    // === Inline CV Routing ===
    // Voice group labels (Primary, Secondary)
    const char* voiceGroupNames[] = {"Primary", "Secondary"};
    for (int g = 0; g < 2; g++) {
        cvVoiceGroupLabels_[g].setText(voiceGroupNames[g], juce::dontSendNotification);
        cvVoiceGroupLabels_[g].setFont(juce::Font(thinTypeface_).withHeight(11.0f));
        cvVoiceGroupLabels_[g].setColour(juce::Label::textColourId, textDim);
        cvVoiceGroupLabels_[g].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(cvVoiceGroupLabels_[g]);
    }

    // Column headers (Trigger, Pitch, Velocity × 2 voice groups)
    const char* colNames[] = {"Trigger", "Pitch", "Velocity", "Trigger", "Pitch", "Velocity"};
    for (int c = 0; c < 6; c++) {
        cvColHeaders_[c].setText(colNames[c], juce::dontSendNotification);
        cvColHeaders_[c].setFont(juce::Font(thinTypeface_).withHeight(10.0f));
        cvColHeaders_[c].setColour(juce::Label::textColourId, accentDim);
        cvColHeaders_[c].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(cvColHeaders_[c]);
    }

    // Role labels
    const char* roleNames[] = {"TIMELINE", "FOUNDATION", "GROOVE", "LEAD"};
    for (int r = 0; r < 4; r++) {
        cvRoleLabels_[r].setText(roleNames[r], juce::dontSendNotification);
        cvRoleLabels_[r].setFont(juce::Font(thinTypeface_).withHeight(11.0f));
        cvRoleLabels_[r].setColour(juce::Label::textColourId, accentDim);
        addAndMakeVisible(cvRoleLabels_[r]);
    }

    // === Audio Device Selector ===
    audioDeviceSelector_.setColour(juce::ComboBox::backgroundColourId, bgMid);
    audioDeviceSelector_.setColour(juce::ComboBox::textColourId, textLight);
    audioDeviceSelector_.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    audioDeviceSelector_.setColour(juce::ComboBox::arrowColourId, accentDim);
    audioDeviceSelector_.onChange = [this] {
        int idx = audioDeviceSelector_.getSelectedId() - 1;
        auto deviceType = deviceManager_.getCurrentAudioDeviceType();
        if (!deviceType.isEmpty()) {
            if (auto* type = deviceManager_.getAvailableDeviceTypes()[0]) {
                auto deviceNames = type->getDeviceNames(false);  // false = output
                if (idx >= 0 && idx < deviceNames.size()) {
                    juce::AudioDeviceManager::AudioDeviceSetup setup;
                    deviceManager_.getAudioDeviceSetup(setup);
                    setup.outputDeviceName = deviceNames[idx];
                    deviceManager_.setAudioDeviceSetup(setup, true);
                }
            }
        }
    };
    addAndMakeVisible(audioDeviceSelector_);

    // Populate audio device list
    auto& deviceTypes = deviceManager_.getAvailableDeviceTypes();
    if (deviceTypes.size() > 0) {
        auto* type = deviceTypes[0];
        auto deviceNames = type->getDeviceNames(false);  // false = output devices
        int currentIdx = 0;
        juce::String currentDevice;
        if (auto* device = deviceManager_.getCurrentAudioDevice()) {
            currentDevice = device->getName();
        }
        for (int i = 0; i < deviceNames.size(); i++) {
            audioDeviceSelector_.addItem(deviceNames[i], i + 1);
            if (deviceNames[i] == currentDevice) {
                currentIdx = i;
            }
        }
        if (deviceNames.size() > 0) {
            audioDeviceSelector_.setSelectedId(currentIdx + 1, juce::dontSendNotification);
        }
    }

    // 24 ComboBoxes: 4 roles × 2 voices × 3 signals (Trig/Pitch/Vel)
    for (int i = 0; i < 24; i++) {
        cvRouteBoxes_[i].addItem("...", 1);  // Off
        for (int ch = 2; ch <= 25; ch++) {
            cvRouteBoxes_[i].addItem(juce::String(ch), ch);
        }
        cvRouteBoxes_[i].setSelectedId(1);  // Default off
        cvRouteBoxes_[i].setColour(juce::ComboBox::backgroundColourId, bgMid);
        cvRouteBoxes_[i].setColour(juce::ComboBox::textColourId, textLight);
        cvRouteBoxes_[i].setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        cvRouteBoxes_[i].setColour(juce::ComboBox::arrowColourId, accentDim);

        int boxIdx = i;
        cvRouteBoxes_[i].onChange = [this, boxIdx] {
            int role = boxIdx / 6;
            int voiceInRole = (boxIdx % 6) / 3;
            int signalType = boxIdx % 3;  // 0=Trig, 1=Pitch, 2=Vel
            int voice = role * 2 + voiceInRole;

            int selectedId = cvRouteBoxes_[boxIdx].getSelectedId();
            int channel = (selectedId == 1) ? -1 : selectedId;  // -1 = off

            int signalIdx = TechnoMachine::CVOutputRouter::getSignalIndex(
                voice, static_cast<TechnoMachine::CVType>(signalType));
            cvRouter_.setRoute(signalIdx, channel);
        };
        addAndMakeVisible(cvRouteBoxes_[i]);
    }

    // === Sample Panel (bottom-right) - 8 voices ===
    samplePanelLabel_.setFont(juce::Font(thinTypeface_).withHeight(11.0f));
    samplePanelLabel_.setColour(juce::Label::textColourId, accentDim);
    addAndMakeVisible(samplePanelLabel_);

    // Voice group labels (Primary, Secondary)
    const char* sampleVoiceGroupNames[] = {"Primary", "Secondary"};
    for (int g = 0; g < 2; g++) {
        sampleVoiceGroupLabels_[g].setText(sampleVoiceGroupNames[g], juce::dontSendNotification);
        sampleVoiceGroupLabels_[g].setFont(juce::Font(thinTypeface_).withHeight(10.0f));
        sampleVoiceGroupLabels_[g].setColour(juce::Label::textColourId, textDim);
        sampleVoiceGroupLabels_[g].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(sampleVoiceGroupLabels_[g]);
    }

    const char* sampleRoleNames[] = {"Timeline", "Foundation", "Groove", "Lead"};
    for (int r = 0; r < 4; r++) {
        // Role label
        sampleRoleLabels_[r].setText(sampleRoleNames[r], juce::dontSendNotification);
        sampleRoleLabels_[r].setFont(juce::Font(thinTypeface_).withHeight(11.0f));
        sampleRoleLabels_[r].setColour(juce::Label::textColourId, textDim);
        addAndMakeVisible(sampleRoleLabels_[r]);
    }

    // 8 sample slots (2 per role)
    for (int v = 0; v < 8; v++) {
        // Sample name display
        sampleNameLabels_[v].setText("-", juce::dontSendNotification);
        sampleNameLabels_[v].setFont(juce::Font(thinTypeface_).withHeight(10.0f));
        sampleNameLabels_[v].setColour(juce::Label::textColourId, textLight);
        sampleNameLabels_[v].setColour(juce::Label::backgroundColourId, bgMid);
        sampleNameLabels_[v].setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(sampleNameLabels_[v]);

        // Load button
        sampleLoadButtons_[v].setButtonText("...");
        sampleLoadButtons_[v].setColour(juce::TextButton::buttonColourId, bgMid);
        sampleLoadButtons_[v].setColour(juce::TextButton::textColourOffId, accent);
        int voiceIdx = v;
        sampleLoadButtons_[v].onClick = [this, voiceIdx] {
            loadSampleForVoice(voiceIdx);
        };
        addAndMakeVisible(sampleLoadButtons_[v]);
    }

    // Status label
    statusLabel_.setJustificationType(juce::Justification::centred);
    statusLabel_.setFont(juce::Font(thinTypeface_).withHeight(16.0f));
    statusLabel_.setColour(juce::Label::textColourId, textLight);
    addAndMakeVisible(statusLabel_);

    transport_.setTempo(132.0);  // Default 132 BPM

    updateDJInfo();
    applyGlobalDensity();  // Apply initial densities

    // Set default swing AFTER all initialization to prevent overwriting
    transport_.setSwingLevel(swingLevel_);
    const char* swingLabels[] = {"Swing: Off", "Swing: 1", "Swing: 2", "Swing: 3"};
    swingButton_.setButtonText(swingLabels[swingLevel_]);

    startTimerHz(30);
    updateUI();
}

MainComponent::~MainComponent()
{
    stopTimer();
    saveSettings();
    deviceManager_.removeAudioCallback(this);
    deviceManager_.closeAudioDevice();
    setLookAndFeel(nullptr);
}

void MainComponent::initializeAudio()
{
    // Initialize audio device manager with multi-channel support
    auto result = deviceManager_.initialise(
        0,      // numInputChannels
        32,     // numOutputChannels (request many for CV support)
        nullptr,
        true,   // selectDefaultDeviceOnFailure
        {},     // preferredDeviceType
        nullptr // preferredSetupOptions
    );

    if (result.isNotEmpty()) {
        DBG("Audio device initialization error: " + result);
    }

    // Register as audio callback
    deviceManager_.addAudioCallback(this);

    // Set up default CV routing based on available channels
    if (auto* device = deviceManager_.getCurrentAudioDevice()) {
        int numChannels = device->getActiveOutputChannels().countNumberOfSetBits();
        cvRouter_.setDefaultRouting(numChannels);
    }
}

void MainComponent::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    double sampleRate = device->getCurrentSampleRate();
    int blockSize = device->getCurrentBufferSizeSamples();

    audioEngine_.prepare(sampleRate, blockSize);
    transport_.prepare(sampleRate);
    cvRouter_.setSampleRate(sampleRate);

    // Update CV routing based on available channels
    int numChannels = device->getActiveOutputChannels().countNumberOfSetBits();
    cvRouter_.setDefaultRouting(numChannels);
}

void MainComponent::audioDeviceStopped()
{
    // Nothing to clean up
}

void MainComponent::audioDeviceIOCallbackWithContext(
    const float* const* /*inputChannelData*/,
    int /*numInputChannels*/,
    float* const* outputChannelData,
    int numOutputChannels,
    int numSamples,
    const juce::AudioIODeviceCallbackContext& /*context*/)
{
    // Clear all output channels
    for (int ch = 0; ch < numOutputChannels; ++ch) {
        juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);
    }

    // Process audio
    if (transport_.isPlaying()) {
        for (int sample = 0; sample < numSamples; ++sample) {
            transport_.advance();

            auto output = audioEngine_.process(transport_);

            // Stereo output to channels 0-1
            if (numOutputChannels >= 1) {
                outputChannelData[0][sample] = output.left;
            }
            if (numOutputChannels >= 2) {
                outputChannelData[1][sample] = output.right;
            }

            // Check for triggers and notify CV router
            // This is done per-sample to maintain timing accuracy
            for (int voice = 0; voice < TechnoMachine::NUM_VOICES; ++voice) {
                if (audioEngine_.wasVoiceTriggered(voice)) {
                    float velocity = audioEngine_.getLastVelocity(voice);
                    float freq = audioEngine_.drums().getVoiceFrequency(voice);
                    cvRouter_.noteTrigger(voice, velocity);
                    cvRouter_.setVoiceFrequency(voice, freq);
                }
            }
        }
    }

    // Process CV outputs (channels 2+)
    cvRouter_.process(outputChannelData, numOutputChannels, numSamples);
}

void MainComponent::paint(juce::Graphics& g)
{
    // Dark warm background
    g.fillAll(juce::Colour(0xff0e0c0c));

    // Title - vibrant pink, 32pt
    g.setColour(juce::Colour(0xffff9eb0));
    g.setFont(juce::Font(thinTypeface_).withHeight(32.0f));
    g.drawText("TECHNO MACHINE", getLocalBounds().removeFromTop(50), juce::Justification::centred);

    // Thin separator line above bottom section
    g.setColour(juce::Colour(0xff302828));
    g.drawLine(20, static_cast<float>(getHeight() - 168), static_cast<float>(getWidth() - 20), static_cast<float>(getHeight() - 168), 0.5f);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(50);

    // Transport controls row 1
    auto controlArea = area.removeFromTop(36);
    playButton_.setBounds(controlArea.removeFromLeft(70));
    controlArea.removeFromLeft(8);
    stopButton_.setBounds(controlArea.removeFromLeft(70));
    controlArea.removeFromLeft(8);
    swingButton_.setBounds(controlArea.removeFromLeft(80));
    controlArea.removeFromLeft(16);
    // BPM label positioned manually (not attached)
    tempoLabel_.setBounds(controlArea.removeFromLeft(35));
    tempoSlider_.setBounds(controlArea.removeFromLeft(200));

    area.removeFromTop(10);

    // Global Density + Fill Intensity sliders (side by side)
    auto globalArea = area.removeFromTop(30);
    globalArea.removeFromLeft(60);  // Label space for "Global"
    globalDensitySlider_.setBounds(globalArea.removeFromLeft(180));
    globalArea.removeFromLeft(60);  // Label space for "Fill"
    fillIntensitySlider_.setBounds(globalArea.removeFromLeft(180));

    area.removeFromTop(5);

    // Status (below Global/Fill)
    statusLabel_.setBounds(20, 155, 480, 24);

    // === Bottom section: Density faders + DJ controls ===
    int faderWidth = 65;
    int faderSpacing = 8;
    int densityStartX = 20;
    int densityY = getHeight() - 160;

    // Density labels
    timelineDensityLabel_.setBounds(densityStartX, densityY, faderWidth, 18);
    foundationDensityLabel_.setBounds(densityStartX + faderWidth + faderSpacing, densityY, faderWidth, 18);
    grooveDensityLabel_.setBounds(densityStartX + (faderWidth + faderSpacing) * 2, densityY, faderWidth, 18);
    leadDensityLabel_.setBounds(densityStartX + (faderWidth + faderSpacing) * 3, densityY, faderWidth, 18);

    // Density sliders (below labels)
    int sliderY = densityY + 20;
    int sliderH = 130;
    timelineDensitySlider_.setBounds(densityStartX, sliderY, faderWidth, sliderH);
    foundationDensitySlider_.setBounds(densityStartX + faderWidth + faderSpacing, sliderY, faderWidth, sliderH);
    grooveDensitySlider_.setBounds(densityStartX + (faderWidth + faderSpacing) * 2, sliderY, faderWidth, sliderH);
    leadDensitySlider_.setBounds(densityStartX + (faderWidth + faderSpacing) * 3, sliderY, faderWidth, sliderH);

    // === DJ Set controls (next to density faders) ===
    int djX = densityStartX + (faderWidth + faderSpacing) * 4 + 25;
    int djY = densityY;

    loadAButton_.setBounds(djX, djY, 85, 26);
    loadBButton_.setBounds(djX + 90, djY, 85, 26);

    // Crossfader
    crossfaderLabel_.setBounds(djX, djY + 30, 175, 14);
    crossfaderSlider_.setBounds(djX, djY + 44, 175, 22);

    // Build-up controls (below crossfader)
    int buildY = djY + 70;
    buildButton_.setBounds(djX, buildY, 70, 26);
    buildBarsSelector_.setBounds(djX + 75, buildY, 50, 26);

    // Role style labels (2x2 grid, below build controls)
    int labelW = 130;
    int labelH = 22;
    int labelY = buildY + 32;
    roleStyleLabels_[0].setBounds(djX, labelY, labelW, labelH);           // Timeline
    roleStyleLabels_[1].setBounds(djX + 135, labelY, labelW, labelH);     // Foundation
    roleStyleLabels_[2].setBounds(djX, labelY + 24, labelW, labelH);      // Groove
    roleStyleLabels_[3].setBounds(djX + 135, labelY + 24, labelW, labelH); // Lead

    // === Audio Device Selector (top right) ===
    audioDeviceSelector_.setBounds(560, 55, 300, 24);

    // === CV Routing panel (upper right, below audio device) ===
    int cvX = 560;  // Start position
    int cvY = 85;   // Below audio device
    int boxW = 50;  // ComboBox width
    int boxH = 20;  // ComboBox height
    int boxGap = 4; // Gap between boxes
    int rowH = 24;  // Row height
    int roleLabelW = 85;
    int voiceGap = 14;  // Extra gap between V1 and V2 groups
    int groupW = 3 * (boxW + boxGap) - boxGap;  // Width of one voice group

    // Voice group labels row (Primary, Secondary)
    int groupLabelY = cvY;
    int group1X = cvX + roleLabelW;
    int group2X = cvX + roleLabelW + 3 * (boxW + boxGap) + voiceGap;
    cvVoiceGroupLabels_[0].setBounds(group1X, groupLabelY, groupW, 14);
    cvVoiceGroupLabels_[1].setBounds(group2X, groupLabelY, groupW, 14);

    // Column headers row (Trigger, Pitch, Velocity × 2)
    int headerY = cvY + 14;
    for (int c = 0; c < 6; c++) {
        int extraGap = (c >= 3) ? voiceGap : 0;
        int headerX = cvX + roleLabelW + c * (boxW + boxGap) + extraGap;
        cvColHeaders_[c].setBounds(headerX, headerY, boxW, 14);
    }

    // Data rows start below headers
    int dataY = cvY + 30;

    // Layout: 4 rows (roles) × 6 ComboBoxes (V1:Trig/Pitch/Vel, V2:Trig/Pitch/Vel)
    for (int r = 0; r < 4; r++) {
        int rowY = dataY + r * rowH;
        cvRoleLabels_[r].setBounds(cvX, rowY, roleLabelW, boxH);

        for (int b = 0; b < 6; b++) {
            int boxIdx = r * 6 + b;
            int extraGap = (b >= 3) ? voiceGap : 0;  // Add gap between V1 and V2
            int boxX = cvX + roleLabelW + b * (boxW + boxGap) + extraGap;
            cvRouteBoxes_[boxIdx].setBounds(boxX, rowY, boxW, boxH);
        }
    }

    // === Sample Panel (bottom-right) - 8 voices ===
    // Align with CV routing Trigger dropdown (X=645)
    int sampleX = cvX + roleLabelW;  // Align with first dropdown column
    int sampleY = getHeight() - 130;
    int sampleRowH = 22;
    int sampleRoleLabelW = 65;
    int sampleNameW = 70;   // Narrower filename display
    int sampleLoadW = 22;
    int sampleGap = 2;
    int sampleColW = sampleNameW + sampleGap + sampleLoadW;  // Width per voice column
    int sampleColGap = 8;  // Gap between Primary and Secondary columns

    samplePanelLabel_.setBounds(sampleX, sampleY, 200, 14);

    // Voice group headers (Primary, Secondary)
    int sampleHeaderY = sampleY + 14;
    sampleVoiceGroupLabels_[0].setBounds(sampleX + sampleRoleLabelW, sampleHeaderY, sampleColW, 12);
    sampleVoiceGroupLabels_[1].setBounds(sampleX + sampleRoleLabelW + sampleColW + sampleColGap, sampleHeaderY, sampleColW, 12);

    // 4 roles × 2 voices
    for (int r = 0; r < 4; r++) {
        int rowY = sampleY + 28 + r * sampleRowH;
        sampleRoleLabels_[r].setBounds(sampleX, rowY, sampleRoleLabelW, sampleRowH);

        for (int v = 0; v < 2; v++) {
            int voiceIdx = r * 2 + v;
            int colX = sampleX + sampleRoleLabelW + v * (sampleColW + sampleColGap);
            sampleNameLabels_[voiceIdx].setBounds(colX, rowY, sampleNameW, sampleRowH - 2);
            sampleLoadButtons_[voiceIdx].setBounds(colX + sampleNameW + sampleGap, rowY, sampleLoadW, sampleRowH - 2);
        }
    }
}

void MainComponent::timerCallback()
{
    updateUI();
    updateDJInfo();
    updateBuildup();
}

void MainComponent::mouseDown(const juce::MouseEvent& event)
{
    if (event.eventComponent == &buildButton_) {
        startBuildup();
    }
}

void MainComponent::mouseUp(const juce::MouseEvent& event)
{
    if (event.eventComponent == &buildButton_) {
        stopBuildup();
    }
}

void MainComponent::updateUI()
{
    juce::String status;
    if (transport_.isPlaying())
    {
        int bar = transport_.getCurrentBar() + 1;
        int beat = transport_.getCurrentBeat() + 1;
        int sixteenth = transport_.getCurrentSixteenth() + 1;
        status = juce::String::formatted("Bar %d | Beat %d.%d | %.1f BPM",
                                          bar, beat, sixteenth, transport_.getTempo());
    }
    else
    {
        status = "Stopped - Press Play";
    }
    statusLabel_.setText(status, juce::dontSendNotification);
}

void MainComponent::updateDJInfo()
{
    using Role = TechnoMachine::Role;

    // Crossfader position indicator
    float pos = audioEngine_.getCrossfader();
    int percent = static_cast<int>(pos * 100.0f);
    crossfaderLabel_.setText(juce::String::formatted("A  %d%%  B", percent), juce::dontSendNotification);

    // Determine active deck based on crossfader position
    int activeDeck = (pos < 0.5f) ? 0 : 1;

    // Role style labels - show active deck's styles with full names
    const char* roleNames[] = {"Timeline: ", "Foundation: ", "Groove: ", "Lead: "};
    const Role roles[] = {Role::TIMELINE, Role::FOUNDATION, Role::GROOVE, Role::LEAD};

    for (int i = 0; i < 4; i++) {
        const char* styleName = audioEngine_.getDeckRoleStyleName(activeDeck, roles[i]);
        roleStyleLabels_[i].setText(juce::String(roleNames[i]) + styleName, juce::dontSendNotification);
    }
}

void MainComponent::cycleSwing()
{
    swingLevel_ = (swingLevel_ + 1) % 4;
    transport_.setSwingLevel(swingLevel_);

    const char* labels[] = {"Swing: Off", "Swing: 1", "Swing: 2", "Swing: 3"};
    swingButton_.setButtonText(labels[swingLevel_]);
}

void MainComponent::applyGlobalDensity()
{
    using Role = TechnoMachine::Role;
    const Role roles[] = {Role::TIMELINE, Role::FOUNDATION, Role::GROOVE, Role::LEAD};

    for (int i = 0; i < 4; i++) {
        float finalDensity = std::clamp(baseDensities_[i] + globalDensityOffset_, 0.0f, 1.0f);
        audioEngine_.setPlaybackDensity(roles[i], finalDensity);
    }
}

void MainComponent::loadSettings()
{
    if (auto* props = appProperties_.getUserSettings()) {
        // Load CV routing
        juce::String cvState = props->getValue("cvRouting", "");
        if (cvState.isNotEmpty()) {
            cvRouter_.setStateFromString(cvState);

            // Sync inline ComboBoxes with loaded state
            for (int r = 0; r < 4; r++) {
                for (int v = 0; v < 2; v++) {
                    int voice = r * 2 + v;
                    for (int s = 0; s < 3; s++) {
                        int boxIdx = r * 6 + v * 3 + s;
                        int signalIdx = TechnoMachine::CVOutputRouter::getSignalIndex(
                            voice, static_cast<TechnoMachine::CVType>(s));
                        int channel = cvRouter_.getRoute(signalIdx);
                        if (channel < 0) {
                            cvRouteBoxes_[boxIdx].setSelectedId(1, juce::dontSendNotification);
                        } else {
                            cvRouteBoxes_[boxIdx].setSelectedId(channel, juce::dontSendNotification);
                        }
                    }
                }
            }
        }

        // Load audio device settings
        auto savedState = props->getXmlValue("audioDeviceState");
        if (savedState != nullptr) {
            deviceManager_.initialise(0, 32, savedState.get(), true);
        }

        // Load sample paths (8 voices: 2 per role)
        for (int v = 0; v < 8; v++) {
            juce::String key = "samplePath" + juce::String(v);
            juce::String path = props->getValue(key, "");
            if (path.isNotEmpty()) {
                juce::File file(path);
                if (file.existsAsFile()) {
                    audioEngine_.loadSample(v, file);
                }
            }
        }
        updateSampleDisplay();
    }
}

void MainComponent::saveSettings()
{
    if (auto* props = appProperties_.getUserSettings()) {
        // Save CV routing
        props->setValue("cvRouting", cvRouter_.getStateAsString());

        // Save audio device settings
        auto state = deviceManager_.createStateXml();
        if (state != nullptr) {
            props->setValue("audioDeviceState", state.get());
        }

        // Save sample paths (8 voices: 2 per role)
        for (int v = 0; v < 8; v++) {
            juce::String key = "samplePath" + juce::String(v);
            juce::String path = audioEngine_.getSamplePath(v);
            props->setValue(key, path);
        }

        props->saveIfNeeded();
    }
}

void MainComponent::syncSwingFromStyle()
{
    // Get style's swing ratio and apply to transport
    float styleSwing = audioEngine_.getStyleSwing();
    transport_.setSwingRatio(styleSwing);

    // Update UI to reflect new swing level
    swingLevel_ = transport_.getSwingLevel();
    const char* labels[] = {"Swing: Off", "Swing: 1", "Swing: 2", "Swing: 3"};
    swingButton_.setButtonText(labels[swingLevel_]);
}

void MainComponent::startBuildup()
{
    if (buildupActive_) return;

    // Save current values
    preBuildupFillIntensity_ = static_cast<float>(fillIntensitySlider_.getValue());
    preBuildupGlobalDensity_ = static_cast<float>(globalDensitySlider_.getValue());
    preBuildupFillInterval_ = audioEngine_.getFillInterval();

    // Record start bar
    buildupStartBar_ = transport_.getCurrentBar();
    buildupActive_ = true;

    // Visual feedback - change button color
    buildButton_.setColour(juce::TextButton::buttonColourId, btnFlashColor_);
}

void MainComponent::stopBuildup()
{
    if (!buildupActive_) return;

    buildupActive_ = false;

    // Restore original values
    fillIntensitySlider_.setValue(preBuildupFillIntensity_, juce::dontSendNotification);
    audioEngine_.setFillIntensity(preBuildupFillIntensity_);

    globalDensitySlider_.setValue(preBuildupGlobalDensity_, juce::dontSendNotification);
    globalDensityOffset_ = preBuildupGlobalDensity_;
    applyGlobalDensity();

    audioEngine_.setFillInterval(preBuildupFillInterval_);

    // Reset button color and text
    buildButton_.setColour(juce::TextButton::buttonColourId, btnBgColor_);
    buildButton_.setButtonText("Build");
}

void MainComponent::updateBuildup()
{
    if (!buildupActive_ || !transport_.isPlaying()) return;

    // Calculate progress (0.0 to 1.0) with sub-bar precision
    int currentBar = transport_.getCurrentBar();
    // getCurrentBeat() = 0-3, getCurrentSixteenth() = 0-3 within beat
    int sixteenthInBar = transport_.getCurrentBeat() * 4 + transport_.getCurrentSixteenth();
    float barFraction = sixteenthInBar / 16.0f;  // 0-1 within bar
    float elapsedBars = static_cast<float>(currentBar - buildupStartBar_) + barFraction;
    float progress = elapsedBars / static_cast<float>(buildupDurationBars_);
    progress = std::clamp(progress, 0.0f, 1.0f);

    // Interpolate Fill Intensity: current → 1.0
    float targetFill = preBuildupFillIntensity_ + (1.0f - preBuildupFillIntensity_) * progress;
    fillIntensitySlider_.setValue(targetFill, juce::dontSendNotification);
    audioEngine_.setFillIntensity(targetFill);

    // Interpolate Global Density: linear rise to maximum (0.5)
    float densityMod = preBuildupGlobalDensity_ + (0.5f - preBuildupGlobalDensity_) * progress;
    densityMod = std::clamp(densityMod, -0.5f, 0.5f);
    globalDensitySlider_.setValue(densityMod, juce::dontSendNotification);
    globalDensityOffset_ = densityMod;
    applyGlobalDensity();

    // Adjust Fill Interval: shorten as progress increases
    // 0-30%: original interval
    // 30-60%: 2 bars
    // 60-100%: 1 bar
    int newInterval;
    if (progress < 0.3f) {
        newInterval = preBuildupFillInterval_;
    } else if (progress < 0.6f) {
        newInterval = std::min(preBuildupFillInterval_, 2);
    } else {
        newInterval = 1;
    }
    audioEngine_.setFillInterval(newInterval);

    // Update button text to show progress
    int percent = static_cast<int>(progress * 100.0f);
    buildButton_.setButtonText(juce::String::formatted("Build %d%%", percent));

    // Auto-complete when reaching 100%
    if (progress >= 1.0f) {
        // Keep at max values until button released
        buildButton_.setButtonText("DROP!");
    }
}

void MainComponent::loadSampleForVoice(int voiceIdx)
{
    const char* roleNames[] = {"Timeline", "Foundation", "Groove", "Lead"};
    int role = voiceIdx / 2;
    int voiceInRole = voiceIdx % 2;
    juce::String voiceType = (voiceInRole == 0) ? "Primary" : "Secondary";

    sampleFileChooser_ = std::make_unique<juce::FileChooser>(
        "Select Sample for " + juce::String(roleNames[role]) + " " + voiceType,
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.wav;*.aiff;*.WAV;*.AIFF"
    );

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    sampleFileChooser_->launchAsync(flags, [this, voiceIdx](const juce::FileChooser& chooser) {
        auto file = chooser.getResult();
        if (file.existsAsFile()) {
            if (audioEngine_.loadSample(voiceIdx, file)) {
                updateSampleDisplay();
                // Save sample paths
                saveSettings();
            }
        }
    });
}

void MainComponent::updateSampleDisplay()
{
    for (int v = 0; v < 8; v++) {
        if (audioEngine_.hasSample(v)) {
            // Truncate filename to fit
            juce::String name = audioEngine_.getSampleName(v);
            if (name.length() > 10) {
                name = name.substring(0, 8) + "..";
            }
            sampleNameLabels_[v].setText(name, juce::dontSendNotification);
        } else {
            sampleNameLabels_[v].setText("-", juce::dontSendNotification);
        }
    }
}
