#include "test.hpp"

#include <jsonv/optional.hpp>

namespace jsonv_test
{

TEST(optional_none)
{
    jsonv::optional<int> x = jsonv::nullopt;
    ensure(!x);
}

TEST(optional_something)
{
    jsonv::optional<int> x = 1;
    ensure(bool(x));
    ensure_eq(1, *x);
}

}
