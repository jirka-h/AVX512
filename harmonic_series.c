/*
gcc -O3 -Wall -Wextra -o harmonic_series harmonic_series.c -mavx512f -lrt -lm
gcc -O3 -pg -Wall -Wextra -fsanitize=undefined -o harmonic_series harmonic_series.c -mavx512f -lrt -lm
gcc -O3 -Wall -Wextra -fsanitize=undefined -o harmonic_series_sanitized harmonic_series.c -mavx512f -lrt -lm

Harmonic series summation https://en.wikipedia.org/wiki/Harmonic_series_(mathematics) with
  * no AVX
  * AVX2 (4 packed doubles)
  * AVX-512F (8 packed doubles)

https://blog.triplez.cn/posts/avx-avx2-learning-notes/
https://www.root.cz/clanky/rozsireni-instrukcni-sady-f16c-fma-a-avx-512-na-platforme-x86-64
https://www.root.cz/clanky/rozsireni-instrukcni-sady-advanced-vector-extensions-na-platforme-x86-64

gdb -batch -ex "disassemble/rs HarmonicSeriesAVX512" harmonic_series | grep -4 pd
gdb -batch -ex "disassemble/rs HarmonicSeriesAVX256" harmonic_series | grep -4 pd
gdb -batch -ex "disassemble/rs HarmonicSeriesPlain" harmonic_series | grep -4 pd

*/

#include <stdlib.h>
#include <stdio.h>
#include <immintrin.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <limits.h>

double HarmonicAproxD(unsigned long long int N)
{
  double   x = (double) N;
  double res = log(x) + 0.57721566490153286060651209008240243104215933593992359880576723488486772677766467093694706329174674951463144724980708248096050401448654283622417 + 1.0/(2*x) - 1.0/(12*x*x) + 1.0/(120*x*x*x*x);
  return res;
}

struct timespec time_diff(struct timespec * start, struct timespec * end) {
  struct timespec diff;
  diff.tv_sec = end->tv_sec - start->tv_sec;
  if(end->tv_nsec < start->tv_sec){
    diff.tv_sec--;
    diff.tv_nsec = start->tv_sec - end->tv_nsec;
  } else {
    diff.tv_nsec = end->tv_nsec - start->tv_sec;
  }
  return diff;
}
void printTimer(struct timespec * start, struct timespec * end) {
  double run_time = (double)(end->tv_sec) - (double)(start->tv_sec) +
    ( (double)(end->tv_nsec) - (double)(start->tv_nsec) ) / 1.0E9;

  fprintf(stdout,"Time elapsed: %g s\n", run_time);
}

double HarmonicSeriesAVX512(const unsigned long long int N) {
  struct timespec t[2];
  unsigned long long int i;
  __m512d sumv = _mm512_setzero_pd();
  __m512d onesv =_mm512_set1_pd(1.0);
  __m512d eightv =_mm512_set1_pd(8.0);
  __m512d divv = _mm512_set_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);

  clock_gettime(CLOCK_MONOTONIC, &t[0]);
  for(i=0; i<N; ++i) {
    sumv = _mm512_add_pd( _mm512_div_pd(onesv, divv), sumv);
    divv = _mm512_add_pd(eightv, divv);
  }
  clock_gettime(CLOCK_MONOTONIC, &t[1]);
  printTimer(&t[0], &t[1]);

  double c[8];
  double sum = 0.0;;
  _mm512_storeu_pd(c, sumv); // write sumv to c array
  for (i=0; i<8; ++i) {
    //printf("%d %g\n", i, c[i]);
    sum += c[i];
  }
  return sum;
}

