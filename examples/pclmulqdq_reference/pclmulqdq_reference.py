from __future__ import annotations

from exo import DRAM, proc
from exo.platforms.x86 import (
    XMM,
    mm_clmulepi64_si128_00,
    mm_clmulepi64_si128_01,
    mm_clmulepi64_si128_10,
    mm_clmulepi64_si128_11,
    mm_loadu_si128,
    mm_storeu_si128,
)


@proc
def pclmulqdq_reference(
    a_lane: index,
    b_lane: index,
    a: ui64[2] @ DRAM,
    b: ui64[2] @ DRAM,
    out: ui64[2] @ DRAM,
):
    assert 0 <= a_lane and a_lane < 2
    assert 0 <= b_lane and b_lane < 2

    out[0] = 0
    out[1] = 0

    for i in seq(0, 64):
        out[0] = out[0] ^ ((a[a_lane] << i) * ((b[b_lane] >> i) & 1))

        if i > 0:
            out[1] = out[1] ^ (
                (a[a_lane] >> (64 - i)) * ((b[b_lane] >> i) & 1)
            )


@proc
def pclmulqdq_00(out: ui64[2] @ DRAM, a: ui64[2] @ DRAM, b: ui64[2] @ DRAM):
    out_reg: ui64[2] @ XMM
    a_reg: ui64[2] @ XMM
    b_reg: ui64[2] @ XMM
    mm_loadu_si128(a_reg, a)
    mm_loadu_si128(b_reg, b)
    mm_clmulepi64_si128_00(out_reg, a_reg, b_reg)
    mm_storeu_si128(out, out_reg)


@proc
def pclmulqdq_01(out: ui64[2] @ DRAM, a: ui64[2] @ DRAM, b: ui64[2] @ DRAM):
    out_reg: ui64[2] @ XMM
    a_reg: ui64[2] @ XMM
    b_reg: ui64[2] @ XMM
    mm_loadu_si128(a_reg, a)
    mm_loadu_si128(b_reg, b)
    mm_clmulepi64_si128_01(out_reg, a_reg, b_reg)
    mm_storeu_si128(out, out_reg)


@proc
def pclmulqdq_10(out: ui64[2] @ DRAM, a: ui64[2] @ DRAM, b: ui64[2] @ DRAM):
    out_reg: ui64[2] @ XMM
    a_reg: ui64[2] @ XMM
    b_reg: ui64[2] @ XMM
    mm_loadu_si128(a_reg, a)
    mm_loadu_si128(b_reg, b)
    mm_clmulepi64_si128_10(out_reg, a_reg, b_reg)
    mm_storeu_si128(out, out_reg)


@proc
def pclmulqdq_11(out: ui64[2] @ DRAM, a: ui64[2] @ DRAM, b: ui64[2] @ DRAM):
    out_reg: ui64[2] @ XMM
    a_reg: ui64[2] @ XMM
    b_reg: ui64[2] @ XMM
    mm_loadu_si128(a_reg, a)
    mm_loadu_si128(b_reg, b)
    mm_clmulepi64_si128_11(out_reg, a_reg, b_reg)
    mm_storeu_si128(out, out_reg)


__all__ = [
    "pclmulqdq_reference",
    "pclmulqdq_00",
    "pclmulqdq_01",
    "pclmulqdq_10",
    "pclmulqdq_11",
]
