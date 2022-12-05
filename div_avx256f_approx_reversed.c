/*
gcc -O3 -Wall -Wextra -o div_avx256f_approx_reversed div_avx256f_approx_reversed.c -mavx512f
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

static FORCE_INLINE __m256d _m256d_div_double_r5(__m256d a, __m256d b) {
  //inspired by http://www.mersenneforum.org/showthread.php?t=11765
  const __m256d one = _mm256_set1_pd(1.0);
  __m256d x = _mm256_cvtps_pd(_mm_rcp_ps(_mm256_cvtpd_ps(b)));
  x =  _mm256_mul_pd(a, x);
  __m256d r = _mm256_sub_pd(a, _mm256_mul_pd(b, x));
  __m256d r2 = _mm256_mul_pd(r, r);
  __m256d r2r = _mm256_add_pd(r2, r);      // r^2 + r
  __m256d r21 = _mm256_add_pd(r2, one);    // r^2 + 1
  __m256d poly = _mm256_mul_pd(r2r, r21);
  __m256d res = _mm256_add_pd(_mm256_mul_pd(poly, x), x);
  return res;
}

/*
static FORCE_INLINE __m256d _m256d_div_double_nr2(__m256d a, __m256d b) {
  __m256d muls, res_prev;
  __m256d res = _mm256_cvtps_pd(_mm_rcp_ps(_mm256_cvtpd_ps(b)));
  __m256d x =  _mm256_mul_pd(a, res);
  muls = _mm_mul_pd(b, _mm_mul_pd(res, res));
  res_prev = res;
  res = _mm_sub_pd(_mm_add_pd(res, res), muls);
  x =  _mm_sub_pd(x,  _mm_mul_pd(b,a)
  muls = _mm_mul_pd(b, _mm_mul_pd(res, res));
  res = _mm_sub_pd(_mm_add_pd(res, res), muls);
  return x;
}
*/

int main(void) {
  const unsigned long long int N = (unsigned long long int) 4.0e9;
  unsigned long long int i;
  if( ! __builtin_cpu_supports("avx512f")) {
    printf("CPU does not support avx512f, exiting.\n");
    return 1;
  }
  __m256d divv = _mm256_set_pd(1.0000000001, 1.0000000002, 1.0000000003, 1.0000000004);
  __m256d av = _mm256_set_pd(2.22554107593822170, 4.95303308029771472, 11.02317856892717884, 24.53253668667874265);
  __m256d rec;
  __attribute__ ((aligned (32))) double out[4];

  for(i=0; i<2*N; ++i) {
    rec = _m256d_recip_double_r5(av);
    av = _mm256_mul_pd(divv, rec);
  }

  _mm256_store_pd(out, av);
  for (i=0; i<4; ++i) {
    printf("%llu %.17f\n", i, out[i]);
  }

  return 0;
}
