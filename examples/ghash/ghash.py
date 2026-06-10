from __future__ import annotations

from exo import DRAM, proc
from exo.platforms.x86 import (
    XMM,
    mm_clmulepi64_si128_00,
    mm_clmulepi64_si128_11,
    mm_loadl_epi64,
    mm_loadu_si128,
    mm_setzero_si128,
    mm_storeu_si128,
    mm_swap64_si128,
    mm_xor_si128,
    mm_xor_si128_inplace,
)


@proc
def polyval_blocks_8x(
    nblocks: size,
    acc: ui64[2] @ DRAM,
    powers: ui64[8, 2] @ DRAM,
    powers_xored: ui64[8] @ DRAM,
    gfpoly: ui64[1] @ DRAM,
    data: ui64[nblocks, 2] @ DRAM,
):
    assert nblocks % 8 == 0
    assert stride(acc, 0) == 1
    assert stride(powers, 1) == 1
    assert stride(powers_xored, 0) == 1
    assert stride(gfpoly, 0) == 1
    assert stride(data, 1) == 1

    acc_reg: ui64[2] @ XMM
    lo: ui64[2] @ XMM
    mi: ui64[2] @ XMM
    hi: ui64[2] @ XMM
    a: ui64[2] @ XMM
    a_xored: ui64[2] @ XMM
    b: ui64[2] @ XMM
    b_xored: ui64[2] @ XMM
    tmp: ui64[2] @ XMM
    swapped: ui64[2] @ XMM
    gfpoly_reg: ui64[2] @ XMM

    mm_loadu_si128(acc_reg, acc)
    mm_loadl_epi64(gfpoly_reg, gfpoly)

    for chunk in seq(0, nblocks / 8):
        mm_setzero_si128(lo)
        mm_setzero_si128(mi)
        mm_setzero_si128(hi)

        for j in seq(0, 8):
            mm_loadu_si128(a, powers[j, 0:2])
            mm_loadl_epi64(a_xored, powers_xored[j : j + 1])
            mm_loadu_si128(b, data[8 * chunk + j, 0:2])

            if j == 0:
                mm_xor_si128_inplace(b, acc_reg)

            mm_clmulepi64_si128_00(tmp, a, b)
            mm_xor_si128_inplace(lo, tmp)

            mm_swap64_si128(b_xored, b)
            mm_xor_si128_inplace(b_xored, b)

            mm_clmulepi64_si128_11(tmp, a, b)
            mm_xor_si128_inplace(hi, tmp)

            mm_clmulepi64_si128_00(tmp, a_xored, b_xored)
            mm_xor_si128_inplace(mi, tmp)

        # Convert the three Karatsuba terms into a 256-bit product, then
        # reduce modulo x^128 + x^127 + x^126 + x^121 + 1.
        mm_xor_si128_inplace(mi, lo)
        mm_xor_si128_inplace(mi, hi)
        mm_swap64_si128(swapped, lo)
        mm_clmulepi64_si128_00(tmp, gfpoly_reg, lo)
        mm_xor_si128_inplace(mi, swapped)
        mm_xor_si128_inplace(mi, tmp)

        mm_swap64_si128(swapped, mi)
        mm_clmulepi64_si128_00(tmp, gfpoly_reg, mi)
        mm_xor_si128(acc_reg, hi, swapped)
        mm_xor_si128_inplace(acc_reg, tmp)

    mm_storeu_si128(acc, acc_reg)


__all__ = ["polyval_blocks_8x"]
