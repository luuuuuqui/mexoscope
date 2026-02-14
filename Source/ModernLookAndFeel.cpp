#include "ModernLookAndFeel.h"
#include "UiTheme.h"

namespace {
float toFloat(const bool value)
{
    return value ? 1.0f : 0.0f;
}

juce::Colour blend(const juce::Colour base, const juce::Colour overlay, const float amount)
{
    return base.interpolatedWith(overlay, amount);
}
}

ModernLookAndFeel::ModernLookAndFeel()
{
    setColour(juce::ComboBox::backgroundColourId, ui::kPanelColour);
    setColour(juce::ComboBox::outlineColourId, ui::kPanelEdgeColour);
    setColour(juce::ComboBox::textColourId, ui::kTextColour);
    setColour(juce::Slider::thumbColourId, ui::kAccentColour);
    setColour(juce::Slider::rotarySliderFillColourId, ui::kAccentColour);
    setColour(juce::Slider::rotarySliderOutlineColourId, ui::kPanelEdgeColour);
    setColour(juce::TooltipWindow::backgroundColourId, ui::kPanelColour);
    setColour(juce::TooltipWindow::outlineColourId, ui::kPanelEdgeColour);
    setColour(juce::TooltipWindow::textColourId, ui::kTextColour);
}

void ModernLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPosProportional, float rotaryStartAngle,
                                         float rotaryEndAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float>(float(x), float(y), float(width), float(height)).reduced(2.5f);
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    const auto focus = toFloat(slider.hasKeyboardFocus(true));
    const auto hover = toFloat(slider.isMouseOverOrDragging());
    const auto ringColour = blend(ui::kPanelEdgeColour, ui::kAccentColour, juce::jmax(hover * 0.28f, focus * 0.45f));

    g.setColour(ui::kPanelColour.brighter(0.04f));
    g.fillEllipse(bounds);

    g.setColour(ringColour);
    g.drawEllipse(bounds, 1.5f);

    juce::Path valueArc;
    const auto strokeRadius = radius - 4.5f;
    valueArc.addCentredArc(centre.x, centre.y, strokeRadius, strokeRadius, 0.0f,
                           rotaryStartAngle, angle, true);
    g.setColour(ui::kAccentColour);
    g.strokePath(valueArc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path pointer;
    pointer.addRoundedRectangle(-1.6f, -radius + 8.0f, 3.2f, radius * 0.44f, 1.5f);

    g.setColour(ui::kTextColour);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
}

void ModernLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float, float,
                                         const juce::Slider::SliderStyle sliderStyle, juce::Slider& slider)
{
    if (sliderStyle != juce::Slider::LinearVertical && sliderStyle != juce::Slider::LinearBarVertical) {
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, 0.0f, 0.0f, sliderStyle, slider);
        return;
    }

    const auto bounds = juce::Rectangle<float>(float(x), float(y), float(width), float(height));
    const auto rail = bounds.withSizeKeepingCentre(6.0f, bounds.getHeight()).reduced(0.0f, 2.0f);

    g.setColour(ui::kPanelEdgeColour.withAlpha(0.7f));
    g.fillRoundedRectangle(rail, 3.0f);

    const float thumbY = juce::jlimit(bounds.getY() + 7.0f, bounds.getBottom() - 7.0f, sliderPos);
    const auto thumb = juce::Rectangle<float>(bounds.getX(), thumbY - 6.0f, bounds.getWidth(), 12.0f).reduced(1.0f, 0.0f);

    const auto activeFill = toFloat(slider.isMouseOverOrDragging()) * 0.14f + toFloat(slider.hasKeyboardFocus(true)) * 0.2f;
    g.setColour(blend(ui::kAccentColour, juce::Colours::white, activeFill));
    g.fillRoundedRectangle(thumb, 6.0f);
}

void ModernLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                         bool shouldDrawButtonAsHighlighted, bool)
{
    auto bounds = button.getLocalBounds().toFloat();
    const auto toggleBounds = bounds.removeFromLeft(40.0f).reduced(4.0f, 6.0f);
    const auto knobDiameter = toggleBounds.getHeight() - 4.0f;

    const bool enabled = button.getToggleState();

    juce::Colour backColour = enabled ? ui::kAccentColour : ui::kPanelEdgeColour.withAlpha(0.9f);
    if (shouldDrawButtonAsHighlighted) {
        backColour = backColour.brighter(0.08f);
    }

    g.setColour(backColour);
    g.fillRoundedRectangle(toggleBounds, toggleBounds.getHeight() * 0.5f);

    const float knobX = enabled ? (toggleBounds.getRight() - knobDiameter - 2.0f) : (toggleBounds.getX() + 2.0f);
    const auto knob = juce::Rectangle<float>(knobX, toggleBounds.getY() + 2.0f, knobDiameter, knobDiameter);

    g.setColour(ui::kBackgroundColour.brighter(0.08f));
    g.fillEllipse(knob);

    const auto textBounds = button.getLocalBounds().withTrimmedLeft(46);
    g.setColour(ui::kTextColour);
    g.setFont(ui::labelFont());
    g.drawText(button.getButtonText(), textBounds, juce::Justification::centredLeft, false);
}

void ModernLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                     int buttonX, int buttonY, int buttonW, int buttonH,
                                     juce::ComboBox& box)
{
    const auto bounds = juce::Rectangle<float>(0.0f, 0.0f, float(width), float(height));
    const auto focus = toFloat(box.hasKeyboardFocus(true));
    const auto hover = toFloat(box.isMouseOver());
    const auto edge = blend(ui::kPanelEdgeColour, ui::kAccentColour, juce::jmax(hover * 0.2f, focus * 0.5f));

    g.setColour(ui::kPanelColour);
    g.fillRoundedRectangle(bounds, 8.0f);

    g.setColour(edge);
    g.drawRoundedRectangle(bounds.reduced(0.7f), 8.0f, 1.4f);

    juce::Path arrow;
    const auto arrowBounds = juce::Rectangle<float>(float(buttonX), float(buttonY), float(buttonW), float(buttonH)).reduced(9.0f, 11.0f);
    arrow.startNewSubPath(arrowBounds.getX(), arrowBounds.getY());
    arrow.lineTo(arrowBounds.getCentreX(), arrowBounds.getBottom());
    arrow.lineTo(arrowBounds.getRight(), arrowBounds.getY());

    g.setColour(ui::kTextColour.withAlpha(0.9f));
    g.strokePath(arrow, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

juce::Font ModernLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return ui::labelFont();
}

juce::Label* ModernLookAndFeel::createComboBoxTextBox(juce::ComboBox& box)
{
    auto* label = LookAndFeel_V4::createComboBoxTextBox(box);
    label->setJustificationType(juce::Justification::centredLeft);
    label->setColour(juce::Label::textColourId, ui::kTextColour);
    label->setFont(ui::labelFont());
    return label;
}
