#include "preemph.h"
#include "../../modules/math/constants.h"
#include <iostream>
#include <utility>

using namespace Nodes;

PreEmphasis::PreEmphasis()
    : Node(NodeDescriptor {
            .inputCount = 1,
            .inputs = &kNodeIoTypeAudioTime,
            .outputCount = 1,
            .outputs = &kNodeIoTypeAudioTime,
        })
{
}

void PreEmphasis::process(const NodeIO *inputs[], NodeIO *outputs[])
{
    auto in = inputs[0]->as<IO::AudioTime>();
    auto out = outputs[0]->as<IO::AudioTime>();

    int sampleRate = in->getSampleRate();
    int length = in->getLength();

    out->setSampleRate(sampleRate);
    out->setLength(length);

    constexpr double a = 0.68;

    std::vector<double> x(in->getConstData(), in->getConstData() + length);
    for (int i = length - 1; i >= 1; --i) {
        x[i] -= a * x[i - 1];
    }

    for (int i = 0; i < length; ++i) {
        out->getData()[i] = x[i];
    }
}
