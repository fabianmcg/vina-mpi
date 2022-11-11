//============================================================================
// Name        : util.cc
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================


#include <iomanip>
#include <sstream>
#include "util.hh"

namespace MPIBatch {

bool test(const std::vector <bool> &vector) {
    for (auto x : vector)
        if (x == false)
            return false;
    return true;
}

std::string display_duration(duration ns) {
    using namespace std;
    using namespace std::chrono;
    typedef std::chrono::duration<int, ratio <86400>> days;
    std::ostringstream os;
    char fill = os.fill();
    os.fill('0');
    auto d = duration_cast <days>(ns);
    ns -= d;
    auto h = duration_cast <hours>(ns);
    ns -= h;
    auto m = duration_cast <minutes>(ns);
    ns -= m;
    auto s = duration_cast <seconds>(ns);
    ns -= s;
    auto ms = duration_cast <milliseconds>(ns);
    os << setw(2) << d.count() << "d:"
       << setw(2) << h.count() << "h:"
       << setw(2) << m.count() << "m:"
       << setw(2) << s.count() << "s "
       << setw(3) << ms.count() << "ms";
    os.fill(fill);
    return os.str();
};
}
