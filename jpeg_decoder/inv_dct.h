#ifndef INV_DCT_H_INCLUDED
#define INV_DCT_H_INCLUDED

#include <math.h>

void inv_dct64(double* in, double* out);
void trans64(double* in, double* out);
void mul64(double* a, double* b, double* r);
int pos64(int u, int v);

const double pi = 3.1415926;
const double va = 0.5 * cos(pi / 4);
const double vb = 0.5 * cos(pi / 16);
const double vc = 0.5 * cos(pi / 8);
const double vd = 0.5 * cos(3 * pi / 16);
const double ve = 0.5 * cos(5 * pi / 16);
const double vf = 0.5 * cos(3 * pi / 8);
const double vg = 0.5 * cos(7 * pi / 16);


#endif
