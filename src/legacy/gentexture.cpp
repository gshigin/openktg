/****************************************************************************/
/***                                                                      ***/
/***   Written by Fabian Giesen.                                          ***/
/***   I hereby place this code in the public domain.                     ***/
/***                                                                      ***/
/****************************************************************************/

#include <cassert>

#include <openktg/core/matrix.h>
#include <openktg/core/pixel.h>
#include <openktg/core/texture.h>
#include <openktg/legacy/gentexture.h>
#include <openktg/noise/perlin.h>
#include <openktg/tex/sampling.h>
#include <openktg/util/helpers.h>

auto SizeMatchesWith(const openktg::texture &x, const openktg::texture &y) -> sBool
{
    return y.width() == x.width() && y.height() == x.height();
}

// ---- The operators themselves

