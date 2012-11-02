#include "fourproductcorrelatortester.h"

#include "observatorium.h"
#include "model.h"
#include "uvimager.h"

FourProductCorrelatorTester::FourProductCorrelatorTester(class Model &model, class UVImager &imager, class Observatorium &observatorium)
	: _model(model), _imager(imager), _observatorium(observatorium), _incoherent(false)
{
}

void FourProductCorrelatorTester::SimulateObservation(num_t delayDirectionDEC, num_t delayDirectionRA, num_t frequency)
{
	size_t frequencySteps = 1;

	for(size_t f=0;f<frequencySteps;++f)
	{
		double channelFrequency = frequency + _observatorium.ChannelWidthHz() * f * 256 / frequencySteps;
		for(size_t i=0;i<_observatorium.AntennaCount();++i)
		{
			for(size_t j=0;j<_observatorium.AntennaCount();++j)
			{
				for(size_t k=i;k<_observatorium.AntennaCount();++k)
				{
					for(size_t l=j;l<_observatorium.AntennaCount();++l)
					{
						bool notAnyAutoCorrelation = true;
						if(!notAnyAutoCorrelation || !(i==j || i==k || i==l || j==k || j==l || k==l))
						{
							if(!(i == j && j == k && k == l))
							{
								const AntennaInfo
									&a1 = _observatorium.GetAntenna(i),
									&a2 = _observatorium.GetAntenna(j),
									&a3 = _observatorium.GetAntenna(k),
									&a4 = _observatorium.GetAntenna(l);
			
								SimulateCorrelation(delayDirectionDEC, delayDirectionRA, a1, a2, a3, a4, channelFrequency, 12*60*60, 10.0);
							}
						}
					}
				}
			}
			std::cout << "." << std::flush;
		}
	}
	std::cout << std::endl;
}

void FourProductCorrelatorTester::complexSqrt(num_t &r, num_t &i)
{
	num_t rtmp = r;
	num_t a = sqrt(r*r + i*i);
	r = sqrt((a + r)/2.0);
	if(i != 0)
		i = i / sqrt(2.0*(a + rtmp));
}

void FourProductCorrelatorTester::amplitudeSqrt(num_t &r, num_t &i)
{
	num_t factor = 1.0 / pow(r*r + i*i, 1.0/4.0);
	r *= factor;
	i *= factor;
}

void FourProductCorrelatorTester::phaseMul2(num_t &r, num_t &i)
{
	num_t rtmp = r;
	num_t a = sqrt(r*r + i*i);
	r = (r*r - i*i) / a;
	i = (2.0 * rtmp * i) / a;
}

void FourProductCorrelatorTester::SimulateAntenna(double time, num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t frequency, num_t earthLattitude, num_t &r, num_t &i, size_t index)
{
	if(_incoherent)
		_model.SimulateUncoherentAntenna(time, delayDirectionDEC, delayDirectionRA, dx, dy, frequency, earthLattitude, r, i, index);
	else
		_model.SimulateAntenna(time, delayDirectionDEC, delayDirectionRA, dx, dy, frequency, earthLattitude, r, i);
}

