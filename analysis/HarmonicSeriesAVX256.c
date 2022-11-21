#include <immintrin.h>
double HarmonicSeriesAVX256() {
  struct timespec t[2];
  unsigned long long int i;
  __m256d sumv = _mm256_setzero_pd();
  __m256d onesv =_mm256_set1_pd(1.0);
  __m256d fourv =_mm256_set1_pd(4.0);
  __m256d divv = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);
  __m256d av;

  for(i=0; i<1000000000; ++i) {
    av = _mm256_div_pd(onesv, divv);
    sumv = _mm256_add_pd(av, sumv);
    divv = _mm256_add_pd(fourv, divv);
  }

  double c[4];
  double sum = 0.0;;
  _mm256_storeu_pd(c, sumv); // write sumv to c array
  for (i=0; i<4; ++i) {
    //printf("%d %g\n", i, c[i]);
    sum += c[i];
  }
  return sum;
}
