#pragma once
#include <common/serialize.hpp>
#include <common/assert.hpp>
#include <plonk/composer/composer_base.hpp>
#include <plonk/composer/turbo_composer.hpp>
#include <stdlib/encryption/ecdsa/ecdsa_impl.hpp>
#include <crypto/ecdsa/ecdsa.hpp>
#include <stdlib/types/turbo.hpp>
#include <stdlib/primitives/packed_byte_array/packed_byte_array.hpp>
#include <ecc/curves/secp256k1/secp256k1.hpp>

using namespace plonk::stdlib::types::turbo;
using namespace barretenberg;

crypto::ecdsa::signature ecdsa_convert_signature(waffle::TurboComposer& composer, std::vector<uint32_t> signature)
{

    crypto::ecdsa::signature signature_cr;

    // Get the witness assignment for each witness index
    // Write the witness assignment to the byte_array

    for (unsigned int i = 0; i < 32; i++) {
        auto witness_index = signature[i];

        std::vector<uint8_t> fr_bytes(sizeof(fr));

        fr value = composer.get_variable(witness_index);

        fr::serialize_to_buffer(value, &fr_bytes[0]);

        signature_cr.r[i] = fr_bytes.back();
    }

    for (unsigned int i = 32; i < 64; i++) {
        auto witness_index = signature[i];

        std::vector<uint8_t> fr_bytes(sizeof(fr));

        fr value = composer.get_variable(witness_index);

        fr::serialize_to_buffer(value, &fr_bytes[0]);

        signature_cr.s[i - 32] = fr_bytes.back();
    }

    return signature_cr;
}

namespace plonk {
namespace stdlib {
namespace secp256k {
typedef typename plonk::stdlib::bigfield<waffle::TurboComposer, secp256k1::Secp256k1FqParams> fq;
typedef typename plonk::stdlib::bigfield<waffle::TurboComposer, secp256k1::Secp256k1FrParams> fr;
typedef typename plonk::stdlib::element<waffle::TurboComposer, fq, fr, secp256k1::g1> g1;

} // namespace secp256k
} // namespace stdlib
} // namespace plonk

stdlib::secp256k::g1 ecdsa_convert_inputs(waffle::TurboComposer* ctx, const secp256k1::g1::affine_element& input)
{
    uint256_t x_u256(input.x);
    uint256_t y_u256(input.y);

    stdlib::secp256k::fq x(witness_ct(ctx, barretenberg::fr(x_u256.slice(0, stdlib::secp256k::fq::NUM_LIMB_BITS * 2))),
                           witness_ct(ctx,
                                      barretenberg::fr(x_u256.slice(stdlib::secp256k::fq::NUM_LIMB_BITS * 2,
                                                                    stdlib::secp256k::fq::NUM_LIMB_BITS * 4))));
    stdlib::secp256k::fq y(witness_ct(ctx, barretenberg::fr(y_u256.slice(0, stdlib::secp256k::fq::NUM_LIMB_BITS * 2))),
                           witness_ct(ctx,
                                      barretenberg::fr(y_u256.slice(stdlib::secp256k::fq::NUM_LIMB_BITS * 2,
                                                                    stdlib::secp256k::fq::NUM_LIMB_BITS * 4))));

    return stdlib::secp256k::g1(x, y);
}

// vector of bytes here, assumes that the witness indices point to a field element which can be represented
// with just a byte.
// notice that this function truncates each field_element to a byte
byte_array_ct ecdsa_vector_of_bytes_to_byte_array(waffle::TurboComposer& composer,
                                                  std::vector<uint32_t> vector_of_bytes)
{
    byte_array_ct arr(&composer);

    // Get the witness assignment for each witness index
    // Write the witness assignment to the byte_array
    for (const auto& witness_index : vector_of_bytes) {

        field_ct element = field_ct::from_witness_index(&composer, witness_index);
        size_t num_bytes = 1;

        byte_array_ct element_bytes(element, num_bytes);
        arr.write(element_bytes);
    }
    return arr;
}
witness_ct ecdsa_index_to_witness(waffle::TurboComposer& composer, uint32_t index)
{
    fr value = composer.get_variable(index);
    return witness_ct(&composer, value);
}

struct EcdsaSecp256k1Constraint {
    // This is just a bunch of bytes
    // which need to be interpreted as a string
    // Note this must be a bunch of bytes
    std::vector<uint32_t> message;

    // This is the supposed public key which signed the
    // message, giving rise to the signature.
    // Since Fr does not have enough bits to represent
    // the prime field in secp256k1, a byte array is used.
    // Can also use low and hi where lo=128 bits
    std::vector<uint32_t> pub_x_indices;
    std::vector<uint32_t> pub_y_indices;

    // This is the result of verifying the signature
    uint32_t result;

    // This is the computed signature
    //
    std::vector<uint32_t> signature;

    // secp256k1::g1::affine_element pub_key;

    // crypto::ecdsa::signature sig;
};

void create_ecdsa_verify_constraints(waffle::TurboComposer& composer, const EcdsaSecp256k1Constraint& input)
{

    auto new_sig = ecdsa_convert_signature(composer, input.signature);

    auto message = ecdsa_vector_of_bytes_to_byte_array(composer, input.message);
    auto pub_key_x_byte_arr = ecdsa_vector_of_bytes_to_byte_array(composer, input.pub_x_indices);
    auto pub_key_y_byte_arr = ecdsa_vector_of_bytes_to_byte_array(composer, input.pub_y_indices);

    auto pub_key_x_fq = stdlib::secp256k::fq(pub_key_x_byte_arr);
    auto pub_key_y_fq = stdlib::secp256k::fq(pub_key_y_byte_arr);

    std::vector<uint8_t> rr(new_sig.r.begin(), new_sig.r.end());
    std::vector<uint8_t> ss(new_sig.s.begin(), new_sig.s.end());

    stdlib::ecdsa::signature<waffle::TurboComposer> sig{ stdlib::byte_array<waffle::TurboComposer>(&composer, rr),
                                                         stdlib::byte_array<waffle::TurboComposer>(&composer, ss) };

    auto pub_key = stdlib::secp256k::g1(pub_key_x_fq, pub_key_y_fq);

    // stdlib::bool_t<waffle::TurboComposer> signature_result = stdlib::ecdsa::
    //     verify_signature<waffle::TurboComposer, stdlib::secp256k::fq, stdlib::secp256k::fr, stdlib::secp256k::g1>(
    //         message, pub_key, sig);

    // auto result_bool = composer.add_variable(signature_result.get_value() == true);

    composer.copy_from_to(false, input.result);
}

template <typename B> inline void read(B& buf, EcdsaSecp256k1Constraint& constraint)
{
    using serialize::read;
    read(buf, constraint.message);
    read(buf, constraint.signature);
    read(buf, constraint.pub_x_indices);
    read(buf, constraint.pub_y_indices);
    read(buf, constraint.result);
}

template <typename B> inline void write(B& buf, EcdsaSecp256k1Constraint const& constraint)
{
    using serialize::write;
    write(buf, constraint.message);
    write(buf, constraint.signature);
    write(buf, constraint.pub_x_indices);
    write(buf, constraint.pub_y_indices);
    write(buf, constraint.result);
}

inline bool operator==(EcdsaSecp256k1Constraint const& lhs, EcdsaSecp256k1Constraint const& rhs)
{
    // clang-format off
    return
        lhs.message == rhs.message &&
        lhs.signature == rhs.signature &&
        lhs.result == rhs.result;
    // clang-format on
}
