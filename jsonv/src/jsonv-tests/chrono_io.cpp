#include <iomanip>
#include <ostream>

#include "chrono_io.hpp"

namespace jsonv_test
{

std::ostream& operator<<(std::ostream& os, std::chrono::nanoseconds value)
{
    using namespace std::chrono;

    if (value < nanoseconds(0))
    {
        // Don't know how ISO 8601 is supposed to deal with negative durations since the Wikipedia did not mention them.
        os << '-';
        value = -value;
    }
    os << "PT";
    auto hrs = duration_cast<hours>(value);
    if (hrs > hours(0))
    {
        os << hrs.count() << 'H';
        value -= hrs;
    }
    auto mins = duration_cast<minutes>(value);
    if (mins > minutes(0))
    {
        os << mins.count() << 'M';
        value -= mins;
    }
    auto secs = duration_cast<seconds>(value);
    value -= secs;
    os << secs.count();
    os << '.';
    os << std::setfill('0') << std::setw(9) << value.count();
    os << 'S';

    return os;
}

}
