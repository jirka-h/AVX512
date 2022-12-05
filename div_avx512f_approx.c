/*
gcc -O3 -Wall -Wextra -o div_avx512f_approx div_avx512f_approx.c -mavx512f
*/

#include <stdio.h>
#include <immintrin.h>

# define FORCE_INLINE __attribute__((always_inline)) inline

// Inspired by https://github.com/stgatilov/recip_rsqrt_benchmark/blob/master/routines_sse.h#L108
static FORCE_INLINE __m512d _m512d_recip_double_r5(__m512d a) {
  //inspired by http://www.mersenneforum.org/showthread.php?t=11765
  const __m512d one = _mm512_set1_pd(1.0);
  __m512d x = _mm512_cvtps_pd(_mm256_rcp_ps(_mm512_cvtpd_ps(a)));
  __m512d r = _mm512_sub_pd(one, _mm512_mul_pd(a, x));
  __m512d r2 = _mm512_mul_pd(r, r);
  __m512d r2r = _mm512_add_pd(r2, r);      // r^2 + r
  __m512d r21 = _mm512_add_pd(r2, one);    // r^2 + 1
  __m512d poly = _mm512_mul_pd(r2r, r21);
  __m512d res = _mm512_add_pd(_mm512_mul_pd(poly, x), x);
  return res;
}

int main(void) {
  const unsigned long long int N = (unsigned long long int) 4.0e9;
  unsigned long long int i;
  if( ! __builtin_cpu_supports("avx512f")) {
    printf("CPU does not support avx512f, exiting.\n");
    return 1;
  }
  volatile __m512d divv = _mm512_set_pd(1.0000000001, 1.0000000002, 1.0000000003, 1.0000000004, 1.0000000005, 1.0000000006, 1.0000000007, 1.0000000008);
  __m512d av = _mm512_set_pd(1.49182474713380642, 2.22554107607292417, 3.32011725278397885, 4.95303308049624480, 7.38905732167703544, 11.02317856826629416, 16.44465057627291671, 24.53253668079271677);
  volatile __m512d rec; //This is to avoid compiler moving this out of loop
  __attribute__ ((aligned (64))) double out[8];

  for(i=0; i<N; ++i) {
    //av = _mm512_div_pd(av, divv);
    rec = _m512d_recip_double_r5(divv);
    av = _mm512_mul_pd(av, rec);
  }

  _mm512_store_pd(out, av);
  for (i=0; i<8; ++i) {
    printf("%llu %.17f\n", i, out[i]);
  }

  return 0;
}
