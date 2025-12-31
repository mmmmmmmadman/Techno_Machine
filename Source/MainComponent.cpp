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

    setSize(1000, 420);

    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [&](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
    }
    else
    {
        setAudioChannels(0, 2);
    }

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
    tempoLabel_.attachToComponent(&tempoSlider_, true);
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

    using Role = TechnoMachine::Role;

    // Timeline (Hi-Hat) level
    timelineSlider_.setRange(0.0, 1.0, 0.01);
    timelineSlider_.setValue(0.5);
    timelineSlider_.setSliderStyle(juce::Slider::LinearVertical);
    timelineSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 36, 16);
    timelineSlider_.onValueChange = [this] {
        audioEngine_.drums().setLevel(Role::TIMELINE, static_cast<float>(timelineSlider_.getValue()));
    };
    styleSlider(timelineSlider_);
    addAndMakeVisible(timelineSlider_);
    styleLabel(timelineLabel_);
    timelineLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(timelineLabel_);

    // Foundation (Kick) level
    foundationSlider_.setRange(0.0, 1.5, 0.01);
    foundationSlider_.setValue(1.0);
    foundationSlider_.setSliderStyle(juce::Slider::LinearVertical);
    foundationSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 36, 16);
    foundationSlider_.onValueChange = [this] {
        audioEngine_.drums().setLevel(Role::FOUNDATION, static_cast<float>(foundationSlider_.getValue()));
    };
    styleSlider(foundationSlider_);
    addAndMakeVisible(foundationSlider_);
    styleLabel(foundationLabel_);
    foundationLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(foundationLabel_);

    // Groove (Clap) level
    grooveSlider_.setRange(0.0, 1.0, 0.01);
    grooveSlider_.setValue(0.7);
    grooveSlider_.setSliderStyle(juce::Slider::LinearVertical);
    grooveSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 36, 16);
    grooveSlider_.onValueChange = [this] {
        audioEngine_.drums().setLevel(Role::GROOVE, static_cast<float>(grooveSlider_.getValue()));
    };
    styleSlider(grooveSlider_);
    addAndMakeVisible(grooveSlider_);
    styleLabel(grooveLabel_);
    grooveLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(grooveLabel_);

    // Lead (Rim) level
    leadSlider_.setRange(0.0, 1.0, 0.01);
    leadSlider_.setValue(0.5);
    leadSlider_.setSliderStyle(juce::Slider::LinearVertical);
    leadSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 36, 16);
    leadSlider_.onValueChange = [this] {
        audioEngine_.drums().setLevel(Role::LEAD, static_cast<float>(leadSlider_.getValue()));
    };
    styleSlider(leadSlider_);
    addAndMakeVisible(leadSlider_);
    styleLabel(leadLabel_);
    leadLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(leadLabel_);

    // === Density sliders ===

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

    // === DJ Set controls ===

    // Store colors for flash effect
    btnBgColor_ = bgMid;
    btnFlashColor_ = accent;

    // Load A button with flash effect
    loadAButton_.onClick = [this] {
        audioEngine_.loadToDeck(0);
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
    crossfaderSlider_.setValue(0.0);
    crossfaderSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    crossfaderSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    crossfaderSlider_.onValueChange = [this] {
        audioEngine_.setCrossfader(static_cast<float>(crossfaderSlider_.getValue()));
        updateDJInfo();
    };
    styleSlider(crossfaderSlider_);
    addAndMakeVisible(crossfaderSlider_);

    crossfaderLabel_.setFont(juce::Font(thinTypeface_).withHeight(16.0f));
    crossfaderLabel_.setColour(juce::Label::textColourId, textDim);
    crossfaderLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(crossfaderLabel_);

    // Role style labels
    for (int i = 0; i < 4; i++) {
        roleStyleLabels_[i].setFont(juce::Font(thinTypeface_).withHeight(16.0f));
        roleStyleLabels_[i].setColour(juce::Label::textColourId, textLight);
        addAndMakeVisible(roleStyleLabels_[i]);
    }

    // Status label
    statusLabel_.setJustificationType(juce::Justification::centred);
    statusLabel_.setFont(juce::Font(thinTypeface_).withHeight(16.0f));
    statusLabel_.setColour(juce::Label::textColourId, textLight);
    addAndMakeVisible(statusLabel_);

    transport_.setTempo(132.0);  // Default 132 BPM
    transport_.setSwingLevel(1);  // Default swing level 1
    swingButton_.setButtonText("Swing: 1");
    updateDJInfo();
    applyGlobalDensity();  // Apply initial densities

    startTimerHz(30);
    updateUI();
}

MainComponent::~MainComponent()
{
    stopTimer();
    shutdownAudio();
    setLookAndFeel(nullptr);
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    audioEngine_.prepare(sampleRate, samplesPerBlockExpected);
    transport_.prepare(sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    if (transport_.isPlaying())
    {
        auto* leftChannel = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto* rightChannel = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

        for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            transport_.advance();

            auto output = audioEngine_.process(transport_);
            leftChannel[sample] = output.left;
            rightChannel[sample] = output.right;
        }
    }
}

