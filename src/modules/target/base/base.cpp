#include "base.h"
#include <algorithm>
#include <stdexcept>

using namespace Module::Target;

AbstractBase::AbstractBase(std::initializer_list<Type> supportedRenderers)
    : mSupportedRenderers(supportedRenderers)
{
}

AbstractBase::~AbstractBase()
{
    if (supportsRenderer(Type::NanoVG)) {
        delete mNvgProvider;
    }
}

bool AbstractBase::supportsRenderer(Type rendererType)
{
#ifndef RENDERER_USE_NVG
    if (rendererType == Type::NanoVG) return false;
#endif

    auto it = std::find(mSupportedRenderers.begin(), mSupportedRenderers.end(), rendererType);
    return (it != mSupportedRenderers.end());
}

NvgProvider *AbstractBase::getNvgProvider()
{
    if (!supportsRenderer(Type::NanoVG)) {
        throw std::runtime_error("Target::AbstractBase] NanoVG renderer not supported");
    }
    return mNvgProvider;
}

void AbstractBase::setNvgProvider(NvgProvider *provider)
{
    if (!supportsRenderer(Type::NanoVG)) {
        throw std::runtime_error("Target::AbstractBase] NanoVG renderer not supported");
    }
    mNvgProvider = provider;
}
