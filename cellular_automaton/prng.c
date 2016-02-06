#include <stdint.h>
#include "prng.h"
#include "cellular_automaton.h"

uint64_t s[2];

uint64_t xorshift128plus(void) {
  uint64_t x = s[0];
  uint64_t const y = s[1];
  s[0] = y;
  x ^= x << 23;
  s[1] = x ^ y ^ (x >> 17) ^ (y >> 26);
  return s[1] + y;
}

void seed(uint64_t n) {
  s[0] = n;
  s[1] = n;
}

float next(void) {
  return (float)xorshift128plus()/UINT64_MAX;
}
