#ifndef RFDATA_UTIL_H_
#define RFDATA_UTIL_H_

#undef M_PI
#define M_PI 3.14159265358979323846264338327

#include <time.h>
#include <math.h>
#include <assert.h>

#include <complex>
#include <iostream>
#include <fstream>

typedef std::complex<double> cplx;

const cplx cplxZero(0.0, 0.0);
const cplx imUnit(0.0, 1.0);
const cplx cplxOne(1.0, 1.0);

struct vector {
    double x;
    double y;
    double z;
};

inline double mod(const vector& v) {
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}



class fresnelInt {
 public:
  fresnelInt();
  ~fresnelInt();
  cplx fastFresnel(double x);

 private:
  cplx *fresBase;
  cplx fresnel(double x);
  double interp1(double *x, double *y, int size, double x0);

  double EPSLON, FPMIN, DELTAX, LARGELIM, XMIN, eps;
  int MAXIT, BASESIZE;
};

#endif  // RFDATA_UTIL_H_
