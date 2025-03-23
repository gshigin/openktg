#pragma once

#include <cassert>

#include <openktg/gentexture.h>
#include <openktg/types.h>

// Create a simple linear gradient texture
// Input colors are 0xaarrggbb (not premultiplied!)
static auto LinearGradient(sU32 startCol, sU32 endCol) -> GenTexture
{
    GenTexture tex;

    tex.Init(2, 1);
    tex.Data[0].Init(startCol);
    tex.Data[1].Init(endCol);

    return tex;
}

// Create a pattern of randomly colored voronoi cells
static void RandomVoronoi(GenTexture &dest, const GenTexture &grad, sInt intensity, sInt maxCount, sF32 minDist)
{
    srand(0);
    assert(maxCount <= 256);
    CellCenter centers[256];

    // generate random center points
    for (sInt i = 0; i < maxCount; i++)
    {
        int intens = (1.0f * rand() * intensity) / RAND_MAX;

        centers[i].x = 1.0f * rand() / RAND_MAX;
        centers[i].y = 1.0f * rand() / RAND_MAX;
        centers[i].color.Init(intens, intens, intens, 255);
    }

    // remove points too close together
    sF32 minDistSq = minDist * minDist;
    for (sInt i = 1; i < maxCount;)
    {
        sF32 x = centers[i].x;
        sF32 y = centers[i].y;

        // try to find a point closer than minDist
        sInt j;
        for (j = 0; j < i; j++)
        {
            sF32 dx = centers[j].x - x;
            sF32 dy = centers[j].y - y;

            if (dx < 0.0f)
                dx += 1.0f;
            if (dy < 0.0f)
                dy += 1.0f;

            dx = sMin(dx, 1.0f - dx);
            dy = sMin(dy, 1.0f - dy);

            if (dx * dx + dy * dy < minDistSq) // point is too close, stop
                break;
        }

        if (j < i)                            // we found such a point
            centers[i] = centers[--maxCount]; // remove this one
        else                                  // accept this one
            i++;
    }

    // generate the image
    dest.Cells(grad, centers, maxCount, 0.0f, GenTexture::CellInner);
}

// Transforms a grayscale image to a colored one with a matrix transform
static void Colorize(GenTexture &img, sU32 startCol, sU32 endCol)
{
    Matrix44 m;
    Pixel s, e;

    s.Init(startCol);
    e.Init(endCol);

    // calculate matrix
    sSetMem(m, 0, sizeof(m));
    m[0][0] = (e.r - s.r) / 65535.0f;
    m[1][1] = (e.g - s.g) / 65535.0f;
    m[2][2] = (e.b - s.b) / 65535.0f;
    m[3][3] = 1.0f;
    m[0][3] = s.r / 65535.0f;
    m[1][3] = s.g / 65535.0f;
    m[2][3] = s.b / 65535.0f;

    // transform
    img.ColorMatrixTransform(img, m, sTRUE);
}