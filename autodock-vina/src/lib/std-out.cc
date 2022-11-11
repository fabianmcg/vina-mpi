//============================================================================
// Name        : std-out.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#include "std-out.hh"

vina_std_out_t vina_std_outs = vina_std_out_t();
std::ostringstream &vina_std_out = vina_std_outs.std_out;
std::ostringstream &vina_std_err = vina_std_outs.std_err;
