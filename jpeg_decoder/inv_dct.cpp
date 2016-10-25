#include "inv_dct.h"


double mat_a[64] = {
	//   1   2   3   4   5   6   7   8
	va, va, va, va, va, va, va, va,
	vb, vd, ve, vg,-vg,-ve,-vd,-vb,
	vc, vf,-vf,-vc,-vc,-vf, vf, vc,
	vd,-vg,-vb,-ve, ve, vb, vg,-vd,
	va,-va,-va, va, va,-va,-va, va,
	ve,-vb, vg, vd,-vd,-vg, vb,-ve,
	vf,-vc, vc,-vf,-vf, vc,-vc, vf,
	vg,-ve, vd,-vb, vb,-vd, ve,-vg
};


int pos64(int u, int v)
{
	return u * 8 + v;
}

void trans64(double* in, double* out)
{
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			out[pos64(i, j)] = in[pos64(j, i)];
}

void mul64(double* a, double* b, double* r)
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			r[pos64(i, j)] = 0;
			for (int k = 0; k < 8; k++)
			{
				r[pos64(i, j)] += a[pos64(i, k)] * b[pos64(k, j)];
			}
		}

	}
		
}

void inv_dct64(double* in, double* out)
{
	double t1[64];
	double t2[64];
	double t3[64];

	mul64(in, mat_a, t1);
	trans64(t1, t2);
	mul64(t2, mat_a, t3);
	trans64(t3, out);

}