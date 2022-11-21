#include <immintrin.h>
double HarmonicSeriesAVX512_four_vectors() {
  struct timespec t[2];
  unsigned long long int i;
  int j;
  __m512d sumv[4];
  const __m512d onesv = _mm512_set1_pd(1.0);
  const __m512d addv =_mm512_set1_pd(4.0*8.0);
  __m512d divv[4];

  for (j=0; j<4; ++j) {
    sumv[j] = _mm512_setzero_pd();
    divv[j] = _mm512_set_pd(8*j + 1.0, 8*j + 2.0, 8*j + 3.0, 8*j + 4.0, 8*j + 5.0, 8*j + 6.0, 8*j + 7.0, 8*j + 8.0);
  }

  for(i=0; i<1000000000; ++i) {
    for (j=0; j<4; ++j) {
      sumv[j] = _mm512_add_pd( _mm512_div_pd(onesv, divv[j]), sumv[j]);
      divv[j] = _mm512_add_pd(addv, divv[j]);
    }
  }

  double c[8];
  double sum = 0.0;;
  for (j=1; j<4; ++j) {
    sumv[0] = _mm512_add_pd(sumv[j], sumv[0]);
  }

  _mm512_storeu_pd(c, sumv[0]); // write sumv to c array
  for (i=0; i<8; ++i) {
    //printf("%d %g\n", i, c[i]);
    sum += c[i];
  }
  return sum;
}

