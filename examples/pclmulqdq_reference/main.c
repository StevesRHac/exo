#include "pclmulqdq_reference.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

static void clmul64_c(uint64_t a, uint64_t b, uint64_t out[2]) {
  uint64_t lo = 0;
  uint64_t hi = 0;

  for (unsigned i = 0; i < 64; i++) {
    if (((b >> i) & UINT64_C(1)) != 0) {
      lo ^= a << i;
      if (i > 0) {
        hi ^= a >> (64 - i);
      }
    }
  }

  out[0] = lo;
  out[1] = hi;
}

static uint64_t next_random(uint64_t *state) {
  uint64_t x = *state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  *state = x;
  return x;
}

typedef void (*pclmulqdq_fn)(void *, uint64_t *, const uint64_t *,
                            const uint64_t *);

static int check_case(const uint64_t a[2], const uint64_t b[2], int a_lane,
                      int b_lane, pclmulqdq_fn hardware) {
  uint64_t exo_reference[2];
  uint64_t hardware_result[2];
  uint64_t expected[2];

  pclmulqdq_reference(NULL, a_lane, b_lane, a, b, exo_reference);
  hardware(NULL, hardware_result, a, b);
  clmul64_c(a[a_lane], b[b_lane], expected);

  if (exo_reference[0] == expected[0] && exo_reference[1] == expected[1] &&
      hardware_result[0] == expected[0] && hardware_result[1] == expected[1]) {
    return 0;
  }

  fprintf(stderr, "mismatch for lanes (%d, %d):\n", a_lane, b_lane);
  fprintf(stderr, "  a        = 0x%016" PRIx64 "\n", a[a_lane]);
  fprintf(stderr, "  b        = 0x%016" PRIx64 "\n", b[b_lane]);
  fprintf(stderr, "  expected = 0x%016" PRIx64 "%016" PRIx64 "\n", expected[1],
          expected[0]);
  fprintf(stderr, "  Exo ref  = 0x%016" PRIx64 "%016" PRIx64 "\n",
          exo_reference[1], exo_reference[0]);
  fprintf(stderr, "  hardware = 0x%016" PRIx64 "%016" PRIx64 "\n",
          hardware_result[1], hardware_result[0]);
  return 1;
}

int main(void) {
  static const pclmulqdq_fn hardware[2][2] = {
      {pclmulqdq_00, pclmulqdq_10},
      {pclmulqdq_01, pclmulqdq_11},
  };
  uint64_t state = UINT64_C(0x243f6a8885a308d3);
  uint64_t a[2] = {UINT64_C(0x0123456789abcdef),
                   UINT64_C(0xfedcba9876543210)};
  uint64_t b[2] = {UINT64_C(0x1111111111111111),
                   UINT64_C(0x8000000000000001)};

  for (int a_lane = 0; a_lane < 2; a_lane++) {
    for (int b_lane = 0; b_lane < 2; b_lane++) {
      if (check_case(a, b, a_lane, b_lane, hardware[a_lane][b_lane]) != 0) {
        return 1;
      }
    }
  }

  for (int trial = 0; trial < 1000; trial++) {
    a[0] = next_random(&state);
    a[1] = next_random(&state);
    b[0] = next_random(&state);
    b[1] = next_random(&state);

    for (int a_lane = 0; a_lane < 2; a_lane++) {
      for (int b_lane = 0; b_lane < 2; b_lane++) {
        if (check_case(a, b, a_lane, b_lane, hardware[a_lane][b_lane]) != 0) {
          return 1;
        }
      }
    }
  }

  puts("PCLMULQDQ reference and hardware implementations: all tests passed");
  return 0;
}
