#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize(700, 450);

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

    // Status label
    statusLabel_.setJustificationType(juce::Justification::centred);
    statusLabel_.setFont(juce::Font(28.0f));
    addAndMakeVisible(statusLabel_);

    transport_.setTempo(128.0);

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

    // Mixer section header
    g.setFont(14.0f);
    g.drawText("MIXER", 30, 180, 100, 20, juce::Justification::left);
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

    // Mixer faders (4 channels)
    auto mixerArea = area.removeFromTop(180);
    mixerArea.removeFromTop(25);

    int faderWidth = 70;
    int faderSpacing = 25;

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
}

void MainComponent::timerCallback()
{
    updateUI();
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
