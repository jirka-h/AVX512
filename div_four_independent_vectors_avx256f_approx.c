/*
gcc -O3 -Wall -Wextra -g -o div_four_independent_vectors_avx256f_approx div_four_independent_vectors_avx256f_approx.c -mavx512f
*/

#include <stdio.h>
#include <immintrin.h>

# define FORCE_INLINE __attribute__((always_inline)) inline

// Inspired by https://github.com/stgatilov/recip_rsqrt_benchmark/blob/master/routines_sse.h#L108
static FORCE_INLINE __m256d _m256d_recip_double_r5(__m256d a) {
  //inspired by http://www.mersenneforum.org/showthread.php?t=11765
  const __m256d one = _mm256_set1_pd(1.0);
  __m256d x = _mm256_cvtps_pd(_mm_rcp_ps(_mm256_cvtpd_ps(a)));
  __m256d r = _mm256_sub_pd(one, _mm256_mul_pd(a, x));
  __m256d r2 = _mm256_mul_pd(r, r);
  __m256d r2r = _mm256_add_pd(r2, r);      // r^2 + r
  __m256d r21 = _mm256_add_pd(r2, one);    // r^2 + 1
  __m256d poly = _mm256_mul_pd(r2r, r21);
  __m256d res = _mm256_add_pd(_mm256_mul_pd(poly, x), x);
  return res;
}

int main(void) {
  const unsigned long long int N = (unsigned long long int) 4.0e9;
  unsigned long long int i;
  if( ! __builtin_cpu_supports("avx512f")) {
    printf("CPU does not support avx512f, exiting.\n");
    return 1;
  }
  __m256d divv[4];
  __m256d av[4];
  volatile __m256d rec[4]; //This is to avoid moving the rec out of loop
  divv[0] = _mm256_set_pd(1.0000000001, 1.0000000002, 1.0000000003, 1.0000000004);
  divv[1] = _mm256_set_pd(1.0000000002, 1.0000000003, 1.0000000004, 1.0000000005);
  divv[2] = _mm256_set_pd(1.0000000005, 1.0000000003, 1.0000000004, 1.0000000002);
  divv[3] = _mm256_set_pd(1.0000000001, 1.0000000004, 1.0000000003, 1.0000000005);
  av[0] = _mm256_set_pd(1.4918246976114338, 2.2255409283144243, 3.3201169221389264, 4.9530324228101444);
  av[1] = _mm256_set_pd(2.2255409283144243, 3.3201169221389264, 4.9530324228101444, 7.3890560952361222);
  av[2] = _mm256_set_pd(7.3890560952361222, 3.3201169221389264, 4.9530324228101444, 2.2255409283144243);
  av[3] = _mm256_set_pd(1.4918246976114338, 4.9530324228101444, 3.3201169221389264, 7.3890560952361222);
  __attribute__ ((aligned (32))) double out[4*4];

  for(i=0; i<N/2; ++i) {
    rec[0] = _m256d_recip_double_r5(divv[0]);
    av[0] = _mm256_mul_pd(av[0], rec[0]);
    rec[1] = _m256d_recip_double_r5(divv[1]);
    av[1] = _mm256_mul_pd(av[1], rec[1]);
    rec[2] = _m256d_recip_double_r5(divv[2]);
    av[2] = _mm256_mul_pd(av[2], rec[2]);
    rec[3] = _m256d_recip_double_r5(divv[3]);
    av[3] = _mm256_mul_pd(av[3], rec[3]);
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
