#pragma once

#include <JuceHeader.h>
#include "ModernLookAndFeel.h"
#include "PluginProcessor.h"
#include "WaveDisplay.h"

class MexoscopeAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    explicit MexoscopeAudioProcessorEditor(MexoscopeAudioProcessor&);
    ~MexoscopeAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateParameters();

    void configureKnob(juce::Slider& knob, double defaultValue, const juce::String& tooltip);
    void configureToggle(juce::ToggleButton& button, const juce::String& text, const juce::String& tooltip);

    juce::String formatMetricValue(float value) const;
    juce::String formatAnalysisValue(float value, int decimals = 5) const;

    void drawSection(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title) const;

    MexoscopeAudioProcessor& audioProcessor;
    Mexoscope& effect;

    ModernLookAndFeel lookAndFeel;
    juce::TooltipWindow tooltipWindow;
    juce::ComponentBoundsConstrainer constrainer;

    juce::Slider timeKnob;
    juce::Slider ampKnob;
    juce::Slider intTrigSpeedKnob;
    juce::Slider retrigThreshKnob;
    juce::Slider retrigLevelSlider;

    juce::ComboBox triggerModeBox;

    juce::ToggleButton syncRedrawButton;
    juce::ToggleButton freezeButton;
    juce::ToggleButton dcKillButton;
    juce::ToggleButton rightChannelButton;

    WaveDisplay waveDisplay;

    juce::Rectangle<int> displaySection;
    juce::Rectangle<int> triggerSection;
    juce::Rectangle<int> optionsSection;
    juce::Rectangle<int> analysisSection;

    juce::Rectangle<int> timeLabelBounds;
    juce::Rectangle<int> ampLabelBounds;
    juce::Rectangle<int> speedLabelBounds;
    juce::Rectangle<int> threshLabelBounds;
    juce::Rectangle<int> triggerModeLabelBounds;
    juce::Rectangle<int> triggerLevelLabelBounds;

    juce::Rectangle<int> timeValueBounds;
    juce::Rectangle<int> ampValueBounds;
    juce::Rectangle<int> speedValueBounds;
    juce::Rectangle<int> threshValueBounds;

    juce::String timeValueText;
    juce::String ampValueText;
    juce::String speedValueText;
    juce::String threshValueText;

    juce::String analysisYText { "--" };
    juce::String analysisYDbText { "--" };
    juce::String analysisSamplesText { "--" };
    juce::String analysisSecondsText { "--" };
    juce::String analysisMsText { "--" };
    juce::String analysisHzText { "--" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MexoscopeAudioProcessorEditor)
};
