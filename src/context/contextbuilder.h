#ifndef MAIN_CONTEXT_BUILDER_H
#define MAIN_CONTEXT_BUILDER_H

#include <sstream>

#include "context.h"

namespace Main {

    using namespace Module;

    template <typename TargetType>
    class RenderingContextBuilder {
    public:
        RenderingContextBuilder(Renderer::Type rendererType)
            : rendererType(rendererType) {
        }

        RenderingContext build() {
            RenderingContext rctx;
            rctx.target = std::make_unique<TargetType>(rendererType);
            checkBuildRenderer(rctx);
            return rctx;
        }
       
        void throwRendererError(const std::string& name) {
            std::stringstream ss;
            ss << "Main] Requested " << name << " renderer but wasn't compiled with " << name << " support";
            ss.flush();
            throw std::runtime_error(ss.str());
        }

        template<typename RendererType, typename ProviderFunc>
        void buildRenderer(RenderingContext& rctx, ProviderFunc provider) {
            rctx.renderer = std::make_unique<RendererType>();
            rctx.renderer->setProvider((rctx.target.get()->*provider)());
        }

        void checkBuildRenderer(RenderingContext& ctx) {
            switch (rendererType) {
            case Renderer::Type::NanoVG:
#if ! RENDERER_USE_NVG
                throwRendererError("NanoVG");
#else
                buildRenderer<Renderer::NanoVG>(ctx, &Target::AbstractBase::getNvgProvider);
                break;
#endif
            default:
                throwRendererError("<unknown>");
            }
        }
    
    private:
        Renderer::Type rendererType;
    };

    template <typename AudioType, typename TargetType, Renderer::Type rendererType>
    class ContextBuilder {
    public:
        std::unique_ptr<Context> build() {
            auto ctx = std::make_unique<Context>();
            ctx->rendererType = rendererType;

            ctx->audio = std::make_unique<AudioType>();

            ctx->captureBuffer = std::make_unique<Audio::Buffer>(
                    captureSampleRate, captureDuration);

            ctx->audio->setCaptureBuffer(ctx->captureBuffer.get());

            ctx->freetypeInstance = std::make_unique<Freetype::FTInstance>();
            
            return ctx;
        }

        ContextBuilder& setCaptureSampleRate(int o) {
            captureSampleRate = o;
            return *this;
        }

        ContextBuilder& setCaptureDuration(const std::chrono::milliseconds& o) {
            captureDuration = o.count();
            return *this;
        }

    private:
        int captureSampleRate;
        int captureDuration;
    };

}

#endif // MAIN_CONTEXT_BUILDER_H
