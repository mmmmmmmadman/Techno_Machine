#pragma once

#include <JuceHeader.h>
#include "../Core/CVOutputRouter.h"

/**
 * CV Routing Panel - 12 個 CV 輸出通道選擇器
 */
class CVRoutingPanel : public juce::Component,
                       public juce::ChangeListener
{
public:
    CVRoutingPanel(TechnoMachine::CVOutputRouter& cvRouter,
                   juce::AudioDeviceManager& deviceManager);
    ~CVRoutingPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    TechnoMachine::CVOutputRouter& cvRouter_;
    juce::AudioDeviceManager& deviceManager_;

    static constexpr int NUM_ROLES = 4;
    static constexpr int CV_TYPES = 3;  // Trigger, Pitch, Velocity

    // Role 名稱
    const char* roleNames_[NUM_ROLES] = {"TIMELINE", "FOUNDATION", "GROOVE", "LEAD"};
    const char* cvTypeNames_[CV_TYPES] = {"Trig", "Pitch", "Vel"};

    // 12 個 ComboBox (4 roles × 3 CV types)
    juce::OwnedArray<juce::ComboBox> channelSelectors_;

    void updateAvailableChannels();
    void onChannelChanged(int signalIdx, int comboBoxId);
    int getAvailableChannelCount() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CVRoutingPanel)
};

/**
 * Audio Settings Component - 包含 Audio Device 和 CV Routing 的 Tab
 */
class AudioSettingsComponent : public juce::Component
{
public:
    AudioSettingsComponent(juce::AudioDeviceManager& deviceManager,
                          TechnoMachine::CVOutputRouter& cvRouter);
    ~AudioSettingsComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioDeviceManager& deviceManager_;
    TechnoMachine::CVOutputRouter& cvRouter_;

    juce::TabbedComponent tabs_{juce::TabbedButtonBar::TabsAtTop};
    std::unique_ptr<juce::AudioDeviceSelectorComponent> deviceSelector_;
    std::unique_ptr<CVRoutingPanel> cvRoutingPanel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioSettingsComponent)
};

/**
 * Audio Settings Window - 獨立的設定視窗
 */
class AudioSettingsWindow : public juce::DocumentWindow
{
public:
    AudioSettingsWindow(juce::AudioDeviceManager& deviceManager,
                       TechnoMachine::CVOutputRouter& cvRouter);

    void closeButtonPressed() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioSettingsWindow)
};
