#include "ghash.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLOCKS 1024
#define TRIALS 200000

typedef struct {
  uint64_t lo;
  uint64_t hi;
} elem;

static void clmul64(uint64_t a, uint64_t b, uint64_t *lo, uint64_t *hi) {
  *lo = 0;
  *hi = 0;
  for (unsigned i = 0; i < 64; i++) {
    uint64_t mask = -((b >> i) & 1);
    *lo ^= (a << i) & mask;
    if (i != 0)
      *hi ^= (a >> (64 - i)) & mask;
  }
}

static elem mul(elem a, elem b) {
  uint64_t c0, c1, c2, c3, m0, m1;
  clmul64(a.lo, b.lo, &c0, &c1);
  clmul64(a.hi, b.hi, &c2, &c3);
  clmul64(a.lo ^ a.hi, b.lo ^ b.hi, &m0, &m1);
  m0 ^= c0 ^ c2;
  m1 ^= c1 ^ c3;
  c1 ^= m0;
  c2 ^= m1;
  c1 ^= (c0 << 63) ^ (c0 << 62) ^ (c0 << 57);
  c2 ^= c0 ^ (c0 >> 1) ^ (c0 >> 2) ^ (c0 >> 7);
  c2 ^= (c1 << 63) ^ (c1 << 62) ^ (c1 << 57);
  c3 ^= c1 ^ (c1 >> 1) ^ (c1 >> 2) ^ (c1 >> 7);
  return (elem){c2, c3};
}

static elem reference(elem acc, elem h, const elem *data, size_t nblocks) {
  for (size_t i = 0; i < nblocks; i++) {
    acc.lo ^= data[i].lo;
    acc.hi ^= data[i].hi;
    acc = mul(acc, h);
  }
  return acc;
}

static uint64_t random64(uint64_t *state) {
  uint64_t x = *state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  return *state = x;
}

static double now(void) {
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(void) {
  static elem data[BLOCKS];
  elem powers[8];
  uint64_t powers_xored[8];
  const uint64_t gfpoly = UINT64_C(0xc200000000000000);
  const elem initial = {UINT64_C(0x0123456789abcdef),
                        UINT64_C(0xfedcba9876543210)};
  const elem h = {UINT64_C(0x92d5b02b2f9a58bd),
                  UINT64_C(0xd3a1ca3f7c18d42b)};
  uint64_t state = UINT64_C(0x243f6a8885a308d3);

  for (size_t i = 0; i < BLOCKS; i++)
    data[i] = (elem){random64(&state), random64(&state)};

  powers[7] = h;
  for (int i = 6; i >= 0; i--)
    powers[i] = mul(powers[i + 1], h);
  for (int i = 0; i < 8; i++)
    powers_xored[i] = powers[i].lo ^ powers[i].hi;

  elem expected = reference(initial, h, data, BLOCKS);
  elem actual = initial;
  polyval_blocks_8x(NULL, BLOCKS, (uint64_t *)&actual, (uint64_t *)powers,
                    powers_xored, &gfpoly,
                    (const uint64_t *)data);
  if (memcmp(&actual, &expected, sizeof(actual)) != 0) {
    fprintf(stderr, "mismatch: got %016" PRIx64 "%016" PRIx64
                    ", expected %016" PRIx64 "%016" PRIx64 "\n",
            actual.hi, actual.lo, expected.hi, expected.lo);
    return 1;
  }

  double start = now();
  for (int trial = 0; trial < TRIALS; trial++)
    polyval_blocks_8x(NULL, BLOCKS, (uint64_t *)&actual, (uint64_t *)powers,
                      powers_xored, &gfpoly,
                      (const uint64_t *)data);
  double elapsed = now() - start;
  double bytes = (double)BLOCKS * 16 * TRIALS;
  printf("correct; %.2f GiB/s (%zu blocks per call), checksum=%016" PRIx64 "\n",
         bytes / elapsed / (1ULL << 30), (size_t)BLOCKS, actual.lo);
  return 0;
}
