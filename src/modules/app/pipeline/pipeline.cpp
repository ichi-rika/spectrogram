#include "pipeline.h"

using namespace Module::App;

Pipeline::Pipeline(Module::Audio::Buffer *captureBuffer)
    : ndi(nullptr), ndo(nullptr),
      wasInitializedAtLeastOnce(false),
      captureBuffer(captureBuffer)
{
}

Pipeline::~Pipeline()
{
    if (ndi) delete[] ndi;
    if (ndo) delete[] ndo;
}

void Pipeline::initialize()
{
    captureSampleRate = captureBuffer->getSampleRate();
    createNodes();
    createIOs();
    wasInitializedAtLeastOnce = true;
}

Pipeline& Pipeline::setFFTSampleRate(float value)
{
    fftSampleRate = value;
    if (wasInitializedAtLeastOnce) {
        nodes["rs_fft"]->as<Nodes::Resampler>()->setOutputSampleRate(value);
    }
    return *this;
}

Pipeline& Pipeline::setFFTSize(int value)
{
    fftSize = value;
    if (wasInitializedAtLeastOnce) {
        nodes["fft"]->as<Nodes::Spectrum>()->setFFTLength(value);
    }
    return *this;
}

const std::vector<std::array<float, 2>>& Pipeline::getFFTSlice() const
{
    return fftSlice;
}

void Pipeline::processAll()
{
    updatePrereqsForFFT();

    processStart("prereqs");

    processArc("prereqs", "rs_fft");
    processArc("rs_fft", "fft");

    updateOutputData();
}

void Pipeline::processStart(const std::string& nodeName)
{
    Nodes::NodeIO **outs = Nodes::unpack(nodeIOs[nodeName], &ndo);
    nodes[nodeName]->process(nullptr, outs);
}

void Pipeline::processArc(const std::string& input, const std::string& output)
{
    const Nodes::NodeIO **ins = const_cast<decltype(ins)>(Nodes::unpack(nodeIOs[input], &ndi));
    Nodes::NodeIO **outs = Nodes::unpack(nodeIOs[output], &ndo);
    nodes[output]->process(ins, outs);
}

void Pipeline::updatePrereqsForFFT()
{
    int fftInLength = nodes["rs_fft"]->as<Nodes::Resampler>()->getRequiredInputLength(fftSize);
    nodes["prereqs"]->as<Nodes::Prereqs>()->setMinimumOutputLength(fftInLength);
}

void Pipeline::updateOutputData()
{
    auto ioFFT = nodeIOs["fft"][0]->as<Nodes::IO::AudioSpec>();
    int fftSliceLength = ioFFT->getLength();
    fftSlice.resize(fftSliceLength);
    for (int i = 0; i < fftSliceLength; ++i) {
        fftSlice[i][0] = (fftSampleRate * i) / (2.0f * fftSliceLength);
        fftSlice[i][1] = ioFFT->getConstData()[i];
    }
}

void Pipeline::createNodes()
{
    nodes["prereqs"]            = std::make_unique<Nodes::Prereqs>(captureBuffer, 0, 0);

    nodes["rs_fft"]             = std::make_unique<Nodes::Resampler>(captureSampleRate, fftSampleRate);
    nodes["fft"]                = std::make_unique<Nodes::Spectrum>(fftSize);
}

void Pipeline::createIOs()
{
    for (const auto& [name, node] : nodes) {
        std::vector<std::unique_ptr<Nodes::NodeIO>> ioVector;
        for (const auto type : node->getOutputTypes()) {
            ioVector.push_back(Nodes::makeNodeIO(type));
        }
        nodeIOs[name] = std::move(ioVector);
    }
}
