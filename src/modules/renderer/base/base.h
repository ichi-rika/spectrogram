#ifndef RENDERER_BASE_H
#define RENDERER_BASE_H

#include "../../freetype/freetype.h"

#include <nanosvg.h>
#include <nanosvgrast.h>

#ifdef RENDERER_USE_NVG
#  include <nanovg.h>
#  if defined(__WIN32)
//#    include <nanovg_dx11.h>
//#    define NANOVG_DX11
#    include "../nanovg/nvg_gl.h"
#  elif defined(__APPLE__)
//#    include <nanovg_mtl.h>
//#    define NANOVG_METAL
#    include "../nanovg/nvg_gl.h"
#  else
#    include "../nanovg/nvg_gl.h"
#  endif
#endif

#include <vector>
#include <map>

namespace Module::Renderer {

    enum class Type {
        NanoVG,
    };
    
    class NvgProvider {
#ifdef RENDERER_USE_NVG
    public:
        virtual ~NvgProvider() {}
        virtual NVGcontext *createContext(int flags) = 0;
        virtual void deleteContext(NVGcontext *ctx) = 0;
        
        virtual void beforeBeginFrame() = 0;
        virtual void afterEndFrame() = 0;

        virtual void *createFramebuffer(NVGcontext *ctx, int width, int height, int imageFlags) = 0;
        virtual void bindFramebuffer(void *framebuffer) = 0;
        virtual void deleteFramebuffer(void *framebuffer) = 0;
        virtual int framebufferImage(void *framebuffer) = 0;
#endif
    };

    class Parameters;

    struct GraphRenderDataPoint {
        float x;
        float y;
    };
    using GraphRenderData = std::vector<GraphRenderDataPoint>;

    struct SpectrogramRenderDataPoint {
        float frequency;
        float intensity;
    };
    using SpectrogramRenderData = std::vector<SpectrogramRenderDataPoint>;

    class AbstractBase { 
    public:
        AbstractBase(Type type);
        virtual ~AbstractBase();

        virtual void setProvider(void *provider) = 0;
        virtual void initialize() = 0;
        virtual void terminate() = 0;

        virtual void begin() = 0;
        virtual void end() = 0;

        virtual void test() = 0;

        virtual void renderGraph(const GraphRenderData& data, float pmin, float pmax, float thick, float r, float g, float b) = 0;
       
        virtual void scrollSpectrogram(const SpectrogramRenderData& data, int count) = 0;

        virtual void renderSpectrogram() = 0;

        virtual void renderFrequencyScaleBar(Module::Freetype::Font& majorFont, Module::Freetype::Font& minorFont) = 0;

        virtual float renderFrequencyCursor(float mx, float my) = 0;
        
        virtual int renderFrameCursor(float mx, float my, int count) = 0;

        virtual void renderRoundedRect(float x, float y, float w, float h, float r, float g, float b, float a) = 0;

        virtual void renderSVG(const std::string& path, float dpi, float x, float y, float w, float h) = 0;

        virtual void renderText(Module::Freetype::Font& font, const std::string& text, int x, int y, float r, float g, float b) = 0;

        virtual std::tuple<float, float, float, float> renderInputBox(Module::Freetype::Font& font, const std::string& content, int x, int y, int w, bool isFocused) = 0;

        virtual uintptr_t getContextNumber() = 0;

        void setDrawableSize(int width, int height);
        void setWindowSize(int width, int height);

        Parameters *getParameters();
        constexpr Type getType() { return mType; }

    protected:
        void getDrawableSize(int *pWidth, int *pHeight) const;
        void getWindowSize(int *pWidth, int *pHeight) const;

        bool hasDrawableSizeChanged() const;
        void resetDrawableSizeChanged();

        bool hasWindowSizeChanged() const;
        void resetWindowSizeChanged();

        float frequencyToCoordinate(float frequency) const;
        float coordinateToFrequency(float y) const;
        void gainToColor(float gain, float *r, float *g, float *b) const;

    private:
        Type mType;
 
        int mDrawableWidth;
        int mDrawableHeight;
        bool mDrawableSizeChanged;

        int mWindowWidth;
        int mWindowHeight;
        bool mWindowSizeChanged;

        Parameters *mParameters;
    };
}

#include "../../target/base/base.h"
#include "parameters.h"

template<typename Tx, typename Tlo, typename Thi>
Tx clamp(const Tx& val, const Tlo& lo, const Thi& hi) {
    return Tx(val < lo ? lo : (val > hi ? hi : val));
}


#endif // RENDERER_BASE_H
