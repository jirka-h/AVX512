/*
gcc -O3 -Wall -Wextra -o sub_avx512f sub_avx512f.c -mavx512f
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
  __m512d subv = _mm512_set_pd(1.01, 1.02, 1.03, 1.04, 1.05, 1.06, 1.07, 1.08);
  __m512d av = _mm512_set_pd(4040000000.0, 4080000000.0, 4120000000.0, 4160000000.0, 4200000000.0, 4240000000.0, 4280000000.0, 4320000000.0);
  __attribute__ ((aligned (64))) double out[8];

  for(i=0; i<N; ++i) {
    av = _mm512_sub_pd(av, subv);
  }

  _mm512_store_pd(out, av);
  for (i=0; i<8; ++i) {
    printf("%llu %.17f\n", i, out[i]);
  }

  return 0;
}
