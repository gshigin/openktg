#pragma once

#include <openktg/types.h>

// Simple 4x4 matrix type
using Matrix44 = sF32[4][4];

// 4x4 matrix multiply
static void MatMult(Matrix44 &dest, const Matrix44 &a, const Matrix44 &b)
{
    for (sInt i = 0; i < 4; i++)
        for (sInt j = 0; j < 4; j++)
            dest[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j];
}

// Create a scaling matrix
static void MatScale(Matrix44 &dest, sF32 sx, sF32 sy, sF32 sz)
{
    sSetMem(dest, 0, sizeof(dest));
    dest[0][0] = sx;
    dest[1][1] = sy;
    dest[2][2] = sz;
    dest[3][3] = 1.0f;
}

// Create a translation matrix
static void MatTranslate(Matrix44 &dest, sF32 tx, sF32 ty, sF32 tz)
{
    MatScale(dest, 1.0f, 1.0f, 1.0f);
    dest[3][0] = tx;
    dest[3][1] = ty;
    dest[3][2] = tz;
}

// Create a z-axis rotation matrix
static void MatRotateZ(Matrix44 &dest, sF32 angle)
{
    sF32 s = sFSin(angle);
    sF32 c = sFCos(angle);

    MatScale(dest, 1.0f, 1.0f, 1.0f);
    dest[0][0] = c;
    dest[0][1] = s;
    dest[1][0] = -s;
    dest[1][1] = c;
}