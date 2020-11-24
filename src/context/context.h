#ifndef MAIN_CONTEXT_H
#define MAIN_CONTEXT_H

#include "../modules/modules.h"
#include "../analysis/analysis.h"
#include <memory>
#include <map>
#include <string>

namespace Main {

    using namespace Module;

    struct RenderingContext {
        std::unique_ptr<Target::AbstractBase>   target;
        std::unique_ptr<Renderer::AbstractBase> renderer;
    };

    struct Context { 
        std::unique_ptr<Audio::AbstractBase>        audio;
        Renderer::Type                              rendererType; 
        std::map<std::string, RenderingContext>     renderingContexts;

        std::unique_ptr<Freetype::FTInstance>       freetypeInstance;

        std::unique_ptr<Audio::Buffer>              captureBuffer;
    };

}

#endif // MAIN_CONTEXT_H
