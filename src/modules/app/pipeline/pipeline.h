#ifndef APP_PIPELINE_H
#define APP_PIPELINE_H

#include "../../../analysis/analysis.h"
#include "../../../nodes/nodes.h"
#include "../../audio/audio.h"

#include <unordered_map>
#include <chrono>

namespace Module::App
{
    using millis = std::chrono::milliseconds;
    using namespace std::chrono_literals;

    class Pipeline {
    public:
        Pipeline(Module::Audio::Buffer *);
        ~Pipeline();

        void initialize();

        Pipeline& setFFTSampleRate(float);
        Pipeline& setFFTSize(int);

        Pipeline& setPreEmphasisFrequency(float);
        
        const std::vector<std::array<float, 2>>&  getFFTSlice() const;

        void processAll();

    private:
        void processStart(const std::string& nodeName);
        void processArc(const std::string& input, const std::string& output);

        void updatePrereqsForFFT();
        void updateOutputData();

        void createNodes();
        void createIOs();

        std::unordered_map<std::string, std::unique_ptr<Nodes::Node>> nodes;
        std::unordered_map<std::string, std::vector<std::unique_ptr<Nodes::NodeIO>>> nodeIOs;

        Nodes::NodeIO **ndi, **ndo;

        std::vector<std::array<float, 2>> fftSlice;

        bool wasInitializedAtLeastOnce;

        millis analysisDuration;
        float captureSampleRate;

        Module::Audio::Buffer *captureBuffer;

        float fftSampleRate;
        int fftSize;

        float preEmphasisFrequency;
    };
}

#endif // APP_PIPELINE_H

