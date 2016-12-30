#include "chrono_io.hpp"
#include "stopwatch.hpp"

#include <ostream>

namespace jsonv_test
{

std::ostream& operator<<(std::ostream& os, const stopwatch::values& x)
{
    os << "{\"total\": \"" << x.sum   << '\"';
    os << ", \"count\": "  << x.count;
    os << ", \"mean\": \"" << x.mean  << '\"';
    os << '}';
    return os;
}

}
