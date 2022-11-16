/*
gcc -O3 -Wall -Wextra -o fma_avx512f fma_avx512f.c -mavx512f
*/

#include <stdio.h>
#include <immintrin.h>

int main(void) {
  const unsigned long long int N = (unsigned long long int) 4.0e9;
  unsigned long long int i;
  if( ! __builtin_cpu_supports("avx512f")) {
    printf("CPU does not support avx512f, exiting.\n");
    return 1;
  }
  __m512d mul = _mm512_set_pd(0.0000000001, 0.0000000002, 0.0000000003, 0.0000000004, 0.0000000005, 0.0000000006, 0.0000000007, 0.0000000008);
  __m512d av = _mm512_set_pd(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
  __attribute__ ((aligned (64))) double out[8];

  for(i=0; i<N; ++i) {
    av = _mm512_fmadd_pd(mul, av, av);
  }

  _mm512_store_pd(out, av);
  for (i=0; i<8; ++i) {
    printf("%llu %.17f\n", i, out[i]);
  }

  return 0;
}
