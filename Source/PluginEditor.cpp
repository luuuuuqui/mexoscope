#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UiTheme.h"

MexoscopeAudioProcessorEditor::MexoscopeAudioProcessorEditor(MexoscopeAudioProcessor& p)
    : juce::AudioProcessorEditor(&p),
      audioProcessor(p),
      effect(audioProcessor.mexoscope),
      tooltipWindow(this, 700),
      waveDisplay(effect)
{
    setLookAndFeel(&lookAndFeel);

    configureKnob(timeKnob, 0.75, "Time window");
    configureKnob(ampKnob, 0.5, "Amplitude window");
    configureKnob(intTrigSpeedKnob, 0.5, "Internal trigger speed");
    configureKnob(retrigThreshKnob, 0.5, "Retrigger threshold");

    retrigLevelSlider.setSliderStyle(juce::Slider::LinearVertical);
    retrigLevelSlider.setRange(0.0, 1.0, 0.0);
    retrigLevelSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    retrigLevelSlider.setPopupDisplayEnabled(false, false, this);
    retrigLevelSlider.setDoubleClickReturnValue(true, 0.5);
    retrigLevelSlider.setTooltip("Trigger level");

    triggerModeBox.addItem("Free", 1);
    triggerModeBox.addItem("Rising", 2);
    triggerModeBox.addItem("Falling", 3);
    triggerModeBox.addItem("Internal", 4);
    triggerModeBox.setTooltip("Trigger mode");

    configureToggle(syncRedrawButton, "Sync Redraw", "Refresh display on trigger only");
    configureToggle(freezeButton, "Freeze", "Freeze waveform rendering");
    configureToggle(dcKillButton, "DC-Kill", "Enable DC offset removal");
    configureToggle(rightChannelButton, "Right Channel", "Toggle left/right monitored channel");

    addAndMakeVisible(waveDisplay);
    addAndMakeVisible(timeKnob);
    addAndMakeVisible(ampKnob);
    addAndMakeVisible(intTrigSpeedKnob);
    addAndMakeVisible(retrigThreshKnob);
    addAndMakeVisible(retrigLevelSlider);
    addAndMakeVisible(triggerModeBox);
    addAndMakeVisible(syncRedrawButton);
    addAndMakeVisible(freezeButton);
    addAndMakeVisible(dcKillButton);
    addAndMakeVisible(rightChannelButton);

    timeKnob.setValue(effect.getParameter(Mexoscope::kTimeWindow));
    ampKnob.setValue(effect.getParameter(Mexoscope::kAmpWindow));
    intTrigSpeedKnob.setValue(effect.getParameter(Mexoscope::kTriggerSpeed));
    retrigThreshKnob.setValue(effect.getParameter(Mexoscope::kTriggerLimit));
    retrigLevelSlider.setValue(effect.getParameter(Mexoscope::kTriggerLevel));

    const int triggerIndex = juce::jlimit(0, Mexoscope::kNumTriggerTypes - 1,
                                          int(effect.getParameter(Mexoscope::kTriggerType) * float(Mexoscope::kNumTriggerTypes) + 0.0001f));
    triggerModeBox.setSelectedItemIndex(triggerIndex, juce::dontSendNotification);

    syncRedrawButton.setToggleState(effect.getParameter(Mexoscope::kSyncDraw) > 0.5f, juce::dontSendNotification);
    freezeButton.setToggleState(effect.getParameter(Mexoscope::kFreeze) > 0.5f, juce::dontSendNotification);
    dcKillButton.setToggleState(effect.getParameter(Mexoscope::kDCKill) > 0.5f, juce::dontSendNotification);
    rightChannelButton.setToggleState(effect.getParameter(Mexoscope::kChannel) > 0.5f, juce::dontSendNotification);

    constrainer.setSizeLimits(840, 400, 1800, 1000);
    setResizable(true, true);
    setConstrainer(&constrainer);
    setSize(1024, 480);

    updateParameters();
    startTimerHz(30);
}

