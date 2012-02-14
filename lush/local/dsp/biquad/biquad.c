#include <math.h>
#include "biquad.h"

////////////////////////////////////////////////////////////////
// biquad low-pass filter
// info at:
// http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
//
// Direct Form 1:
//
//    y[n] = (b0/a0)*x[n] + (b1/a0)*x[n-1] + (b2/a0)*x[n-2]
//                        - (a1/a0)*y[n-1] - (a2/a0)*y[n-2]  
//    A  = sqrt( 10^(dBgain/20) )
//       =       10^(dBgain/40)     (for peaking and shelving EQ filters only)
//
//    w0 = 2*pi*f0/Fs
//
//    cos(w0)
//    sin(w0)
//
//    alpha = sin(w0)/(2*Q)                                       (case: Q)
//          = sin(w0)*sinh( ln(2)/2 * BW * w0/sin(w0) )           (case: BW)
//          = sin(w0)/2 * sqrt( (A + 1/A)*(1/S - 1) + 2 )         (case: S)
//
//        FYI: The relationship between bandwidth and Q is
//             1/Q = 2*sinh(ln(2)/2*BW*w0/sin(w0))     (digital filter w BLT)
//        or   1/Q = 2*sinh(ln(2)/2*BW)             (analog filter prototype)
//
//        The relationship between shelf slope and Q is
//             1/Q = sqrt((A + 1/A)*(1/S - 1) + 2)
//
//    2*sqrt(A)*alpha  =  sin(w0) * sqrt( (A^2 + 1)*(1/S - 1) + 2*A )
//        is a handy intermediate variable for shelving EQ filters.
//
//Finally, compute the coefficients for whichever filter type you want:
//   (The analog prototypes, H(s), are shown for each filter
//        type for normalized frequency.)
//
//LPF:        H(s) = 1 / (s^2 + s/Q + 1)
//
//            b0 =  (1 - cos(w0))/2
//            b1 =   1 - cos(w0)
//            b2 =  (1 - cos(w0))/2
//            a0 =   1 + alpha
//            a1 =  -2*cos(w0)
//            a2 =   1 - alpha
//
//HPF:        H(s) = s^2 / (s^2 + s/Q + 1)
//
//            b0 =  (1 + cos(w0))/2
//            b1 = -(1 + cos(w0))
//            b2 =  (1 + cos(w0))/2
//            a0 =   1 + alpha
//            a1 =  -2*cos(w0)
//            a2 =   1 - alpha


// initialize state variables.
// This must be called once, at least.
int BiquadClear(Biquad *bq) {
  bq->x1 = 0;
  bq->x2 = 0;
  bq->y1 = 0;
  bq->y2 = 0;
  return 0;
}

// Fill up and initialize a biquad filter structure.
// type = is 1 for high pass, 0 for low pass
// f0 = cutoff frequency
// q = resonance
// fs sampling rate
int BiquadSet(Biquad *bq, int type, float f0, float q, float fs) {
  // float A = sqrt(powf(10, gain/20));
  float w0 = 2*M_PI*f0/fs;
  float cosw0 = cosf(w0);
  float sinw0 = sinf(w0);
  float alpha = sinw0/(2.0*q);
  float a0 = 1.0 + alpha;
  if (type == 0) {
    // low pass filter
    bq->b0 = (1.0 - cosw0)/(2.0*a0);
    bq->b1 = (1.0 - cosw0)/a0;
    bq->b2 = bq->b0;
    bq->a1 =-2.0*cosw0/a0;
    bq->a2 = (1.0 - alpha)/a0;
  } else {
    // high pass filter
    bq->b0 = (1.0 + cosw0)/(2.0*a0);
    bq->b1 =-(1.0 + cosw0)/a0;
    bq->b2 = bq->b0;
    bq->a1 =-2.0*cosw0/a0;
    bq->a2 = (1.0 - alpha)/a0;
  }
  BiquadClear(bq);
  return 0;
}

// run a biquad on a chunk of data
// bq; pointer to biquad object
// n: number of samples
// input: pointer to input samples
// output: pointer to output buffer (can be the same as input)
int BiquadRun(Biquad *bq, int n, float *input, float *output) {
  int i;
  float b0 = bq->b0, b1 = bq->b1, b2 = bq->b2;
  float a1 = bq->a1, a2 = bq->a2;
  float x1 = bq->x1, x2 = bq->x2;
  float y1 = bq->y1, y2 = bq->y2;
  for (i=0; i<n; i++) {
    float out = b0*input[i] + b1*x1 + b2*x2 - a1*y1 - a2*y2;
    x2 = x1;
    x1 = input[i];
    y2 = y1;
    y1 = out;
    output[i] = out;
  }
  bq->x1 = x1; bq->x2 = x2;
  bq->y1 = y1; bq->y2 = y2;
  return 0;
}
