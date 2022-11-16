/*
gcc -O3 -Wall -Wextra -o add_avx512f add_avx512f.c -mavx512f
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
  __m512d addv = _mm512_set_pd(0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8);
  __m512d av = _mm512_setzero_pd();
  __attribute__ ((aligned (64))) double out[8];

  for(i=0; i<N; ++i) {
    av = _mm512_add_pd(av, addv);
  }

  _mm512_store_pd(out, av);
  for (i=0; i<8; ++i) {
    printf("%llu %.17f\n", i, out[i]);
  }

  return 0;
}
