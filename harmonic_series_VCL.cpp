/*
g++ -Wall -Wextra -O3 -fno-trapping-math -fno-math-errno -I include -fopenmp -march=native -std=c++17 -o harmonic_series_VCL harmonic_series_VCL.cpp
g++ -Wall -Wextra -O2 -g -fsanitize=undefined -I include -fopenmp -march=native            -std=c++17 -o harmonic_series_VCL harmonic_series_VCL.cpp
OMP_NUM_THREADS=1 time ./harmonic_series_VLC 1 1e9

Harmonic series summation https://en.wikipedia.org/wiki/Harmonic_series_(mathematics) with
  * Agner Fog's VCL (Vector Class Library) - https://www.agner.org/optimize/#manual_cpp https://github.com/vectorclass/version2
  * OpenMP parralelization
  * Testing different number of accumulators - see Chapter "9.7 Using multiple accumulators" from https://www.agner.org/optimize/vcl_manual.pdf

Performance / bottleneck analysis
1) AMD
OMP_NUM_THREADS=1 perf stat -M PipelineL1 ./avx.D.x 5
OMP_NUM_THREADS=1 perf stat -M PipelineL2 ./avx.D.x 5
OMP_NUM_THREADS=1 perf stat -M backend_bound_group ./avx.D.x 5

2)Intel
OMP_NUM_THREADS=1 perf stat -M pipeline ./avx.D.x 5

(c) Copyright 2021-2023 Jirka Hladky
GNU Affero General Public License
*/

#include <iostream>
#include <vectorclass.h>
#include <omp.h>
#include <cmath>
using std::cout;
using std::cin;
using std::endl;

double time_diff(const struct timespec * start, const struct timespec * end) {
  double run_time = (double)(end->tv_sec) - (double)(start->tv_sec) +
    ( (double)(end->tv_nsec) - (double)(start->tv_nsec) ) / 1.0E9;
  return run_time;
}

void printTimer(const struct timespec * start, const struct timespec * end) {
  double run_time = time_diff(start, end);
  printf("Time elapsed: %g s\n", run_time);
}

double HarmonicAproxD(unsigned long long int N)
{
  double   x = (double) N;
  double res = log(x) + 0.57721566490153286060651209008240243104215933593992359880576723488486772677766467093694706329174674951463144724980708248096050401448654283622417 + 1.0/(2*x) - 1.0/(12*x*x) + 1.0/(120*x*x*x*x);
  return res;
}

struct kahan_sum {
  double sum;
  double cor;
};

