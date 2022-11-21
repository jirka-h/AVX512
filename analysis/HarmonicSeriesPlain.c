double HarmonicSeriesPlain() {
  unsigned long long int i;
  double sum = 0.0;

  for(i=1; i<1000000001; ++i) {
    sum += 1.0/i;
  }

  return sum;
}
