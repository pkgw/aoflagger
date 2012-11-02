#ifndef FOURPRODUCTCORRELATORTESTER_H
#define FOURPRODUCTCORRELATORTESTER_H

#include "../msio/types.h"

#include <cstring>

class FourProductCorrelatorTester
{
	public:
		FourProductCorrelatorTester(class Model &model, class UVImager &imager, class Observatorium &observatorium);
	
		void SimulateObservation(num_t delayDirectionDEC, num_t delayDirectionRA, num_t frequency);
	
		void SimulateCorrelation(num_t delayDirectionDEC, num_t delayDirectionRA, const class AntennaInfo &a1, const class AntennaInfo &a2, const class AntennaInfo &a3, const class AntennaInfo &a4, num_t frequency, double totalTime, double integrationTime);

		void SimulateTwoProdObservation(num_t delayDirectionDEC, num_t delayDirectionRA, num_t frequency);
	
		void SimulateTwoProdCorrelation(num_t delayDirectionDEC, num_t delayDirectionRA, const class AntennaInfo &a1, const class AntennaInfo &a2, num_t frequency, double totalTime, double integrationTime);
	private:
		void SimulateAntenna(double time, num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t frequency, num_t earthLattitude, num_t &r, num_t &i, size_t index);

		void complexSqrt(num_t &r, num_t &i);
		void phaseMul2(num_t &r, num_t &i);
		void amplitudeSqrt(num_t &r, num_t &i);
		class Model &_model;
		class UVImager &_imager;
		class Observatorium &_observatorium;
		bool _incoherent;
};

#endif
