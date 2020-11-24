#ifndef ANALYSIS_UTIL_H
#define ANALYSIS_UTIL_H

#include <vector>

namespace Analysis {
    std::vector<int> findPeaks(const double *data, int length, int sign = +1);
}

#endif // ANALYSIS_UTIL_H