MexoscopeAudioProcessorEditor::~MexoscopeAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void MexoscopeAudioProcessorEditor::configureKnob(juce::Slider& knob, const double defaultValue,
                                                   const juce::String& tooltip)
{
    knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setRange(0.0, 1.0, 0.0);
    knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    knob.setPopupDisplayEnabled(false, false, this);
    knob.setDoubleClickReturnValue(true, defaultValue);
    knob.setTooltip(tooltip);
    knob.setRotaryParameters(juce::MathConstants<float>::pi * 1.15f,
                             juce::MathConstants<float>::pi * 2.85f,
                             true);
}

void MexoscopeAudioProcessorEditor::configureToggle(juce::ToggleButton& button, const juce::String& text,
                                                     const juce::String& tooltip)
{
    button.setButtonText(text);
    button.setTooltip(tooltip);
    button.setClickingTogglesState(true);
}

juce::String MexoscopeAudioProcessorEditor::formatMetricValue(const float value) const
{
    if (value < 1000.0f) {
        return juce::String(value, 3);
    }

    return juce::String(int(value));
}

juce::String MexoscopeAudioProcessorEditor::formatAnalysisValue(const float value, const int decimals) const
{
    return juce::String(value, decimals);
}

void MexoscopeAudioProcessorEditor::drawSection(juce::Graphics& g, const juce::Rectangle<int> bounds,
                                                 const juce::String& title) const
{
    const auto section = bounds.toFloat();
    g.setColour(ui::kPanelColour);
    g.fillRoundedRectangle(section, ui::kCardCorner);

    g.setColour(ui::kPanelEdgeColour);
    g.drawRoundedRectangle(section.reduced(0.5f), ui::kCardCorner, 1.0f);

    g.setColour(ui::kMutedTextColour);
    g.setFont(ui::headingFont());
    g.drawText(title, bounds.reduced(ui::kSectionPadding, 0).removeFromTop(24), juce::Justification::centredLeft, false);
}

void MexoscopeAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(ui::kBackgroundColour);

    drawSection(g, displaySection, "Display");
    drawSection(g, triggerSection, "Trigger");
    drawSection(g, optionsSection, "Options");
    drawSection(g, analysisSection, "Analysis");

    g.setColour(ui::kMutedTextColour);
    g.setFont(ui::labelFont());

    g.drawText("Time", timeLabelBounds, juce::Justification::centred, false);
    g.drawText("Amp", ampLabelBounds, juce::Justification::centred, false);
    g.drawText("Speed", speedLabelBounds, juce::Justification::centred, false);
    g.drawText("Thresh", threshLabelBounds, juce::Justification::centred, false);
    g.drawText("Mode", triggerModeLabelBounds, juce::Justification::centredLeft, false);
    g.drawText("Level", triggerLevelLabelBounds, juce::Justification::centred, false);

    g.setColour(ui::kTextColour);
    g.setFont(ui::valueFont());
    g.drawText(timeValueText, timeValueBounds, juce::Justification::centred, false);
    g.drawText(ampValueText, ampValueBounds, juce::Justification::centred, false);
    g.drawText(speedValueText, speedValueBounds, juce::Justification::centred, false);
    g.drawText(threshValueText, threshValueBounds, juce::Justification::centred, false);

    auto rowsArea = analysisSection.reduced(ui::kSectionPadding);
    rowsArea.removeFromTop(26);

    const juce::String labels[] = { "Y (lin)", "Y (dB)", "X (samples)", "X (seconds)", "X (ms)", "X (Hz)" };
    const juce::String values[] = { analysisYText, analysisYDbText, analysisSamplesText,
                                    analysisSecondsText, analysisMsText, analysisHzText };

    const int rowHeight = juce::jmax(14, rowsArea.getHeight() / 6);

    g.setFont(ui::monoFont());
    for (int i = 0; i < 6; ++i) {
        auto row = rowsArea.removeFromTop(rowHeight);
        auto left = row.removeFromLeft(int(float(row.getWidth()) * 0.44f));

        g.setColour(ui::kMutedTextColour);
        g.drawText(labels[i], left, juce::Justification::centredLeft, false);

        g.setColour(ui::kTextColour);
        g.drawText(values[i], row, juce::Justification::centredRight, false);
    }
}

