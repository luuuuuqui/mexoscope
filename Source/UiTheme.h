#pragma once

#include <JuceHeader.h>

namespace ui {
inline const juce::Colour kBackgroundColour { 0xFF121417 };
inline const juce::Colour kPanelColour { 0xFF1B1F24 };
inline const juce::Colour kPanelEdgeColour { 0xFF2B323A };
inline const juce::Colour kTextColour { 0xFFE7EBEE };
inline const juce::Colour kMutedTextColour { 0xFF9AA3AB };
inline const juce::Colour kAccentColour { 0xFFF2A93B };
inline const juce::Colour kZeroLineColour { 0xFFC27A2C };
inline const juce::Colour kWaveInterpolatedColour { 0xFFF2A93B };
inline const juce::Colour kWaveDenseColour { 0xFF8A939C };
inline const juce::Colour kScopeBackgroundColour { 0xFF151A20 };
inline const juce::Colour kScopeGridColour { 0xFF242C35 };
inline const juce::Colour kTriggerLineColour { 0xFF5A646E };

inline constexpr int kOuterPadding = 16;
inline constexpr int kSectionPadding = 12;
inline constexpr int kSectionGap = 12;
inline constexpr float kCardCorner = 12.0f;
inline constexpr float kScopePadding = 12.0f;

inline juce::Font headingFont()
{
    return juce::Font(15.0f, juce::Font::bold);
}

inline juce::Font labelFont()
{
    return juce::Font(13.0f, juce::Font::plain);
}

inline juce::Font valueFont()
{
    return juce::Font(13.0f, juce::Font::bold);
}

inline juce::Font monoFont()
{
    return juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::plain);
}
}
