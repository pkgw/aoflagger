/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef MSIO_TYPES
#define MSIO_TYPES

class AntennaInfo;
class BandInfo;
class FieldInfo;
class TimeFrequencyData;
class TimeFrequencyImager;
class FitsFile;

#define NUM_T_IS_FLOAT

#ifdef NUM_T_IS_FLOAT
typedef float num_t;

#define sqrtn(X) sqrtf(X)
#define expn(X) expf(X)
#define logn(X) logf(X)
#define sinn(X) sinf(X)
#define asinn(X) asinf(X)
#define cosn(X) cosf(X)
#define acosn(X) acosf(X)
#define tann(X) tanf(X)
#define atann(X) atanf(X)
#define atan2n(X, Y) atan2(X, Y)
#define fabsn(X) fabsf(X)
#define floorn(X) floorf(X)
#define ceiln(X) ceilf(X)
#define roundn(X) roundf(X)
#define pown(X, Y) powf(X, Y)
#define fmodn(X, Y) fmodf(X, Y)
#define M_PIn M_PI

#else // NOT NUM_T_IS_FLOAT

typedef double num_t;

#define sqrtn(X) sqrt(X)
#define expn(X) exp(X)
#define logn(X) log(X)
#define sinn(X) sin(X)
#define asinn(X) asin(X)
#define cosn(X) cos(X)
#define acosn(X) acos(X)
#define tann(X) tan(X)
#define atann(X) atan(X)
#define atan2n(X, Y) atan2(X, Y)
#define fabsn(X) fabs(X)
#define floorn(X) floor(X)
#define ceiln(X) ceil(X)
#define roundn(X) round(X)
#define pown(X, Y) pow(X, Y)
#define fmodn(X, Y) fmod(X, Y)
#define M_PIn M_PI

#endif

// numl_t is the numeric type for high precision, intermediate calculations
typedef long double numl_t;

#define sqrtnl(X) sqrtl(X)
#define expnl(X) expl(X)
#define lognl(X) logl(X)
#define sinnl(X) sinl(X)
#define asinnl(X) asinl(X)
#define cosnl(X) cosl(X)
#define tannl(X) tanl(X)
#define atannl(X) atanl(X)
#define atan2nl(X, Y) atan2l(X, Y)
#define fabsnl(X) fabsl(X)
#define floornl(X) floorl(X)
#define ceilnl(X) ceill(X)
#define roundnl(X) roundl(X)
#define pownl(X, Y) powl(X, Y)
#define fmodnl(X, Y) fmodl(X, Y)
// M_PIl is not defined on some OS-X systems
#ifndef M_PIl
# define M_PIl          3.1415926535897932384626433832795029L  /* pi */
#endif
#define M_PInl M_PIl

enum DataKind { ObservedData, CorrectedData, ResidualData, ModelData, WeightData };

enum PolarisationType { SinglePolarisation, DipolePolarisation, AutoDipolePolarisation, CrossDipolePolarisation, StokesIPolarisation, StokesQPolarisation, StokesUPolarisation, StokesVPolarisation, XXPolarisation, XYPolarisation, YXPolarisation, YYPolarisation };

class ParmTable;

enum BaselineIOMode { DirectReadMode, IndirectReadMode, MemoryReadMode, AutoReadMode };

#endif // MSIO_TYPES