void MexoscopeAudioProcessorEditor::resized()
{
    auto content = getLocalBounds().reduced(ui::kOuterPadding);

    const int sidebarWidth = juce::jlimit(260, 360, int(std::round(float(content.getWidth()) * 0.30f)));
    auto sidebar = content.removeFromRight(sidebarWidth);
    content.removeFromRight(ui::kSectionGap);

    waveDisplay.setBounds(content);

    const int gap = ui::kSectionGap;
    const int availableHeight = juce::jmax(0, sidebar.getHeight() - gap * 3);

    int displayHeight = int(std::round(float(availableHeight) * 0.21f));
    int triggerHeight = int(std::round(float(availableHeight) * 0.32f));
    int optionsHeight = int(std::round(float(availableHeight) * 0.25f));
    int analysisHeight = availableHeight - displayHeight - triggerHeight - optionsHeight;

    displaySection = sidebar.removeFromTop(displayHeight);
    sidebar.removeFromTop(gap);
    triggerSection = sidebar.removeFromTop(triggerHeight);
    sidebar.removeFromTop(gap);
    optionsSection = sidebar.removeFromTop(optionsHeight);
    sidebar.removeFromTop(gap);
    analysisSection = sidebar.withHeight(analysisHeight);

    auto displayInner = displaySection.reduced(ui::kSectionPadding);
    displayInner.removeFromTop(24);

    auto displayRows = displayInner;
    const int displayGap = 8;
    auto timeColumn = displayRows.removeFromLeft((displayRows.getWidth() - displayGap) / 2);
    displayRows.removeFromLeft(displayGap);
    auto ampColumn = displayRows;

    const int labelHeight = 14;
    const int valueHeight = 16;

    const int knobSize = juce::jlimit(30, 72, juce::jmin(timeColumn.getWidth(), timeColumn.getHeight() - labelHeight - valueHeight));

    timeKnob.setBounds(timeColumn.removeFromTop(knobSize).withSizeKeepingCentre(knobSize, knobSize));
    timeLabelBounds = timeColumn.removeFromTop(labelHeight);
    timeValueBounds = timeColumn.removeFromTop(valueHeight);

    ampKnob.setBounds(ampColumn.removeFromTop(knobSize).withSizeKeepingCentre(knobSize, knobSize));
    ampLabelBounds = ampColumn.removeFromTop(labelHeight);
    ampValueBounds = ampColumn.removeFromTop(valueHeight);

    auto triggerInner = triggerSection.reduced(ui::kSectionPadding);
    triggerInner.removeFromTop(24);

    auto modeRow = triggerInner.removeFromTop(28);
    triggerModeLabelBounds = modeRow.removeFromLeft(52);
    triggerModeBox.setBounds(modeRow);

    triggerInner.removeFromTop(8);

    auto triggerControls = triggerInner;
    auto levelColumn = triggerControls.removeFromRight(30);
    triggerControls.removeFromRight(8);

    triggerLevelLabelBounds = levelColumn.removeFromTop(14);
    retrigLevelSlider.setBounds(levelColumn.reduced(4, 0));

    auto speedColumn = triggerControls.removeFromLeft((triggerControls.getWidth() - displayGap) / 2);
    triggerControls.removeFromLeft(displayGap);
    auto threshColumn = triggerControls;

    const int triggerKnobSize = juce::jlimit(28, 64, juce::jmin(speedColumn.getWidth(), speedColumn.getHeight() - labelHeight - valueHeight));

    intTrigSpeedKnob.setBounds(speedColumn.removeFromTop(triggerKnobSize).withSizeKeepingCentre(triggerKnobSize, triggerKnobSize));
    speedLabelBounds = speedColumn.removeFromTop(labelHeight);
    speedValueBounds = speedColumn.removeFromTop(valueHeight);

    retrigThreshKnob.setBounds(threshColumn.removeFromTop(triggerKnobSize).withSizeKeepingCentre(triggerKnobSize, triggerKnobSize));
    threshLabelBounds = threshColumn.removeFromTop(labelHeight);
    threshValueBounds = threshColumn.removeFromTop(valueHeight);

    auto optionsInner = optionsSection.reduced(ui::kSectionPadding);
    optionsInner.removeFromTop(24);

    const int optionGap = 4;
    const int optionHeight = juce::jmax(18, (optionsInner.getHeight() - optionGap * 3) / 4);

    syncRedrawButton.setBounds(optionsInner.removeFromTop(optionHeight));
    optionsInner.removeFromTop(optionGap);
    freezeButton.setBounds(optionsInner.removeFromTop(optionHeight));
    optionsInner.removeFromTop(optionGap);
    dcKillButton.setBounds(optionsInner.removeFromTop(optionHeight));
    optionsInner.removeFromTop(optionGap);
    rightChannelButton.setBounds(optionsInner.removeFromTop(optionHeight));
}

