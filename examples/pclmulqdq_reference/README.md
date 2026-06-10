# PCLMULQDQ Reference Semantics

This example contains both the scalar reference semantics of PCLMULQDQ and
four hardware implementations using `_mm_clmulepi64_si128`. It selects one
64-bit lane from each 128-bit input and computes their carry-less product as
a 128-bit result:

```text
out[0] = low 64 bits
out[1] = high 64 bits
```

The lane arguments correspond to the PCLMULQDQ immediate selectors:

```text
a_lane = imm8 bit 0
b_lane = imm8 bit 4
```

Generate the C implementation and verify that all four intrinsics are present:

```sh
make check-codegen
```

On an x86 machine with PCLMULQDQ support, build and run the verification
program:

```sh
make check
```

The C driver compares three implementations:

- The dynamic-lane Exo scalar reference.
- An independent scalar C reference.
- The corresponding Exo XMM/PCLMULQDQ hardware implementation.

All four lane combinations are checked using fixed inputs and 1000 sets of
deterministic random inputs. The default compiler flag is `-mpclmul`; override
`CFLAGS` if your compiler requires a different target flag.
