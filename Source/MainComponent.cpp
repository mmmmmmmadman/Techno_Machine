#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize(1050, 450);

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

    // Play button
    playButton_.onClick = [this] {
        transport_.start();
        updateUI();
    };
    addAndMakeVisible(playButton_);

    // Stop button
    stopButton_.onClick = [this] {
        transport_.stop();
        transport_.reset();
        updateUI();
    };
    addAndMakeVisible(stopButton_);

    // Regenerate button
    regenerateButton_.onClick = [this] {
        audioEngine_.regeneratePatterns(static_cast<float>(variationSlider_.getValue()));
    };
    addAndMakeVisible(regenerateButton_);

    // Tempo slider
    tempoSlider_.setRange(80.0, 180.0, 0.1);
    tempoSlider_.setValue(128.0);
    tempoSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    tempoSlider_.onValueChange = [this] {
        transport_.setTempo(tempoSlider_.getValue());
    };
    addAndMakeVisible(tempoSlider_);
    tempoLabel_.attachToComponent(&tempoSlider_, true);
    addAndMakeVisible(tempoLabel_);

    // Variation slider
    variationSlider_.setRange(0.0, 1.0, 0.01);
    variationSlider_.setValue(0.1);
    variationSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    addAndMakeVisible(variationSlider_);
    variationLabel_.attachToComponent(&variationSlider_, true);
    addAndMakeVisible(variationLabel_);

    using Role = TechnoMachine::Role;

    // Timeline (Hi-Hat) level
    timelineSlider_.setRange(0.0, 1.0, 0.01);
    timelineSlider_.setValue(0.5);
    timelineSlider_.setSliderStyle(juce::Slider::LinearVertical);
    timelineSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 18);
    timelineSlider_.onValueChange = [this] {
        audioEngine_.drums().setLevel(Role::TIMELINE, static_cast<float>(timelineSlider_.getValue()));
    };
    addAndMakeVisible(timelineSlider_);
    addAndMakeVisible(timelineLabel_);

    // Foundation (Kick) level
    foundationSlider_.setRange(0.0, 1.5, 0.01);
    foundationSlider_.setValue(1.0);
    foundationSlider_.setSliderStyle(juce::Slider::LinearVertical);
    foundationSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 18);
    foundationSlider_.onValueChange = [this] {
        audioEngine_.drums().setLevel(Role::FOUNDATION, static_cast<float>(foundationSlider_.getValue()));
    };
    addAndMakeVisible(foundationSlider_);
    addAndMakeVisible(foundationLabel_);

    // Groove (Clap) level
    grooveSlider_.setRange(0.0, 1.0, 0.01);
    grooveSlider_.setValue(0.7);
    grooveSlider_.setSliderStyle(juce::Slider::LinearVertical);
    grooveSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 18);
    grooveSlider_.onValueChange = [this] {
        audioEngine_.drums().setLevel(Role::GROOVE, static_cast<float>(grooveSlider_.getValue()));
    };
    addAndMakeVisible(grooveSlider_);
    addAndMakeVisible(grooveLabel_);

    // Lead (Rim) level
    leadSlider_.setRange(0.0, 1.0, 0.01);
    leadSlider_.setValue(0.5);
    leadSlider_.setSliderStyle(juce::Slider::LinearVertical);
    leadSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 18);
    leadSlider_.onValueChange = [this] {
        audioEngine_.drums().setLevel(Role::LEAD, static_cast<float>(leadSlider_.getValue()));
    };
    addAndMakeVisible(leadSlider_);
    addAndMakeVisible(leadLabel_);

    // === Density sliders ===
    // (header drawn in paint())

    // Timeline density（即時控制，不重新生成 pattern）
    timelineDensitySlider_.setRange(0.0, 1.0, 0.01);
    timelineDensitySlider_.setValue(1.0);
    timelineDensitySlider_.setSliderStyle(juce::Slider::LinearVertical);
    timelineDensitySlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 18);
    timelineDensitySlider_.onValueChange = [this] {
        audioEngine_.setPlaybackDensity(Role::TIMELINE, static_cast<float>(timelineDensitySlider_.getValue()));
    };
    addAndMakeVisible(timelineDensitySlider_);

    // Foundation density
    foundationDensitySlider_.setRange(0.0, 1.0, 0.01);
    foundationDensitySlider_.setValue(1.0);
    foundationDensitySlider_.setSliderStyle(juce::Slider::LinearVertical);
    foundationDensitySlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 18);
    foundationDensitySlider_.onValueChange = [this] {
        audioEngine_.setPlaybackDensity(Role::FOUNDATION, static_cast<float>(foundationDensitySlider_.getValue()));
    };
    addAndMakeVisible(foundationDensitySlider_);

    // Groove density
    grooveDensitySlider_.setRange(0.0, 1.0, 0.01);
    grooveDensitySlider_.setValue(1.0);
    grooveDensitySlider_.setSliderStyle(juce::Slider::LinearVertical);
    grooveDensitySlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 18);
    grooveDensitySlider_.onValueChange = [this] {
        audioEngine_.setPlaybackDensity(Role::GROOVE, static_cast<float>(grooveDensitySlider_.getValue()));
    };
    addAndMakeVisible(grooveDensitySlider_);

    // Lead density
    leadDensitySlider_.setRange(0.0, 1.0, 0.01);
    leadDensitySlider_.setValue(1.0);
    leadDensitySlider_.setSliderStyle(juce::Slider::LinearVertical);
    leadDensitySlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 18);
    leadDensitySlider_.onValueChange = [this] {
        audioEngine_.setPlaybackDensity(Role::LEAD, static_cast<float>(leadDensitySlider_.getValue()));
    };
    addAndMakeVisible(leadDensitySlider_);

    // === DJ Set controls ===

    // Generate Set button
    generateSetButton_.onClick = [this] {
        audioEngine_.generateRandomSet(8);  // 8 songs in set
        updateDJInfo();
    };
    addAndMakeVisible(generateSetButton_);

    // Next Song button
    nextSongButton_.onClick = [this] {
        audioEngine_.triggerNextSong();
    };
    addAndMakeVisible(nextSongButton_);

    // Style label
    styleLabel_.setFont(juce::FontOptions(18.0f).withStyle("Bold"));
    styleLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff00ffaa));
    addAndMakeVisible(styleLabel_);

    // Song info label
    songInfoLabel_.setFont(juce::FontOptions(12.0f));
    songInfoLabel_.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(songInfoLabel_);

    // Transition progress bar
    transitionProgress_.setColour(juce::ProgressBar::foregroundColourId, juce::Colour(0xff00aaff));
    addAndMakeVisible(transitionProgress_);

    // Song bars slider
    songBarsSlider_.setRange(8, 128, 1);
    songBarsSlider_.setValue(32);
    songBarsSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    songBarsSlider_.onValueChange = [this] {
        audioEngine_.setSongDuration(static_cast<int>(songBarsSlider_.getValue()));
    };
    addAndMakeVisible(songBarsSlider_);
    addAndMakeVisible(songBarsLabel_);

    // Transition bars slider
    transitionBarsSlider_.setRange(2, 32, 1);
    transitionBarsSlider_.setValue(8);
    transitionBarsSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    transitionBarsSlider_.onValueChange = [this] {
        audioEngine_.setTransitionDuration(static_cast<int>(transitionBarsSlider_.getValue()));
    };
    addAndMakeVisible(transitionBarsSlider_);
    addAndMakeVisible(transitionBarsLabel_);

    // Status label
    statusLabel_.setJustificationType(juce::Justification::centred);
    statusLabel_.setFont(juce::FontOptions(28.0f));
    addAndMakeVisible(statusLabel_);

    transport_.setTempo(128.0);
    updateDJInfo();

    startTimerHz(30);
    updateUI();
}