void FourProductCorrelatorTester::SimulateCorrelation(num_t delayDirectionDEC, num_t delayDirectionRA, const AntennaInfo &a1, const AntennaInfo &a2, const AntennaInfo &a3, const AntennaInfo &a4, num_t frequency, double totalTime, double integrationTime)
{
	num_t
		x = a1.position.x,
		y = a1.position.y,
		z = a1.position.z,
		dx2 = x - a2.position.x,
		dy2 = y - a2.position.y,
		dz2 = z - a2.position.z,
		dx3 = x - a3.position.x,
		dy3 = y - a3.position.y,
		dz3 = z - a3.position.z,
		dx4 = x - a4.position.x,
		dy4 = y - a4.position.y,
		dz4 = z - a4.position.z,
		combdx = dx2 - dx3 + dx4,
		combdy = dy2 - dy3 + dy4,
		combdz = dz2 - dz3 + dz4;

	num_t wavelength = 1.0L / frequency;
	int index = 0;
	for(num_t t=0.0;t<totalTime;t+=integrationTime)
	{
		++index;
		double earthLattitudeApprox = t*(M_PI/12.0/60.0/60.0);
		num_t u, v, rsub1, isub1, rsub2, isub2, rsub3, isub3, rsub4, isub4, r, i;
		_model.GetUVPosition(u, v, earthLattitudeApprox, delayDirectionDEC, delayDirectionRA, combdx, combdy, combdz, wavelength);

		// First product
		SimulateAntenna(t, delayDirectionDEC, delayDirectionRA, 0, 0, frequency, earthLattitudeApprox, rsub1, isub1, index);
		amplitudeSqrt(rsub1, isub1);
		r = rsub1;
		i = isub1;

		// Second product
		if(a2.id == a1.id)
		{
			rsub2 = rsub1;
			isub2 = isub1;
		} else {
			SimulateAntenna(t, delayDirectionDEC, delayDirectionRA, dx2, dy2, frequency, earthLattitudeApprox, rsub2, isub2, index);
			amplitudeSqrt(rsub2, isub2);
		}
		num_t rtmp = r;
		r = r*rsub2 - (i*-isub2);
		i = rtmp*-isub2 + i*rsub2;

		// Third product
		if(a3.id == a1.id)
		{
			rsub3 = rsub1;
			isub3 = isub1;
		} else if(a3.id == a2.id)
		{
			rsub3 = rsub2;
			isub3 = isub2;
		} else {
			SimulateAntenna(t, delayDirectionDEC, delayDirectionRA, dx3, dy3, frequency, earthLattitudeApprox, rsub3, isub3, index);
			amplitudeSqrt(rsub3, isub3);
		}
		rtmp = r;
		r = r*rsub3 - (i*isub3);
		i = rtmp*isub3 + i*rsub3;

		// Fourth product
		if(a4.id == a1.id)
		{
			rsub4 = rsub1;
			isub4 = isub1;
		} else if(a4.id == a2.id)
		{
			rsub4 = rsub2;
			isub4 = isub2;
		} else if(a4.id == a3.id)
		{
			rsub4 = rsub3;
			isub4 = isub3;
		} else { 
 			SimulateAntenna(t, delayDirectionDEC, delayDirectionRA, dx4, dy4, frequency, earthLattitudeApprox, rsub4, isub4, index);
			amplitudeSqrt(rsub4, isub4);
		}
		rtmp = r;
		r = r*rsub4 - (i*-isub4);
		i = rtmp*-isub4 + i*rsub4;

		_imager.SetUVValue(u, v, r, i, 1.0);
		_imager.SetUVValue(-u, -v, r, -i, 1.0);
	}
}

void FourProductCorrelatorTester::SimulateTwoProdObservation(num_t delayDirectionDEC, num_t delayDirectionRA, num_t frequency)
{
	size_t frequencySteps = 1;

	for(size_t f=0;f<frequencySteps;++f)
	{
		double channelFrequency = frequency + _observatorium.ChannelWidthHz() * f * 256 / frequencySteps;
		for(size_t i=0;i<_observatorium.AntennaCount();++i)
		{
			for(size_t j=i+1;j<_observatorium.AntennaCount();++j)
			{
				if(!(i == j))
				{
					const AntennaInfo
						&a1 = _observatorium.GetAntenna(i),
						&a2 = _observatorium.GetAntenna(j);

					SimulateTwoProdCorrelation(delayDirectionDEC, delayDirectionRA, a1, a2, channelFrequency, 12*60*60, 10.0);
				}
			}
			std::cout << "." << std::flush;
		}
	}
	std::cout << std::endl;
}

void FourProductCorrelatorTester::SimulateTwoProdCorrelation(num_t delayDirectionDEC, num_t delayDirectionRA, const AntennaInfo &a1, const AntennaInfo &a2, num_t frequency, double totalTime, double integrationTime)
{
	num_t
		x = a1.position.x,
		y = a1.position.y,
		z = a1.position.z,
		dx2 = x - a2.position.x,
		dy2 = y - a2.position.y,
		dz2 = z - a2.position.z;

	num_t wavelength = 1.0L / frequency;
	size_t index = 0;
	for(num_t t=0.0;t<totalTime;t+=integrationTime/4)
	{
		double earthLattitudeApprox = t*(M_PI/12.0/60.0/60.0);
		num_t u, v, rsub1, isub1, rsub2, isub2, r, i;
		_model.GetUVPosition(u, v, earthLattitudeApprox, delayDirectionDEC, delayDirectionRA, dx2*2.0, dy2*2.0, dz2*2.0, wavelength);

		// First product
		SimulateAntenna(t, delayDirectionDEC, delayDirectionRA, 0, 0, frequency, earthLattitudeApprox, rsub1, isub1, index);
		phaseMul2(rsub1, isub1);
		r = rsub1;
		i = isub1;

		// Second product
		if(a2.id == a1.id)
		{
			rsub2 = rsub1;
			isub2 = isub1;
		} else {
			SimulateAntenna(t, delayDirectionDEC, delayDirectionRA, dx2, dy2, frequency, earthLattitudeApprox, rsub2, isub2, index);
			phaseMul2(rsub2, isub2);
		}
		num_t rtmp = r;
		r = r*rsub2 - (i*-isub2);
		i = rtmp*-isub2 + i*rsub2;

		_imager.SetUVValue(u, v, r, i, 1.0);
		_imager.SetUVValue(-u, -v, r, -i, 1.0);

		++index;
	}
}
