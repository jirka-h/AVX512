/*
gcc -O3 -Wall -Wextra -g -o div_four_independent_vectors_avx256f div_four_independent_vectors_avx256f.c -mavx512f
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
  __m256d divv[4];
  __m256d av[4];
  divv[0] = _mm256_set_pd(1.0000000001, 1.0000000002, 1.0000000003, 1.0000000004);
  divv[1] = _mm256_set_pd(1.0000000002, 1.0000000003, 1.0000000004, 1.0000000005);
  divv[2] = _mm256_set_pd(1.0000000005, 1.0000000003, 1.0000000004, 1.0000000002);
  divv[0] = _mm256_set_pd(1.0000000001, 1.0000000004, 1.0000000003, 1.0000000005);
  av[0] = _mm256_set_pd(1.4918246976114338, 2.2255409283144243, 3.3201169221389264, 4.9530324228101444);
  av[1] = _mm256_set_pd(2.2255409283144243, 3.3201169221389264, 4.9530324228101444, 7.3890560952361222);
  av[2] = _mm256_set_pd(7.3890560952361222, 3.3201169221389264, 4.9530324228101444, 2.2255409283144243);
  av[0] = _mm256_set_pd(1.4918246976114338, 4.9530324228101444, 3.3201169221389264, 7.3890560952361222);
  __attribute__ ((aligned (32))) double out[4*4];

  for(i=0; i<N/2; ++i) {
    av[0] = _mm256_div_pd(av[0], divv[0]);
    av[1] = _mm256_div_pd(av[1], divv[1]);
    av[2] = _mm256_div_pd(av[2], divv[2]);
    av[3] = _mm256_div_pd(av[3], divv[3]);
  }

  _mm256_store_pd(out, av[0]);
  _mm256_store_pd(out+4, av[1]);
  _mm256_store_pd(out+8, av[2]);
  _mm256_store_pd(out+12, av[3]);
  for (i=0; i<4*4; ++i) {
    printf("%llu %.17f\n", i, out[i]);
  }

  return 0;
}
