#include "WaveDisplay.h"
#include "UiTheme.h"

namespace {
float mapVirtualYToScope(const juce::Rectangle<float>& scopeArea, const float virtualY)
{
    return scopeArea.getY() + (virtualY / float(OSC_HEIGHT - 1)) * scopeArea.getHeight();
}
}

WaveDisplay::WaveDisplay(Smexoscope& smexoscope)
    : effect(smexoscope)
{
}

float WaveDisplay::linToDb(const float linear)
{
    if (std::abs(double(linear)) < 9e-51) {
        return -1000.0f;
    }

    return 20.0f * std::log10(std::abs(linear));
}

juce::Rectangle<float> WaveDisplay::getScopeArea() const
{
    return getLocalBounds().toFloat().reduced(ui::kScopePadding);
}

juce::Point<int> WaveDisplay::clampToScope(juce::Point<int> position) const
{
    const auto scope = getScopeArea().getSmallestIntegerContainer();
    position.x = juce::jlimit(scope.getX(), scope.getRight() - 1, position.x);
    position.y = juce::jlimit(scope.getY(), scope.getBottom() - 1, position.y);
    return position;
}

float WaveDisplay::scopeXToSamples(const float xInScope, const double samplesPerPixel) const
{
    const auto scope = getScopeArea();
    const float normalized = (scope.getWidth() > 1.0f) ? ((xInScope - scope.getX()) / scope.getWidth()) : 0.0f;
    const float virtualX = normalized * float(OSC_WIDTH);
    return virtualX * float(samplesPerPixel);
}

float WaveDisplay::scopeYToLinear(const float yInScope) const
{
    const auto scope = getScopeArea();
    const float normalized = (scope.getHeight() > 1.0f) ? ((yInScope - scope.getY()) / scope.getHeight()) : 0.5f;
    const float virtualY = normalized * float(OSC_HEIGHT);

    const float gain = std::pow(10.0f, effect.getParameter(Smexoscope::kAmpWindow) * 6.0f - 3.0f);
    return (-2.0f * (virtualY + 1.0f) / float(OSC_HEIGHT) + 1.0f) / gain;
}

void WaveDisplay::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isLeftButtonDown() && event.originalComponent == this) {
        where = clampToScope(event.getPosition());
    }
}

void WaveDisplay::mouseDrag(const juce::MouseEvent& event)
{
    if (event.mods.isLeftButtonDown() && event.originalComponent == this) {
        where = clampToScope(event.getPosition());
    }
}

void WaveDisplay::mouseUp(const juce::MouseEvent& event)
{
    if (event.mods.isRightButtonDown() && event.originalComponent == this) {
        where = { -1, -1 };
        cursorMetrics.reset();
    }
}

std::optional<WaveDisplay::CursorMetrics> WaveDisplay::getCursorMetrics() const
{
    return cursorMetrics;
}

void WaveDisplay::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const auto scopeArea = getScopeArea();

    g.setColour(ui::kPanelColour);
    g.fillRoundedRectangle(bounds, ui::kCardCorner);
    g.setColour(ui::kPanelEdgeColour);
    g.drawRoundedRectangle(bounds.reduced(0.5f), ui::kCardCorner, 1.0f);

    g.setColour(ui::kScopeBackgroundColour);
    g.fillRoundedRectangle(scopeArea, 10.0f);

    g.reduceClipRegion(scopeArea.getSmallestIntegerContainer());

    g.setColour(ui::kScopeGridColour);
    constexpr float gridX = 24.0f;
    constexpr float gridY = 20.0f;

    for (float x = scopeArea.getX(); x <= scopeArea.getRight(); x += gridX) {
        g.drawVerticalLine(int(x), scopeArea.getY(), scopeArea.getBottom());
    }

    for (float y = scopeArea.getY(); y <= scopeArea.getBottom(); y += gridY) {
        g.drawHorizontalLine(int(y), scopeArea.getX(), scopeArea.getRight());
    }

    const int triggerType = int(effect.getParameter(Smexoscope::kTriggerType) * float(Smexoscope::kNumTriggerTypes) + 0.0001f);
    if (triggerType == Smexoscope::kTriggerRising || triggerType == Smexoscope::kTriggerFalling) {
        const float yVirtual = 1.0f + (1.0f - effect.getParameter(Smexoscope::kTriggerLevel)) * float(OSC_HEIGHT - 2);
        g.setColour(ui::kTriggerLineColour);
        g.drawHorizontalLine(int(mapVirtualYToScope(scopeArea, yVirtual)), scopeArea.getX(), scopeArea.getRight());
    }

    g.setColour(ui::kZeroLineColour);
    g.drawHorizontalLine(int(mapVirtualYToScope(scopeArea, float(OSC_CENTER))), scopeArea.getX(), scopeArea.getRight());

    const auto& points = (effect.getParameter(Smexoscope::kSyncDraw) > 0.5f) ? effect.getCopy() : effect.getPeaks();
    const double samplesPerPixel = std::pow(10.0, effect.getParameter(Smexoscope::kTimeWindow) * 5.0 - 1.5);

    juce::Graphics::ScopedSaveState waveformState(g);
    g.reduceClipRegion(scopeArea.getSmallestIntegerContainer());

    const float xScale = scopeArea.getWidth() / float(OSC_WIDTH);
    const float yScale = scopeArea.getHeight() / float(OSC_HEIGHT);
    auto transform = juce::AffineTransform::translation(scopeArea.getX(), scopeArea.getY())
        .scaled(xScale, yScale);
    g.addTransform(transform);

    if (samplesPerPixel < 1.0) {
        g.setColour(ui::kWaveInterpolatedColour);

        double phase = samplesPerPixel;
        const double dPhase = samplesPerPixel;

        double prevX = points[0].x;
        double prevY = points[0].y;

        for (int i = 1; i < OSC_WIDTH; ++i) {
            const size_t index = size_t(phase);
            const double alpha = phase - double(index);
            const double x = i;
            const double y = (1.0 - alpha) * points[index * 2].y + alpha * points[(index + 1) * 2].y;

            g.drawLine(float(prevX), float(prevY), float(x), float(y), 1.0f / juce::jmax(1.0f, xScale));
            prevX = x;
            prevY = y;

            phase += dPhase;
        }
    } else {
        g.setColour(ui::kWaveDenseColour);

        for (size_t i = 0; i < points.size() - 1; ++i) {
            const auto p1 = points[i];
            const auto p2 = points[i + 1];
            g.drawLine(float(p1.x), float(p1.y), float(p1.x), float(p2.y), 1.0f / juce::jmax(1.0f, xScale));
        }
    }

    if (where.x >= 0 && where.y >= 0) {
        g.setColour(ui::kTextColour.withAlpha(0.85f));
        g.drawHorizontalLine(int(where.y), scopeArea.getX(), scopeArea.getRight());
        g.drawVerticalLine(int(where.x), scopeArea.getY(), scopeArea.getBottom());

        CursorMetrics metrics;
        metrics.xSamples = scopeXToSamples(float(where.x), samplesPerPixel);
        metrics.xSeconds = metrics.xSamples / float(effect.getSampleRate());
        metrics.xMs = metrics.xSeconds * 1000.0f;
        metrics.infiniteHz = (metrics.xSamples <= 0.0f);
        metrics.xHz = metrics.infiniteHz ? 0.0f : float(effect.getSampleRate()) / metrics.xSamples;
        metrics.yLinear = scopeYToLinear(float(where.y));
        metrics.yDb = linToDb(metrics.yLinear);
        cursorMetrics = metrics;
    } else {
        cursorMetrics.reset();
    }
}
