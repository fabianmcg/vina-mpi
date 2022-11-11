//============================================================================
// Name        : std-out.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#ifndef __LIB_STD_OUT_HH__
#define __LIB_STD_OUT_HH__

#include <sstream>

struct vina_std_out_t {
    std::ostringstream std_out = std::ostringstream();
    std::ostringstream std_err = std::ostringstream();
    inline void flush() {
        std_out.flush();
        std_err.flush();
    }
};

extern vina_std_out_t vina_std_outs;

extern std::ostringstream &vina_std_out;
extern std::ostringstream &vina_std_err;

#endif
