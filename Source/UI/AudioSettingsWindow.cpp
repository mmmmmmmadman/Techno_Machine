#include "AudioSettingsWindow.h"

//==============================================================================
// CVRoutingPanel
//==============================================================================

CVRoutingPanel::CVRoutingPanel(TechnoMachine::CVOutputRouter& cvRouter,
                               juce::AudioDeviceManager& deviceManager)
    : cvRouter_(cvRouter), deviceManager_(deviceManager)
{
    // 建立 12 個 ComboBox (4 roles × 3 CV types)
    for (int role = 0; role < NUM_ROLES; ++role) {
        for (int cvType = 0; cvType < CV_TYPES; ++cvType) {
            int signalIdx = role * CV_TYPES + cvType;

            // ComboBox only (labels drawn in paint)
            auto* combo = new juce::ComboBox();
            combo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff201a1a));
            combo->setColour(juce::ComboBox::textColourId, juce::Colour(0xffffffff));
            combo->setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff302828));
            combo->onChange = [this, signalIdx, combo]() {
                onChannelChanged(signalIdx, combo->getSelectedId());
            };
            addAndMakeVisible(combo);
            channelSelectors_.add(combo);
        }
    }

    // 監聽 device 變更
    deviceManager_.addChangeListener(this);

    // 初始化通道選項
    updateAvailableChannels();
}

CVRoutingPanel::~CVRoutingPanel()
{
    deviceManager_.removeChangeListener(this);
}

void CVRoutingPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0e0c0c));

    int roleHeight = 40;
    int startY = 8;
    int labelX = 8;
    int comboStartX = 80;
    int comboWidth = 50;
    int comboSpacing = 4;

    // Column headers (Trig, Pitch, Vel)
    g.setColour(juce::Colour(0xffa09098));
    g.setFont(11.0f);
    for (int cvType = 0; cvType < CV_TYPES; ++cvType) {
        int x = comboStartX + cvType * (comboWidth + comboSpacing);
        g.drawText(cvTypeNames_[cvType], x, startY, comboWidth, 12, juce::Justification::centred);
    }

    for (int role = 0; role < NUM_ROLES; ++role) {
        int roleY = startY + 14 + role * roleHeight;

        // Role name
        g.setColour(juce::Colour(0xffff9eb0));
        g.setFont(12.0f);
        g.drawText(roleNames_[role], labelX, roleY + 8, 70, 16, juce::Justification::centredLeft);

        // 分隔線
        if (role < NUM_ROLES - 1) {
            g.setColour(juce::Colour(0xff302828));
            g.drawLine(8.0f, static_cast<float>(roleY + roleHeight - 2),
                      static_cast<float>(getWidth() - 8),
                      static_cast<float>(roleY + roleHeight - 2), 0.5f);
        }
    }
}

void CVRoutingPanel::resized()
{
    int roleHeight = 40;
    int startY = 8;
    int comboStartX = 80;
    int comboWidth = 50;
    int comboHeight = 18;
    int comboSpacing = 4;

    for (int role = 0; role < NUM_ROLES; ++role) {
        int roleY = startY + 14 + role * roleHeight + 6;

        for (int cvType = 0; cvType < CV_TYPES; ++cvType) {
            int idx = role * CV_TYPES + cvType;
            int x = comboStartX + cvType * (comboWidth + comboSpacing);

            if (idx < channelSelectors_.size()) {
                channelSelectors_[idx]->setBounds(x, roleY, comboWidth, comboHeight);
            }
        }
    }
}

void CVRoutingPanel::changeListenerCallback(juce::ChangeBroadcaster*)
{
    updateAvailableChannels();
}

int CVRoutingPanel::getAvailableChannelCount() const
{
    if (auto* device = deviceManager_.getCurrentAudioDevice()) {
        return device->getActiveOutputChannels().countNumberOfSetBits();
    }
    return 2;  // 預設立體聲
}

void CVRoutingPanel::updateAvailableChannels()
{
    int numChannels = getAvailableChannelCount();

    for (int i = 0; i < channelSelectors_.size(); ++i) {
        auto* combo = channelSelectors_[i];

        combo->clear(juce::dontSendNotification);
        combo->addItem("Off", 1);  // ID 1 = Off (-1)

        // 通道 2 開始（0-1 是立體聲音訊）
        for (int ch = 2; ch < numChannels; ++ch) {
            combo->addItem(juce::String(ch), ch + 2);  // ID = channel + 2
        }

        // 還原選擇
        int route = cvRouter_.getRoute(i);
        if (route < 0) {
            combo->setSelectedId(1, juce::dontSendNotification);  // Off
        } else if (route < numChannels) {
            combo->setSelectedId(route + 2, juce::dontSendNotification);
        } else {
            combo->setSelectedId(1, juce::dontSendNotification);  // 超出範圍設為 Off
        }
    }
}

void CVRoutingPanel::onChannelChanged(int signalIdx, int comboBoxId)
{
    int channel = (comboBoxId == 1) ? -1 : (comboBoxId - 2);
    cvRouter_.setRoute(signalIdx, channel);
}

//==============================================================================
// AudioSettingsComponent
//==============================================================================

AudioSettingsComponent::AudioSettingsComponent(juce::AudioDeviceManager& deviceManager,
                                               TechnoMachine::CVOutputRouter& cvRouter)
    : deviceManager_(deviceManager), cvRouter_(cvRouter)
{
    // Audio Device Selector
    deviceSelector_ = std::make_unique<juce::AudioDeviceSelectorComponent>(
        deviceManager_,
        0, 0,     // min/max input channels
        0, 256,   // min/max output channels (high for CV support)
        false,    // show MIDI input
        false,    // show MIDI output
        false,    // show channels as stereo pairs
        false     // hide advanced options
    );

    // CV Routing Panel
    cvRoutingPanel_ = std::make_unique<CVRoutingPanel>(cvRouter_, deviceManager_);

    // Tabs
    tabs_.setColour(juce::TabbedComponent::backgroundColourId, juce::Colour(0xff0e0c0c));
    tabs_.setColour(juce::TabbedComponent::outlineColourId, juce::Colour(0xff302828));

    tabs_.addTab("Audio Device", juce::Colour(0xff151212), deviceSelector_.get(), false);
    tabs_.addTab("CV Routing", juce::Colour(0xff151212), cvRoutingPanel_.get(), false);

    addAndMakeVisible(tabs_);
}

AudioSettingsComponent::~AudioSettingsComponent()
{
}

void AudioSettingsComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0e0c0c));
}

void AudioSettingsComponent::resized()
{
    tabs_.setBounds(getLocalBounds());
}

//==============================================================================
// AudioSettingsWindow
//==============================================================================

AudioSettingsWindow::AudioSettingsWindow(juce::AudioDeviceManager& deviceManager,
                                         TechnoMachine::CVOutputRouter& cvRouter)
    : DocumentWindow("Audio & CV Settings",
                    juce::Colour(0xff0e0c0c),
                    DocumentWindow::closeButton)
{
    setContentOwned(new AudioSettingsComponent(deviceManager, cvRouter), true);
    setResizable(true, false);
    centreWithSize(420, 300);
    setVisible(true);
}

void AudioSettingsWindow::closeButtonPressed()
{
    setVisible(false);
}