int HarmonicSeriesFourAccumulators(const unsigned long long int N, struct kahan_sum * k) {
  unsigned long long int i;
  Vec8d divV1(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
  Vec8d divV2(9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0);
  Vec8d divV3(17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0);
  Vec8d divV4(25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0);
  Vec8d sumV1(0.0);
  Vec8d sumV2(0.0);
  Vec8d sumV3(0.0);
  Vec8d sumV4(0.0);
  const Vec8d addV(32.0);
  const Vec8d oneV(1.0);

  //Kahan summation
  const Vec8d zeroV(0.0);
  Vec8d c1(0.0);
  Vec8d c2(0.0);
  Vec8d c3(0.0);
  Vec8d c4(0.0);
  Vec8d y1, t1;
  Vec8d y2, t2;
  Vec8d y3, t3;
  Vec8d y4, t4;

  bool first_loop = true;
  const Vec8d startdivV1 = divV1;
  const Vec8d startdivV2 = divV2;
  const Vec8d startdivV3 = divV3;
  const Vec8d startdivV4 = divV4;
  #pragma omp declare reduction( + : Vec8d : omp_out = omp_out + omp_in ) initializer (omp_priv=omp_orig)
//It's important to mark "first_loop" variable as firstprivate so that each private copy gets initialized.
//firstprivate	Specifies that each thread should have its own instance of a variable, and that the variable should be initialized with the value of the variable, because it exists before the parallel construct.
//lastprivate	Specifies that the enclosing context's version of the variable is set equal to the private version of whichever thread executes the final iteration (for-loop construct) or last section (#pragma sections).
  #pragma omp parallel for firstprivate(first_loop) lastprivate(divV1, divV2, divV3, divV4) private(y1, y2, y3, y4, t1, t2, t3, t4) reduction(+:sumV1, sumV2, sumV3, sumV4, c1, c2, c3, c4)
  for(i=0; i<N; ++i) {
    if (first_loop) {
      first_loop = false;
      divV1 = startdivV1 + i * addV;
      divV2 = startdivV2 + i * addV;
      divV3 = startdivV3 + i * addV;
      divV4 = startdivV4 + i * addV;
      c1 = zeroV;
      c2 = zeroV;
      c3 = zeroV;
      c4 = zeroV;
      sumV1 = oneV / divV1;
      sumV2 = oneV / divV2;
      sumV3 = oneV / divV3;
      sumV4 = oneV / divV4;
    } else {
      divV1 += addV;
      divV2 += addV;
      divV3 += addV;
      divV4 += addV;
      //sumV += oneV / divV;
      //Kahan summation
      //Algebraically, c is always 0
      //But, when there is a loss in precision, the higher-order y is cancelled out by subtracting y from c and
      //all that remains is the lower-order error in c
      y1 = oneV / divV1 - c1;
      y2 = oneV / divV2 - c2;
      y3 = oneV / divV3 - c3;
      y4 = oneV / divV4 - c4;
      t1 = sumV1 + y1;
      t2 = sumV2 + y2;
      t3 = sumV3 + y3;
      t4 = sumV4 + y4;
      c1 = ( t1 - sumV1 ) - y1;
      c2 = ( t2 - sumV2 ) - y2;
      c3 = ( t3 - sumV3 ) - y3;
      c4 = ( t4 - sumV4 ) - y4;
      sumV1 = t1;
      sumV2 = t2;
      sumV3 = t3;
      sumV4 = t4;
    }
  }
#if 0
  for (int j=0; j<8; ++j) {
    printf("%.0f, ", divV1[j]);
  }
  printf("\n");
  for (int j=0; j<8; ++j) {
    printf("%.0f, ", divV2[j]);
  }
  printf("\n");
  for (int j=0; j<8; ++j) {
    printf("%.0f, ", divV3[j]);
  }
  printf("\n");
  for (int j=0; j<8; ++j) {
    printf("%.0f, ", divV4[j]);
  }
  printf("\n");
#endif
  //Here, stricly speaking,  we should use Kahan sumation as well
  //double sum = horizontal_add(sumV1) + horizontal_add(sumV2) + horizontal_add(sumV3) + horizontal_add(sumV4);
  //double lower_order_error = horizontal_add(c1) + horizontal_add(c2) + horizontal_add(c3) + horizontal_add(c4);
  //printf("Sum: %g, lower-order error: %g\n", sum, lower_order_error);
  //return sum - lower_order_error;
  double partial_sums[2*8*4]; //We will store here both sumV* and c* accumulators
  c1 = -c1;
  c2 = -c2;
  c3 = -c3;
  c4 = -c4;
  //Fill the array from largest to smallest
  sumV1.store(partial_sums );
  sumV2.store(partial_sums + 8 );
  sumV3.store(partial_sums + 16 );
  sumV4.store(partial_sums + 24 );

  c1.store(partial_sums + 32);
  c2.store(partial_sums + 40 );
  c3.store(partial_sums + 48 );
  c4.store(partial_sums + 56 );


  double sum = 0.0;
  double c = 0.0;
  double y, t;
  for (int j=sizeof(partial_sums)/sizeof(partial_sums[0])-1; j>=0; --j) {
    y = partial_sums[j] - c;
    t = sum + y;
    c = (t - sum) - y;
    sum = t;
  }

  //printf("Sum: %g, lower-order error: %g\n", sum, c);
  k->sum = sum;
  k->cor = c;
  return 0;
}

int HarmonicSeriesThreeAccumulators(const unsigned long long int N, struct kahan_sum * k) {
  unsigned long long int i;
  Vec8d divV1(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
  Vec8d divV2(9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0);
  Vec8d divV3(17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0);
  Vec8d sumV1(0.0);
  Vec8d sumV2(0.0);
  Vec8d sumV3(0.0);
  const Vec8d addV(24.0);
  const Vec8d oneV(1.0);

  //Kahan summation
  const Vec8d zeroV(0.0);
  Vec8d c1(0.0);
  Vec8d c2(0.0);
  Vec8d c3(0.0);
  Vec8d y1, t1;
  Vec8d y2, t2;
  Vec8d y3, t3;

  bool first_loop = true;
  const Vec8d startdivV1 = divV1;
  const Vec8d startdivV2 = divV2;
  const Vec8d startdivV3 = divV3;
  #pragma omp declare reduction( + : Vec8d : omp_out = omp_out + omp_in ) initializer (omp_priv=omp_orig)
//It's important to mark "first_loop" variable as firstprivate so that each private copy gets initialized.
//firstprivate	Specifies that each thread should have its own instance of a variable, and that the variable should be initialized with the value of the variable, because it exists before the parallel construct.
//lastprivate	Specifies that the enclosing context's version of the variable is set equal to the private version of whichever thread executes the final iteration (for-loop construct) or last section (#pragma sections).
  #pragma omp parallel for firstprivate(first_loop) lastprivate(divV1, divV2, divV3) private(y1, y2, y3, t1, t2, t3) reduction(+:sumV1, sumV2, sumV3, c1, c2, c3)
  for(i=0; i<N; ++i) {
    if (first_loop) {
      first_loop = false;
      divV1 = startdivV1 + i * addV;
      divV2 = startdivV2 + i * addV;
      divV3 = startdivV3 + i * addV;
      c1 = zeroV;
      c2 = zeroV;
      c3 = zeroV;
      sumV1 = oneV / divV1;
      sumV2 = oneV / divV2;
      sumV3 = oneV / divV3;
    } else {
      divV1 += addV;
      divV2 += addV;
      divV3 += addV;
      //sumV += oneV / divV;
      //Kahan summation
      //Algebraically, c is always 0
      //But, when there is a loss in precision, the higher-order y is cancelled out by subtracting y from c and
      //all that remains is the lower-order error in c
      y1 = oneV / divV1 - c1;
      y2 = oneV / divV2 - c2;
      y3 = oneV / divV3 - c3;
      t1 = sumV1 + y1;
      t2 = sumV2 + y2;
      t3 = sumV3 + y3;
      c1 = ( t1 - sumV1 ) - y1;
      c2 = ( t2 - sumV2 ) - y2;
      c3 = ( t3 - sumV3 ) - y3;
      sumV1 = t1;
      sumV2 = t2;
      sumV3 = t3;
    }
  }
#if 0
  for (int j=0; j<8; ++j) {
    printf("%.0f, ", divV1[j]);
  }
  printf("\n");
  for (int j=0; j<8; ++j) {
    printf("%.0f, ", divV2[j]);
  }
  printf("\n");
  for (int j=0; j<8; ++j) {
    printf("%.0f, ", divV3[j]);
  }
  printf("\n");
#endif
  //Here, stricly speaking,  we should use Kahan sumation as well
  //double sum = horizontal_add(sumV1) + horizontal_add(sumV2) + horizontal_add(sumV3) + horizontal_add(sumV4);
  //double lower_order_error = horizontal_add(c1) + horizontal_add(c2) + horizontal_add(c3) + horizontal_add(c4);
  //printf("Sum: %g, lower-order error: %g\n", sum, lower_order_error);
  //return sum - lower_order_error;
  double partial_sums[2*8*3]; //We will store here both sumV* and c* accumulators
  c1 = -c1;
  c2 = -c2;
  c3 = -c3;
  //Fill the array from largest to smallest
  sumV1.store(partial_sums );
  sumV2.store(partial_sums + 8 );
  sumV3.store(partial_sums + 16 );

  c1.store(partial_sums + 24);
  c2.store(partial_sums + 32 );
  c3.store(partial_sums + 40 );

  double sum = 0.0;
  double c = 0.0;
  double y, t;
  for (int j=sizeof(partial_sums)/sizeof(partial_sums[0])-1; j>=0; --j) {
    y = partial_sums[j] - c;
    t = sum + y;
    c = (t - sum) - y;
    sum = t;
  }

  //printf("Sum: %g, lower-order error: %g\n", sum, c);
  k->sum = sum;
  k->cor = c;
  return 0;
}

int HarmonicSeriesTwoAccumulators(const unsigned long long int N, struct kahan_sum * k) {
  unsigned long long int i;
  Vec8d divV1(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
  Vec8d divV2(9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0);
  Vec8d sumV1(0.0);
  Vec8d sumV2(0.0);
  const Vec8d addV(16.0);
  const Vec8d oneV(1.0);

  //Kahan summation
  const Vec8d zeroV(0.0);
  Vec8d c1(0.0);
  Vec8d c2(0.0);
  Vec8d y1, t1;
  Vec8d y2, t2;

  bool first_loop = true;
  const Vec8d startdivV1 = divV1;
  const Vec8d startdivV2 = divV2;
  #pragma omp declare reduction( + : Vec8d : omp_out = omp_out + omp_in ) initializer (omp_priv=omp_orig)
//It's important to mark "first_loop" variable as firstprivate so that each private copy gets initialized.
//firstprivate	Specifies that each thread should have its own instance of a variable, and that the variable should be initialized with the value of the variable, because it exists before the parallel construct.
//lastprivate	Specifies that the enclosing context's version of the variable is set equal to the private version of whichever thread executes the final iteration (for-loop construct) or last section (#pragma sections).
  #pragma omp parallel for firstprivate(first_loop) lastprivate(divV1, divV2) private(y1, y2, t1, t2) reduction(+:sumV1, sumV2, c1, c2)
  for(i=0; i<N; ++i) {
    if (first_loop) {
      first_loop = false;
      divV1 = startdivV1 + i * addV;
      divV2 = startdivV2 + i * addV;
      c1 = zeroV;
      c2 = zeroV;
      sumV1 = oneV / divV1;
      sumV2 = oneV / divV2;
    } else {
      divV1 += addV;
      divV2 += addV;
      //sumV += oneV / divV;
      //Kahan summation
      //Algebraically, c is always 0
      //But, when there is a loss in precision, the higher-order y is cancelled out by subtracting y from c and
      //all that remains is the lower-order error in c
      y1 = oneV / divV1 - c1;
      y2 = oneV / divV2 - c2;
      t1 = sumV1 + y1;
      t2 = sumV2 + y2;
      c1 = ( t1 - sumV1 ) - y1;
      c2 = ( t2 - sumV2 ) - y2;
      sumV1 = t1;
      sumV2 = t2;
    }
  }
#if 0
  for (int j=0; j<8; ++j) {
    printf("%.0f, ", divV1[j]);
  }
  printf("\n");
  for (int j=0; j<8; ++j) {
    printf("%.0f, ", divV2[j]);
  }
  printf("\n");
  for (int j=0; j<8; ++j) {
    printf("%.0f, ", divV3[j]);
  }
  printf("\n");
#endif
  //Here, stricly speaking,  we should use Kahan sumation as well
  //double sum = horizontal_add(sumV1) + horizontal_add(sumV2) + horizontal_add(sumV3) + horizontal_add(sumV4);
  //double lower_order_error = horizontal_add(c1) + horizontal_add(c2) + horizontal_add(c3) + horizontal_add(c4);
  //printf("Sum: %g, lower-order error: %g\n", sum, lower_order_error);
  //return sum - lower_order_error;
  double partial_sums[2*8*2]; //We will store here both sumV* and c* accumulators
  c1 = -c1;
  c2 = -c2;
  //Fill the array from largest to smallest
  sumV1.store(partial_sums);
  sumV2.store(partial_sums + 8 );

  c1.store(partial_sums + 16 );
  c2.store(partial_sums + 24 );

  double sum = 0.0;
  double c = 0.0;
  double y, t;
  for (int j=sizeof(partial_sums)/sizeof(partial_sums[0])-1; j>=0; --j) {
    y = partial_sums[j] - c;
    t = sum + y;
    c = (t - sum) - y;
    sum = t;
  }

  //printf("Sum: %g, lower-order error: %g\n", sum, c);
  k->sum = sum;
  k->cor = c;
  return 0;
}

int HarmonicSeriesOneAccumulators(const unsigned long long int N, struct kahan_sum * k) {
  unsigned long long int i;
  Vec8d divV1(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
  Vec8d sumV1(0.0);
  const Vec8d addV(8.0);
  const Vec8d oneV(1.0);

  //Kahan summation
  const Vec8d zeroV(0.0);
  Vec8d c1(0.0);
  Vec8d y1, t1;

  bool first_loop = true;
  const Vec8d startdivV1 = divV1;
  #pragma omp declare reduction( + : Vec8d : omp_out = omp_out + omp_in ) initializer (omp_priv=omp_orig)
//It's important to mark "first_loop" variable as firstprivate so that each private copy gets initialized.
//firstprivate	Specifies that each thread should have its own instance of a variable, and that the variable should be initialized with the value of the variable, because it exists before the parallel construct.
//lastprivate	Specifies that the enclosing context's version of the variable is set equal to the private version of whichever thread executes the final iteration (for-loop construct) or last section (#pragma sections).
  #pragma omp parallel for firstprivate(first_loop) lastprivate(divV1) private(y1, t1) reduction(+:sumV1, c1)
  for(i=0; i<N; ++i) {
    if (first_loop) {
      first_loop = false;
      divV1 = startdivV1 + i * addV;
      c1 = zeroV;
      sumV1 = oneV / divV1;
    } else {
      divV1 += addV;
      //sumV += oneV / divV;
      //Kahan summation
      //Algebraically, c is always 0
      //But, when there is a loss in precision, the higher-order y is cancelled out by subtracting y from c and
      //all that remains is the lower-order error in c
      y1 = oneV / divV1 - c1;
      t1 = sumV1 + y1;
      c1 = ( t1 - sumV1 ) - y1;
      sumV1 = t1;
    }
  }
#if 0
  for (int j=0; j<8; ++j) {
    printf("%.0f, ", divV1[j]);
  }
  printf("\n");
  for (int j=0; j<8; ++j) {
    printf("%.0f, ", divV2[j]);
  }
  printf("\n");
  for (int j=0; j<8; ++j) {
    printf("%.0f, ", divV3[j]);
  }
  printf("\n");
#endif
  //Here, stricly speaking,  we should use Kahan sumation as well
  //double sum = horizontal_add(sumV1) + horizontal_add(sumV2) + horizontal_add(sumV3) + horizontal_add(sumV4);
  //double lower_order_error = horizontal_add(c1) + horizontal_add(c2) + horizontal_add(c3) + horizontal_add(c4);
  //printf("Sum: %g, lower-order error: %g\n", sum, lower_order_error);
  //return sum - lower_order_error;
  double partial_sums[2*8*1]; //We will store here both sumV* and c* accumulators
  c1 = -c1;
  //Fill the array from largest to smallest
  sumV1.store(partial_sums );
  c1.store(partial_sums + 8);

  double sum = 0.0;
  double c = 0.0;
  double y, t;
  for (int j=sizeof(partial_sums)/sizeof(partial_sums[0])-1; j>=0; --j) {
    y = partial_sums[j] - c;
    t = sum + y;
    c = (t - sum) - y;
    sum = t;
  }

  //printf("Sum: %g, lower-order error: %g\n", sum, c);
  k->sum = sum;
  k->cor = c;
  return 0;
}

void print_usage_and_exit(char *prog_name) {
  printf("USAGE: %s method [steps] (integer, can be in double format like 1e9)\n", prog_name);
  printf("Methods:\n");
  printf("1: HarmonicSeriesOneAccumulators \n");
  printf("2: HarmonicSeriesTwoAccumulators\n");
  printf("3: HarmonicSeriesThreeAccumulators \n");
  printf("4: HarmonicSeriesFourAccumulators\n");
  exit(EXIT_FAILURE);
}

int string_to_double(const char *a, double *r, const double min, const double max) {
  char *p;
  double d = strtod(a, &p);
  if ((p == a) || (*p != 0) || errno == ERANGE || (d < min ) || (d > max ) ) {
    fprintf(stderr,"ERROR when parsing \"%s\" as double value. Expecting number in range < %g - %g > in double notation, see \"man strtod\" for details.\n", a, min, max);
    return 1;
  }
  *r = d;
  return 0;
}

int main(int argc, char **argv) {
  struct timespec t[2];

  if (argc < 2 || argc > 3 ) {
    fprintf(stderr, "Unsupported number of arguments\n");
    print_usage_and_exit(argv[0]);
  }
  int method = atoi(argv[1]);

  if (method > 4 || method < 1) {
    fprintf(stderr,"Method has to be in range 1-4\n");
    print_usage_and_exit(argv[0]);
  }

  double terms = 1e9;
  if ( argc == 3 ) {
    if ( string_to_double(argv[2], &terms, 1, 1.0e15) > 0 ) print_usage_and_exit(argv[0]);
  }

  unsigned long long int N = (u_int64_t) (terms / 8.0);
  N = ( ( N + 11) / 12 ) * 12;
  struct kahan_sum res;

  switch(method) {
    case 1:
      printf("HarmonicSeriesOneAccumulators:\n");
      clock_gettime(CLOCK_MONOTONIC, &t[0]);
      HarmonicSeriesOneAccumulators(N, &res);
      clock_gettime(CLOCK_MONOTONIC, &t[1]);
      break;
    case 2:
      printf("HarmonicSeriesTwoAccumulators:\n");
      clock_gettime(CLOCK_MONOTONIC, &t[0]);
      HarmonicSeriesTwoAccumulators(N/2, &res);
      clock_gettime(CLOCK_MONOTONIC, &t[1]);
      break;
    case 3:
      printf("HarmonicSeriesThreeAccumulators:\n");
      clock_gettime(CLOCK_MONOTONIC, &t[0]);
      HarmonicSeriesThreeAccumulators(N/3, &res);
      clock_gettime(CLOCK_MONOTONIC, &t[1]);
      break;
    case 4:
      printf("HarmonicSeriesFourAccumulators:\n");
      clock_gettime(CLOCK_MONOTONIC, &t[0]);
      HarmonicSeriesFourAccumulators(N/4, &res);
      clock_gettime(CLOCK_MONOTONIC, &t[1]);
      break;
  }

  double elapsed_time = time_diff(&t[0], &t[1]);
  printf("Sum of first %llu (%g) elements of Harmonic Series completed in %g seconds using %d threads.\n", 8*N, (double) (8*N), elapsed_time, omp_get_max_threads());
  printf("Sum %g, lower order correction term %g\n", res.sum, -res.cor);
  double controlSum = HarmonicAproxD(8*N);
  double epsilon[2];
  double next;
  epsilon[0] = nextafter(controlSum, 2.0*controlSum + 1.0);
  epsilon[1] = nextafter(controlSum, -2.0*controlSum - 1.0);
  if (res.sum >= controlSum) {
    next = epsilon[0];
  } else {
    next = epsilon[1];
  }
  printf("Difference Sum - Formula %g. Difference is %g epsilons.\n", res.sum - controlSum, floor ( (res.sum - controlSum) / (next - controlSum) ) );
  printf("Sum:    \t%a\nFormula:\t%a\n", res.sum, controlSum);
  printf("Avg: %g operations/second\n", (double) (8 * N) / elapsed_time);
  printf("Mop/s total = %g\n", (double) (8 * N) / elapsed_time / 1e6);
  return EXIT_SUCCESS;
}