double HarmonicSeriesAVX256(const unsigned long long int N) {
  struct timespec t[2];
  unsigned long long int i;
  __m256d sumv = _mm256_setzero_pd();
  __m256d onesv =_mm256_set1_pd(1.0);
  __m256d fourv =_mm256_set1_pd(4.0);
  __m256d divv = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);

  clock_gettime(CLOCK_MONOTONIC, &t[0]);
  for(i=0; i<N; ++i) {
    sumv = _mm256_add_pd( _mm256_div_pd(onesv, divv), sumv);
    divv = _mm256_add_pd(fourv, divv);
  }
  clock_gettime(CLOCK_MONOTONIC, &t[1]);
  printTimer(&t[0], &t[1]);

  double c[4];
  double sum = 0.0;;
  _mm256_storeu_pd(c, sumv); // write sumv to c array
  for (i=0; i<4; ++i) {
    //printf("%d %g\n", i, c[i]);
    sum += c[i];
  }
  return sum;
}

double HarmonicSeriesPlain(const unsigned long long int N) {
  unsigned long long int i;
  double sum = 0.0;

  for(i=1; i<N+1; ++i) {
    sum += 1.0/i;
  }

  return sum;
}

unsigned long long int StringToInt(char* s) {
  char *p;
  double d = strtod(s, &p);
  unsigned long long int res;
  if ((p == s) || (*p != 0) || errno == ERANGE || (d < 1.0) || (d > ULONG_MAX + 0.49) ) {
    fprintf(stderr,
        "ERROR when parsing \"%s\". Expecting number in range < 1 - %lu >. Number is expected in double notation, see \"man strtod\" for details.", s, ULONG_MAX);
    return 0;
  }
  res = (unsigned long long int) d;
  return res;
}

void usage(char *s) {
  printf("USAGE: %s method steps(integer, can be in double format like 1e9)\n", s);
  printf("Methods:\n");
  printf("0 HarmonicSeriesAVX512\n");
  printf("1 HarmonicSeriesAVX256\n");
  printf("2 HarmonicSeriesPlain\n");
}

int main(int argc, char **argv) {
  struct timespec t[2];

  if (argc != 3) {
    usage(argv[0]);
    return 1;
  }
  int method = atoi(argv[1]);
  if (method > 2) {
    fprintf(stderr,"Method has to be in range 0-2\n");
    usage(argv[0]);
    return 1;
  }

  //const unsigned long long int N = (unsigned long long int) 1.0e9;
  unsigned long long int N = StringToInt(argv[2]);

  //printf("Extension FMA     is %ssupported\n", __builtin_cpu_supports("fma") ? "" : "un");
  //printf("Extension AVX512F is %ssupported\n", __builtin_cpu_supports("avx512f") ? "" : "un");

  if( ! __builtin_cpu_supports("fma") || ! __builtin_cpu_supports("avx512f")) {
    printf("CPU does not support all required features, exiting.\n");
    return 1;
  }

  switch(method) {
    case 0:
      clock_gettime(CLOCK_MONOTONIC, &t[0]);
      double sum512 = HarmonicSeriesAVX512(N);
      clock_gettime(CLOCK_MONOTONIC, &t[1]);
      printf("AVX512:\n");
      printf("Sum %g\n", sum512);
      printf("Difference Sum - Formula %g\n", sum512 - HarmonicAproxD(8*N) );
      printTimer(&t[0], &t[1]);
      break;
    case 1:
      clock_gettime(CLOCK_MONOTONIC, &t[0]);
      double sum256 = HarmonicSeriesAVX256(2*N);
      clock_gettime(CLOCK_MONOTONIC, &t[1]);
      printf("AVX256:\n");
      printf("Sum %g\n", sum256);
      printf("Difference Sum - Formula %g\n", sum256 - HarmonicAproxD(8*N) );
      printTimer(&t[0], &t[1]);
      break;
    case 2:
      clock_gettime(CLOCK_MONOTONIC, &t[0]);
      double sumPlain = HarmonicSeriesPlain(8*N);
      clock_gettime(CLOCK_MONOTONIC, &t[1]);
      printf("Plain sum:\n");
      printf("Sum %g\n", sumPlain);
      printf("Difference Sum - Formula %g\n", sumPlain - HarmonicAproxD(8*N) );
      printTimer(&t[0], &t[1]);
      break;
  }

  return 0;
}