MainComponent::~MainComponent()
{
    stopTimer();
    shutdownAudio();
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
    g.fillAll(juce::Colour(0xff1a1a2e));

    // Title
    g.setColour(juce::Colours::white);
    g.setFont(36.0f);
    g.drawText("Techno Machine", getLocalBounds().removeFromTop(60), juce::Justification::centred);

    // Section headers - positioned at bottom area
    g.setFont(13.0f);
    int headerY = getHeight() - 175;

    int densityStartX = 20 + (55 + 10) * 4 + 40;
    g.drawText("DENSITY", densityStartX, headerY, 80, 18, juce::Justification::left);

    int djX = densityStartX + (55 + 10) * 4 + 40;
    g.drawText("DJ SET", djX, headerY, 80, 18, juce::Justification::left);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(60);

    // Transport controls
    auto controlArea = area.removeFromTop(40);
    playButton_.setBounds(controlArea.removeFromLeft(80));
    controlArea.removeFromLeft(10);
    stopButton_.setBounds(controlArea.removeFromLeft(80));
    controlArea.removeFromLeft(10);
    regenerateButton_.setBounds(controlArea.removeFromLeft(100));
    controlArea.removeFromLeft(60);
    tempoSlider_.setBounds(controlArea.removeFromLeft(250));

    area.removeFromTop(15);

    // Variation slider
    auto variationArea = area.removeFromTop(30);
    variationArea.removeFromLeft(80);
    variationSlider_.setBounds(variationArea.removeFromLeft(250));

    area.removeFromTop(20);

    // Status
    statusLabel_.setBounds(area.removeFromTop(50));

    area.removeFromTop(20);

    // Mixer faders (4 channels) + Density faders
    auto mixerArea = area.removeFromTop(180);
    mixerArea.removeFromTop(25);

    int faderWidth = 55;
    int faderSpacing = 10;
    int sectionSpacing = 40;

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
    int djX = densityStartX + (faderWidth + faderSpacing) * 4 + 40;
    int djY = getHeight() - 155;

    generateSetButton_.setBounds(djX, djY, 95, 28);
    nextSongButton_.setBounds(djX + 100, djY, 95, 28);

    styleLabel_.setBounds(djX, djY + 35, 195, 22);
    songInfoLabel_.setBounds(djX, djY + 55, 195, 18);
    transitionProgress_.setBounds(djX, djY + 75, 195, 10);

    // Settings sliders
    songBarsLabel_.setBounds(djX, djY + 90, 65, 18);
    songBarsSlider_.setBounds(djX + 65, djY + 90, 130, 18);

    transitionBarsLabel_.setBounds(djX, djY + 110, 65, 18);
    transitionBarsSlider_.setBounds(djX + 65, djY + 110, 130, 18);
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
    auto& te = audioEngine_.transitionEngine();
    auto& sm = te.getSongManager();

    // Style name
    const char* styleName = audioEngine_.getStyleName();
    styleLabel_.setText(juce::String("Style: ") + styleName, juce::dontSendNotification);

    // Song info
    int songIdx = sm.getCurrentSongIdx() + 1;
    int totalSongs = sm.getSongCount();
    int barsInSong = sm.getBarsInCurrentSong();
    const auto& currentSong = sm.getCurrentSong();

    juce::String songInfo;
    if (te.isTransitioning()) {
        songInfo = juce::String::formatted("Song %d/%d - TRANSITIONING...", songIdx, totalSongs);
    } else {
        songInfo = juce::String::formatted("Song %d/%d | Bar %d/%d",
                                           songIdx, totalSongs, barsInSong, currentSong.durationBars);
    }
    songInfoLabel_.setText(songInfo, juce::dontSendNotification);

    // Transition progress
    transitionProgressValue_ = static_cast<double>(te.getTransitionProgress());
    transitionProgress_.setVisible(te.isTransitioning());
}
