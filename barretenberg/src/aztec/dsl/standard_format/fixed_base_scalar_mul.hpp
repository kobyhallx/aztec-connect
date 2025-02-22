#include <common/serialize.hpp>
#include <common/assert.hpp>
#include <plonk/composer/composer_base.hpp>
#include <plonk/composer/turbo_composer.hpp>
#include <stdlib/hash/pedersen/pedersen.hpp>
#include <stdlib/types/turbo.hpp>
#include <stdlib/primitives/packed_byte_array/packed_byte_array.hpp>
#include <stdlib/primitives/byte_array/byte_array.hpp>

using namespace plonk::stdlib::types::turbo;

typedef plonk::stdlib::field_t<waffle::TurboComposer> field_t;
typedef plonk::stdlib::byte_array<waffle::TurboComposer> byte_array;

using namespace barretenberg;
using namespace plonk;

struct FixedBaseScalarMul {
    uint32_t scalar;
    uint32_t pub_key_x;
    uint32_t pub_key_y;
};

void create_fixed_base_constraint(waffle::TurboComposer& composer, const FixedBaseScalarMul& input)
{

    field_t scalar_as_field = field_t::from_witness_index(&composer, input.scalar);
    auto public_key = group_ct::fixed_base_scalar_mul_g1<254>(scalar_as_field);

    composer.copy_from_to(public_key.x.witness_index, input.pub_key_x);
    composer.copy_from_to(public_key.y.witness_index, input.pub_key_y);
}

template <typename B> inline void read(B& buf, FixedBaseScalarMul& constraint)
{
    using serialize::read;
    read(buf, constraint.scalar);
    read(buf, constraint.pub_key_x);
    read(buf, constraint.pub_key_y);
}

template <typename B> inline void write(B& buf, FixedBaseScalarMul const& constraint)
{
    using serialize::write;
    write(buf, constraint.scalar);
    write(buf, constraint.pub_key_x);
    write(buf, constraint.pub_key_y);
}

inline bool operator==(FixedBaseScalarMul const& lhs, FixedBaseScalarMul const& rhs)
{
    // clang-format off
    return
        lhs.scalar == rhs.scalar &&
        lhs.pub_key_x == rhs.pub_key_x &&
        lhs.pub_key_y == rhs.pub_key_y;
    // clang-format on
}
