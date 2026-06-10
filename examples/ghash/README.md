# Eight-way PCLMULQDQ POLYVAL/GHASH kernel

`ghash.py` implements the hot multiplication loop used by GHASH-compatible
POLYVAL hashing.  It follows the structure of `aes-gcm-aesni-x86_64.S`:

- process eight blocks together using precomputed `H^8` through `H`;
- use three PCLMULQDQ instructions per block via Karatsuba multiplication;
- accumulate unreduced products and perform one two-PCLMULQDQ reduction per
  eight blocks.

This costs 3.25 PCLMULQDQ instructions per block, compared with approximately
5 per block for a direct multiply-and-reduce implementation.  Inputs are in
POLYVAL's little-endian field representation.  GHASH callers must byte-reflect
their blocks and key as described in `gf128hash.c`.

Generate the intrinsic C, check correctness, and run the throughput benchmark:

```sh
make check-codegen
make benchmark
```

The benchmark requires an x86 CPU with PCLMULQDQ support.  Its scalar reference
is intended only for correctness checking and is excluded from the timed loop.
