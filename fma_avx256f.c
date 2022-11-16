/*
gcc -O3 -Wall -Wextra -o fma_avx256f fma_avx256f.c -mavx512f -mfma
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
  __m256d mul = _mm256_set_pd(0.0000000001, 0.0000000002, 0.0000000003, 0.0000000004);
  __m256d av = _mm256_set_pd(1.0, 1.0, 1.0, 1.0);
  __attribute__ ((aligned (32))) double out[4];

  for(i=0; i<2*N; ++i) {
    av = _mm256_fmadd_pd(mul, av, av);
  }

  _mm256_store_pd(out, av);
  for (i=0; i<4; ++i) {
    printf("%llu %.17f\n", i, out[i]);
  }

  return 0;
}
