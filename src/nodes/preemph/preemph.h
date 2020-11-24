#ifndef NODES_PREEMPH_H
#define NODES_PREEMPH_H

#include "../node/node.h"

namespace Nodes
{
    class PreEmphasis : public Node {
    public:
        PreEmphasis();

        void process(const NodeIO *inputs[], NodeIO *outputs[]) override;
    };
}

#endif // NODES_PREEMPH_H
