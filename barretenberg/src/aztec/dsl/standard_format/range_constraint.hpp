#pragma once
#include <common/serialize.hpp>

struct RangeConstraint {
    uint32_t witness;
    uint32_t num_bits;
};

template <typename B> inline void read(B& buf, RangeConstraint& constraint)
{
    using serialize::read;
    read(buf, constraint.witness);
    read(buf, constraint.num_bits);
}

template <typename B> inline void write(B& buf, RangeConstraint const& constraint)
{
    using serialize::write;
    write(buf, constraint.witness);
    write(buf, constraint.num_bits);
}

inline bool operator==(RangeConstraint const& lhs, RangeConstraint const& rhs)
{
    // clang-format off
    return
        lhs.witness == rhs.witness &&
        lhs.num_bits == rhs.num_bits;
    // clang-format on
}
