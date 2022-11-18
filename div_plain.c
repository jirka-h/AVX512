/*
gcc -O3 -Wall -Wextra -o div_plain div_plain.c
*/

#include <stdio.h>

int main(void) {
  const unsigned long long int N = (unsigned long long int) 4.0e9;
  unsigned long long int i;
  double div = 1.0000000001;
  double a = 1.49182474713380642;

  for(i=0; i<8*N; ++i) {
    a = a / div;
  }

  printf("%.17f\n", a);

  return 0;
}
