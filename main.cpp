#include <iostream>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>

#include "3rdparty/libpfc/include/libpfc.h"

extern "C" void _asm_0();
extern "C" void _asm_empty();
extern "C" size_t _asm_test_kernel(void *dst, void *src, size_t size);

//  Windows
#ifdef _WIN32

#include <intrin.h>
uint64_t rdtsc() { return __rdtsc(); }

//  Linux/GCC
#else
#include <x86intrin.h>
uint64_t rdtsc() {
  unsigned long lo, hi;
  __asm__ __volatile__("rdtscp" : "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}
uint64_t rdpmc_actual_cycles() {
  uint64_t a, d, c;

  c = (1 << 30) + 1;

  __asm__ volatile("rdpmc" : "=a"(a), "=d"(d) : "c"(c));
  return ((uint64_t)a) | (((uint64_t)d) << 32);
}
#endif

#define measure_begin                                                          \
  _mm_mfence();                                                                \
  _mm_lfence();                                                                \
  auto m1 = rdpmc_actual_cycles();                                             \
  _mm_lfence();

#define measure_end                                                            \
  _mm_lfence();                                                                \
  auto m2 = rdpmc_actual_cycles();                                             \
  _mm_lfence();

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

#define call_pfc(call)                                                         \
  {                                                                            \
    int err = call;                                                            \
    if (err < 0) {                                                             \
      const char *msg = pfcErrorString(err);                                   \
      throw std::runtime_error(std::string(STR(call) "failed (error ") +       \
                               std::to_string(err) + ": " + msg + ")");        \
    }                                                                          \
  }

template <typename T> double measure_fn(T t) {
  uint64_t sum = 0u;
  const uint64_t N = 10000;
  uint64_t counter = 0u;
  PFC_CFG cfg[7] = {7, 7, 7, 0, 0, 0, 0};
  PFC_CNT cnt[7] = {0, 0, 0, 0, 0, 0, 0};
  cfg[3] = pfcParseCfg("cpu_clk_unhalted.ref_xclk:auk");
  cfg[4] = pfcParseCfg("cpu_clk_unhalted.core_clk");
  cfg[5] = pfcParseCfg("*cpl_cycles.ring0>=1:uk");
  call_pfc(pfcWrCfgs(0, 7, cfg));
  call_pfc(pfcWrCnts(0, 7, cnt));
  for (int i = 0; i < N; i++) {
    memset(cnt, 0, sizeof(cnt));
    //_mm_lfence();
    PFCSTART(cnt);
    //_mm_lfence();
    t();
    //_mm_lfence();
    PFCEND(cnt);
    //_mm_lfence();
    if (i > 0) {
      uint64_t diff = 0u;
      diff = cnt[4];
      sum += diff;
      counter++;
    }
  }
  return double(sum) / double(counter);
}

int main(int argc, char **argv) {
  call_pfc(pfcInit());
  call_pfc(pfcPinThread(3));
  /* Warm up the CPU. */
  for (volatile int v = 0; v < 1000000000; v++) {
  }
  double overhead_clks = measure_fn([] {});
  std::cout << "overhead clocks: " << overhead_clks << "\n";
  std::cout << "empty kernel clocks: " << (measure_fn([] { _asm_empty(); }) - overhead_clks) << "\n";
  std::cout << "test kernel clocks: " << (measure_fn([] {
    _asm_test_kernel((void *)1, (void *)2, 3);
  }) - overhead_clks) << "\n";
  return 0;
}
