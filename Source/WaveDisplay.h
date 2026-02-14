#pragma once

#include <JuceHeader.h>
#include <optional>
#include "Defines.h"
#include "Smexoscope.h"

class WaveDisplay : public juce::Component
{
public:
    struct CursorMetrics
    {
        float yLinear = 0.0f;
        float yDb = 0.0f;
        float xSamples = 0.0f;
        float xSeconds = 0.0f;
        float xMs = 0.0f;
        float xHz = 0.0f;
        bool infiniteHz = false;
    };

    explicit WaveDisplay(Smexoscope& effect);

    void paint(juce::Graphics& g) override;

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    std::optional<CursorMetrics> getCursorMetrics() const;

private:
    static float linToDb(float linear);

    juce::Rectangle<float> getScopeArea() const;
    juce::Point<int> clampToScope(juce::Point<int> position) const;
    float scopeXToSamples(float xInScope, double samplesPerPixel) const;
    float scopeYToLinear(float yInScope) const;

    Smexoscope& effect;

    juce::Point<int> where { -1, -1 };
    std::optional<CursorMetrics> cursorMetrics;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveDisplay)
};
