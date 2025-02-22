set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_compile_options(-Werror -Wall -Wextra -Wconversion -Wsign-conversion -Wno-deprecated -Wno-tautological-compare -Wfatal-errors)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-unguarded-availability-new -Wno-c99-extensions -fconstexpr-steps=100000000)

    if(MEMORY_CHECKS)
        message(STATUS "Compiling with memory checks.")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wno-deprecated-copy -fconstexpr-ops-limit=100000000)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# I feel this should be limited to ecc, however it's currently used in headers that go across libraries,
# and there currently isn't an easy way to inherit the DDISABLE_SHENANIGANS parameter.
if(DISABLE_ASM)
    message(STATUS "Using fallback non-assembly methods for field multiplications.")
    add_definitions(-DDISABLE_SHENANIGANS=1)
else()
    message(STATUS "Using optimized assembly for field arithmetic.")
endif()

add_subdirectory(env)
add_subdirectory(numeric)
add_subdirectory(srs)
add_subdirectory(ecc)
add_subdirectory(crypto)
add_subdirectory(polynomials)
add_subdirectory(plonk)
add_subdirectory(stdlib)
add_subdirectory(dsl)
add_subdirectory(bb)
add_subdirectory(rollup)

if(BENCHMARKS)
    add_subdirectory(benchmark)
endif()

if(WASM)
    # Well, this is awkward. We can't build a wasm module by just linking to the libraries as that produces, nothing.
    # There are a couple of other ways to avoiding listing all the object files here and leveraging the dependency
    # tree, but they come with the problem that they will import the 'env' object files. We explicitly want to avoid
    # that as functions in 'env' should be implemented in JS itself.
    # It turns out that just explicitly telling the wasm module which object files to include was easiest.
    add_executable(
        barretenberg.wasm
        $<TARGET_OBJECTS:srs_objects>
        $<TARGET_OBJECTS:numeric_objects>
        $<TARGET_OBJECTS:crypto_sha256_objects>
        $<TARGET_OBJECTS:crypto_aes128_objects>
        $<TARGET_OBJECTS:crypto_blake2s_objects>
        $<TARGET_OBJECTS:crypto_keccak_objects>
        $<TARGET_OBJECTS:crypto_schnorr_objects>
        $<TARGET_OBJECTS:crypto_pedersen_objects>
        $<TARGET_OBJECTS:ecc_objects>
        $<TARGET_OBJECTS:polynomials_objects>
        $<TARGET_OBJECTS:plonk_objects>
        $<TARGET_OBJECTS:stdlib_primitives_objects>
        $<TARGET_OBJECTS:stdlib_schnorr_objects>
        $<TARGET_OBJECTS:stdlib_pedersen_objects>
        $<TARGET_OBJECTS:stdlib_blake2s_objects>
        $<TARGET_OBJECTS:stdlib_sha256_objects>
        $<TARGET_OBJECTS:rollup_proofs_inner_proof_data_objects>
        $<TARGET_OBJECTS:rollup_proofs_notes_objects>
        $<TARGET_OBJECTS:rollup_proofs_account_objects>
        $<TARGET_OBJECTS:rollup_proofs_join_split_objects>
        $<TARGET_OBJECTS:rollup_proofs_claim_objects>
        $<TARGET_OBJECTS:rollup_proofs_standard_example_objects>
        $<TARGET_OBJECTS:dsl_objects>
    )

    target_link_options(
        barretenberg.wasm
        PRIVATE
        -nostartfiles -Wl,--no-entry -Wl,--export-dynamic -Wl,--import-memory -Wl,--allow-undefined -Wl,--stack-first -Wl,-z,stack-size=8388608
    )
endif()
