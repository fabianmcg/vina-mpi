//============================================================================
// Name        : vina_util.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#ifndef __VINA_UTIL_HH__
#define __VINA_UTIL_HH__

#include <vector>
#include "arguments/arguments.hh"

namespace MPIBatch {
void init_dirs(options_t &options);
std::vector <std::string> pdbqt_files(options_t &options);
std::vector <std::string> init_vina_files(options_t &options);
}
#endif