void MexoscopeAudioProcessorEditor::timerCallback()
{
    waveDisplay.repaint();
    updateParameters();
}

void MexoscopeAudioProcessorEditor::updateParameters()
{
    effect.setParameter(Mexoscope::kTimeWindow, float(timeKnob.getValue()));
    effect.setParameter(Mexoscope::kAmpWindow, float(ampKnob.getValue()));
    effect.setParameter(Mexoscope::kTriggerSpeed, float(intTrigSpeedKnob.getValue()));
    effect.setParameter(Mexoscope::kTriggerLimit, float(retrigThreshKnob.getValue()));
    effect.setParameter(Mexoscope::kTriggerLevel, float(retrigLevelSlider.getValue()));

    const int selectedMode = juce::jmax(0, triggerModeBox.getSelectedItemIndex());
    effect.setParameter(Mexoscope::kTriggerType,
                        float(selectedMode) / float(Mexoscope::kNumTriggerTypes));

    effect.setParameter(Mexoscope::kSyncDraw, syncRedrawButton.getToggleState() ? 1.0f : 0.0f);
    effect.setParameter(Mexoscope::kFreeze, freezeButton.getToggleState() ? 1.0f : 0.0f);
    effect.setParameter(Mexoscope::kDCKill, dcKillButton.getToggleState() ? 1.0f : 0.0f);
    effect.setParameter(Mexoscope::kChannel, rightChannelButton.getToggleState() ? 1.0f : 0.0f);

    timeValueText = formatMetricValue(float(std::pow(10.0, 1.5 - timeKnob.getValue() * 5.0)));
    ampValueText = formatMetricValue(float(std::pow(10.0, ampKnob.getValue() * 6.0 - 3.0)));

    const double triggerSpeed = std::pow(10.0, intTrigSpeedKnob.getValue() * 2.5 - 5.0);
    speedValueText = formatMetricValue(float(triggerSpeed * effect.getSampleRate()));
    threshValueText = formatMetricValue(float(std::pow(10.0, retrigThreshKnob.getValue() * 4.0)));

    const auto cursorMetrics = waveDisplay.getCursorMetrics();
    if (cursorMetrics.has_value()) {
        analysisYText = formatAnalysisValue(cursorMetrics->yLinear, 5);
        analysisYDbText = formatAnalysisValue(cursorMetrics->yDb, 4);
        analysisSamplesText = formatAnalysisValue(cursorMetrics->xSamples, 2);
        analysisSecondsText = formatAnalysisValue(cursorMetrics->xSeconds, 5);
        analysisMsText = formatAnalysisValue(cursorMetrics->xMs, 3);
        analysisHzText = cursorMetrics->infiniteHz ? "infinite" : formatAnalysisValue(cursorMetrics->xHz, 3);
    } else {
        analysisYText = "--";
        analysisYDbText = "--";
        analysisSamplesText = "--";
        analysisSecondsText = "--";
        analysisMsText = "--";
        analysisHzText = "--";
    }

    repaint();
}
