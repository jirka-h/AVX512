#include <immintrin.h>

double HarmonicSeriesAVX512() {
  struct timespec t[2];
  unsigned long long int i;
  __m512d sumv = _mm512_setzero_pd();
  __m512d onesv =_mm512_set1_pd(1.0);
  __m512d eightv =_mm512_set1_pd(8.0);
  __m512d divv = _mm512_set_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
  __m512d av;

  for(i=0; i<1000000000; ++i) {
    av = _mm512_div_pd(onesv, divv);
    sumv = _mm512_add_pd(av, sumv);
    divv = _mm512_add_pd(eightv, divv);
  }

  double c[8];
  double sum = 0.0;;
  _mm512_storeu_pd(c, sumv); // write sumv to c array
  for (i=0; i<8; ++i) {
    //printf("%d %g\n", i, c[i]);
    sum += c[i];
  }
  return sum;
}
