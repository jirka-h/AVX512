/*
gcc -O3 -Wall -Wextra -g -o div_two_independent_vectors_avx512f div_two_independent_vectors_avx512f.c -mavx512f
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
  __m512d divv[2];
  __m512d av[2];
  divv[0] = _mm512_set_pd(1.0000000001, 1.0000000002, 1.0000000003, 1.0000000004, 1.0000000005, 1.0000000006, 1.0000000007, 1.0000000008);
  divv[1] = _mm512_set_pd(1.0000000002, 1.0000000001, 1.0000000004, 1.0000000003, 1.0000000008, 1.0000000006, 1.0000000007, 1.0000000005);
  av[0] =  _mm512_set_pd(1.49182474713380642, 2.22554107607292417, 3.32011725278397885, 4.95303308049624480, 7.38905732167703544, 11.02317856826629416, 16.44465057627291671, 24.53253668079271677);
  av[1] =  _mm512_set_pd(2.22554107607292417, 1.49182474713380642, 4.95303308049624480, 3.32011725278397885, 24.53253668079271677, 11.02317856826629416, 16.44465057627291671, 7.38905732167703544);
  __attribute__ ((aligned (64))) double out[8*2];

  for(i=0; i<N/2; ++i) {
    av[0] = _mm512_div_pd(av[0], divv[0]);
    av[1] = _mm512_div_pd(av[1], divv[1]);
  }

  _mm512_store_pd(out, av[0]);
  _mm512_store_pd(out+8, av[1]);
  for (i=0; i<8*2; ++i) {
    printf("%llu %.17f\n", i, out[i]);
  }

  return 0;
}
