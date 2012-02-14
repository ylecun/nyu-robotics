#include <math.h>

typedef struct {
  // state variables
  float x1, x2, y1, y2;
  // coefficients
  float b0, b1, b2, a1, a2;
} Biquad;

int BiquadSet(Biquad *bq, int type, float f0, float q, float fs);

int BiquadRun(Biquad *bq, int n, float *input, float *output);
