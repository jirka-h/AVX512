/*
gcc -O3 -Wall -Wextra -o sub_avx256f sub_avx256f.c -mavx512f
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
  __m256d subv = _mm256_set_pd(1.01, 1.02, 1.03, 1.04);
  __m256d av = _mm256_set_pd(8080000000.0, 8160000000.0, 8240000000.0, 8320000000.0);
  __attribute__ ((aligned (32))) double out[4];

  for(i=0; i<2*N; ++i) {
    av = _mm256_sub_pd(av, subv);
  }

  _mm256_store_pd(out, av);
  for (i=0; i<4; ++i) {
    printf("%llu %.17f\n", i, out[i]);
  }

  return 0;
}
