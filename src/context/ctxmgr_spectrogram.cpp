#include "contextmanager.h"
#include <iomanip>

using namespace Main;
using namespace std::chrono_literals;

void ContextManager::scrollSpectrogram(RenderingContext &rctx)
{
    Renderer::SpectrogramRenderData specRender;
   
    auto& slice = spectrogramTrack.back();

    for (const auto& [frequency, intensity] : slice) {
        specRender.push_back({frequency, intensity});
    }

    rctx.renderer->scrollSpectrogram(specRender, spectrogramCount);
}

void ContextManager::renderSpectrogram(RenderingContext &rctx)
{
    if (!isPaused) {
        scrollSpectrogram(rctx);
    }

    rctx.renderer->renderSpectrogram();

    auto& tickLabelFont = FONT(primaryFont, uiFontSize - 5, rctx);
    rctx.renderer->renderFrequencyScaleBar(tickLabelFont, tickLabelFont);

    if (durLoop > 0us) {
        auto& font = FONT(primaryFont, uiFontSize - 4, rctx);
        auto& smallerFont = FONT(primaryFont, uiFontSize - 5, rctx);
        
        int em = std::get<3>(font.queryTextSize("M"));
        int smallerEm = std::get<3>(smallerFont.queryTextSize("M"));

        float y;
        float tx, ty, tw, th;

        std::stringstream ss;

        y = 15;

        if (displayLegends) {
#if ! ( defined(ANDROID) || defined(__ANDROID__) ) 
            const std::vector<std::string> keyLegends = {
#ifndef __EMSCRIPTEN__
                "F2: Open FFT spectrum",
#endif
                "P: Pause/resume analysis",
                "U: Toggle help text",
                "S: Open settings window",
            };
#else
            const std::array<std::string, 0> keyLegends {};
#endif

            for (const auto& str : keyLegends) {
                rctx.renderer->renderText(
                        font,
                        str,
                        15,   
                        y,
                        1.0f, 1.0f, 1.0f);
                y += em + 10;
            }
        }

        std::vector<std::string> bottomStrings;
        float frequency;

        frequency = rctx.renderer->renderFrequencyCursor(specMX, specMY);
        
        ss.str("");
        ss << "Cursor: " << std::round(frequency) << " Hz";
        bottomStrings.push_back(ss.str());

        y += smallerEm;

        std::tie(tx, ty, tw, th) = smallerFont.queryTextSize("Cursor: 55555 Hz");

        int maxWidth = tw;
        static float maxHeight = 0;
        th = bottomStrings.size() * (smallerEm + 10) - 10;
        maxHeight = 0.1 * th + (1 - 0.1) * maxHeight;

        for (const auto& str : bottomStrings) {
            const auto [tx, ty, tw, th] = smallerFont.queryTextSize(str);
            if (tw > maxWidth) {
                maxWidth = tw;
            }
        }

        rctx.renderer->renderRoundedRect(
                15, y, maxWidth + 16, maxHeight + 16,
                0.157f, 0.165f, 0.212f, 0.8f);

        for (const auto& str : bottomStrings) {
            rctx.renderer->renderText(
                    smallerFont,
                    str,
                    15 + 8,
                    y + 8,
                    0.973f, 0.973f, 0.949f);
            y += smallerEm + 10;
        }

        if (displayLegends) {
            int h;
            rctx.target->getSize(nullptr, &h);

            ss.str("");
            ss << "Loop cycle took " << (durLoop.count() / 1000.0f) << " ms";
            rctx.renderer->
            renderText(
                    smallerFont,
                    ss.str(),
                    15,
                    h - 15 - smallerEm,
                    1.0f, 1.0f, 1.0f);
        }
    }
}

void ContextManager::eventSpectrogram(RenderingContext &rctx)
{ 
#ifndef __EMSCRIPTEN__
    if (rctx.target->isKeyPressedOnce(SDL_SCANCODE_F2)) {
        auto& target = ctx->renderingContexts["FFT spectrum"].target;
        if (!target->isVisible()) {
            target->show();
        }
    }
#endif

    const auto [mx, my] = rctx.target->getMousePosition();
    int mw, mh;
    rctx.target->getSize(&mw, &mh);

    specMX = (float) mx / (float) mw;
    specMY = (float) my / (float) mh;

    if (rctx.target->isKeyPressedOnce(SDL_SCANCODE_U)) {
        displayLegends = !displayLegends;
    }
}

