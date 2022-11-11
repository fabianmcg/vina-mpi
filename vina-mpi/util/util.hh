//============================================================================
// Name        : util.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#ifndef __UTIL_HH__
#define __UTIL_HH__

#include <chrono>
#include <vector>

#include "../definitions.hh"
#include "typename_util.hh"

namespace MPIBatch {
inline duration elapsed(const time_point &end, const time_point &begin) {
    return std::chrono::duration_cast <std::chrono::duration <double>>(end - begin);;
}

inline time_point now() {
    return std::chrono::high_resolution_clock::now();
}

bool test(const std::vector <bool> &vector);

std::string display_duration(duration ns);
}

#endif