void MainComponent::releaseResources()
{
}

void MainComponent::paint(juce::Graphics& g)
{
    // Dark warm background
    g.fillAll(juce::Colour(0xff0e0c0c));

    // Title - vibrant pink, 32pt
    g.setColour(juce::Colour(0xffff9eb0));
    g.setFont(juce::Font(thinTypeface_).withHeight(32.0f));
    g.drawText("TECHNO MACHINE", getLocalBounds().removeFromTop(50), juce::Justification::centred);

    // Section header - 16pt, centered above density faders
    g.setFont(juce::Font(thinTypeface_).withHeight(16.0f));
    g.setColour(juce::Colour(0xffa09098));
    int headerY = getHeight() - 185;

    int densityStartX = 20 + (65 + 8) * 4 + 35;
    int densityWidth = (65 + 8) * 4 - 8;  // 4 faders width
    g.drawText("DENSITY", densityStartX, headerY, densityWidth, 20, juce::Justification::centred);

    // Thin separator line
    g.setColour(juce::Colour(0xff302828));
    g.drawLine(20, static_cast<float>(headerY - 5), static_cast<float>(getWidth() - 20), static_cast<float>(headerY - 5), 0.5f);
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
    swingButton_.setBounds(controlArea.removeFromLeft(100));
    controlArea.removeFromLeft(50);
    tempoSlider_.setBounds(controlArea.removeFromLeft(220));

    area.removeFromTop(10);

    // Global Density slider
    auto globalArea = area.removeFromTop(30);
    globalArea.removeFromLeft(60);
    globalDensitySlider_.setBounds(globalArea.removeFromLeft(220));

    area.removeFromTop(15);

    // Status
    statusLabel_.setBounds(area.removeFromTop(50));

    area.removeFromTop(20);

    // Mixer faders (4 channels) + Density faders
    auto mixerArea = area.removeFromTop(180);
    mixerArea.removeFromTop(25);

    int faderWidth = 65;
    int faderSpacing = 8;
    int sectionSpacing = 35;

    // === LEVEL section ===
    auto timelineArea = mixerArea.removeFromLeft(faderWidth);
    timelineLabel_.setBounds(timelineArea.removeFromTop(20));
    timelineSlider_.setBounds(timelineArea);

    mixerArea.removeFromLeft(faderSpacing);

    auto foundationArea = mixerArea.removeFromLeft(faderWidth);
    foundationLabel_.setBounds(foundationArea.removeFromTop(20));
    foundationSlider_.setBounds(foundationArea);

    mixerArea.removeFromLeft(faderSpacing);

    auto grooveArea = mixerArea.removeFromLeft(faderWidth);
    grooveLabel_.setBounds(grooveArea.removeFromTop(20));
    grooveSlider_.setBounds(grooveArea);

    mixerArea.removeFromLeft(faderSpacing);

    auto leadArea = mixerArea.removeFromLeft(faderWidth);
    leadLabel_.setBounds(leadArea.removeFromTop(20));
    leadSlider_.setBounds(leadArea);

    // === DENSITY section ===
    mixerArea.removeFromLeft(sectionSpacing);

    // Position density sliders
    int densityStartX = 20 + (faderWidth + faderSpacing) * 4 + sectionSpacing;
    int densityY = getHeight() - 155;

    timelineDensitySlider_.setBounds(densityStartX, densityY, faderWidth, 130);
    foundationDensitySlider_.setBounds(densityStartX + faderWidth + faderSpacing, densityY, faderWidth, 130);
    grooveDensitySlider_.setBounds(densityStartX + (faderWidth + faderSpacing) * 2, densityY, faderWidth, 130);
    leadDensitySlider_.setBounds(densityStartX + (faderWidth + faderSpacing) * 3, densityY, faderWidth, 130);

    // === DJ Set controls (right side) ===
    int djX = densityStartX + (faderWidth + faderSpacing) * 4 + 35;
    int djY = getHeight() - 155;

    loadAButton_.setBounds(djX, djY, 85, 26);
    loadBButton_.setBounds(djX + 90, djY, 85, 26);

    // Crossfader
    crossfaderLabel_.setBounds(djX, djY + 30, 175, 14);
    crossfaderSlider_.setBounds(djX, djY + 44, 175, 22);

    // Role style labels (2x2 grid) - larger for bigger font
    int labelW = 130;
    int labelH = 24;
    int labelY = djY + 70;
    roleStyleLabels_[0].setBounds(djX, labelY, labelW, labelH);           // Timeline
    roleStyleLabels_[1].setBounds(djX + 135, labelY, labelW, labelH);     // Foundation
    roleStyleLabels_[2].setBounds(djX, labelY + 26, labelW, labelH);      // Groove
    roleStyleLabels_[3].setBounds(djX + 135, labelY + 26, labelW, labelH); // Lead
}

void MainComponent::timerCallback()
{
    updateUI();
    updateDJInfo();
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
