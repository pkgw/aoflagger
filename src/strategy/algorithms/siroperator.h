
#ifndef SIROPERATOR_H
#define SIROPERATOR_H

#include "../../msio/mask2d.h"
#include "../../msio/types.h"
#include "../../msio/xyswappedmask2d.h"

/**
 * This class contains functions that implement an algorithm to dilate
 * a flag mask: the "scale-invariant rank (SIR) operator".
 * The amount of dilation is relative to the size of the flagged
 * areas in the input, hence it is scale invariant. This behaviour is very
 * effective for application after amplitude based RFI detection and is a step
 * in the default LOFAR flagging pipeline.
 * 
 * The rule for this scale invariant dilation is as follows:
 * Consider the sequence w(y) of size N, where w(y) = 0 if sample y is flagged and w(y) = 1
 * otherwise. If there exists a subsequence within w that
 * includes y and that has a flagged ratio of η or more, y will be flagged.
 * 
 * Thus:
 * if Y1 and Y2 exists, such that \\sum_{y=Y1}^{Y2-1} w(y)  <  η (Y2 - Y1), flag y.
 * 
 * The algorithm will be applied both in time and in frequency direction, thus w(y) can
 * contain a slice through the time-frequency image in either directions.
 * 
 * If you want to read more about the algorithm, read Offringa et al. 2010 (PoS RFI2010),
 * or the Dilate() function, which is the proof of concept, reference
 * O(N) algorithm and includes some comments within the algorithm.
 * 
 * Thanks to Jasper van de Gronde for the idea of an O(N) algorithm.
 * 
 * @author A.R. Offringa
 */
class SIROperator
{
	public:
		/**
		 * This is the proof of concept, reference version of the O(N) algorithm. It is
		 * fast, but OperateHorizontally() and OperateVertically() have been optimized
		 * for operating on a mask directly, which is the common mode of operation.
		 * 
		 * It contains extra comments to explain the algorithm within the code.
		 * 
		 * @param [in,out] flags The input array of flags to be dilated that will be overwritten
		 * by the dilatation of itself.
		 * @param [in] flagsSize Size of the @c flags array.
		 * @param [in] eta The η parameter that specifies the minimum number of good data
		 * that any subsequence should have (see class description for the definition).
		 */
		static void Operate(bool *flags, const unsigned flagsSize, num_t eta)
		{
			// The test for a sample to become flagged can be rewritten as
			//         \\sum_{y=Y1}^{Y2-1} ( η - w(y) ) >= 0.
			// With w(y) =      flags[y] : 0
			//                 !flags[y] : 1
			
			// Make an array in which flagged samples are η and unflagged samples are η-1,
			// such that we can test for \\sum_{y=Y1}^{Y2-1} values[y] >= 0
			num_t *values = new num_t[flagsSize];
			for(unsigned i=0 ; i<flagsSize ; ++i)
			{
				if(flags[i])
					values[i] = eta;
				else
					values[i] = eta - 1.0;
			}
			
			// For each x, we will now search for the largest sum of sequantial values that contains x.
			// If this sum is larger then 0, this value is part of a sequence that exceeds the test.
			
			// Define W(x) = \\sum_{y=0}^{x-1} values[y], such that the largest sequence containing x
			// starts at the element after W(y) is minimal in the range 0 <= y <= x, and ends when
			// W(y) is maximal in the range x < y < N.
			
			// Calculate these W's and minimum prefixes
			const unsigned wSize = flagsSize+1;
			num_t *w = new num_t[wSize];
			w[0] = 0.0;
			unsigned currentMinIndex = 0;
			unsigned *minIndices = new unsigned[wSize];
			minIndices[0] = 0;
			for(unsigned i=1 ; i!=wSize ; ++i)
			{
				w[i] = w[i-1] + values[i-1];

				if(w[i] < w[currentMinIndex])
				{
					currentMinIndex = i;
				}
				minIndices[i] = currentMinIndex;
			}
			
			// Calculate the maximum suffixes
			unsigned currentMaxIndex = wSize-1;
			unsigned *maxIndices = new unsigned[wSize];
			for(unsigned i=flagsSize-1 ; i!=0 ; --i)
			{
				// We directly assign maxIndices[i] to the max index over
				// all indices *higher* than i, since maxIndices[i] is
				// not allowed to be i (maxIndices[i] = max i: x < i < N).
				maxIndices[i] = currentMaxIndex;
				
				if(w[i] > w[currentMaxIndex])
				{
					currentMaxIndex = i;
				}
			}
			maxIndices[0] = currentMaxIndex;
			
			// See if max sequence exceeds limit.
			for(unsigned i=0 ; i!=flagsSize ; ++i )
			{
				const num_t maxW = w[maxIndices[i]] - w[minIndices[i]];
				flags[i] = (maxW >= 0.0);
			}
			
			// Free our temporaries
			delete[] maxIndices;
			delete[] minIndices;
			delete[] w;
			delete[] values;
		}
		
		/**
		 * Performs a horizontal dilation directly on a mask. Algorithm is equal to Dilate().
		 * This is the implementation.
		 * 
		 * @param [in,out] mask The input flag mask to be dilated.
		 * @param [in] eta The η parameter that specifies the minimum number of good data
		 * that any subsequence should have.
		 */
		static void OperateHorizontally(Mask2DPtr &mask, num_t eta)
		{
			operateHorizontally<Mask2D>(*mask, eta);
		}
		
		/**
		 * Performs a vertical dilation directly on a mask. Algorithm is equal to Dilate().
		 * 
		 * @param [in,out] mask The input flag mask to be dilated.
		 * @param [in] eta The η parameter that specifies the minimum number of good data
		 * that any subsequence should have.
		 */
		static void OperateVertically(Mask2DPtr mask, num_t eta)
		{
			XYSwappedMask2D swappedMask(*mask);
			operateHorizontally<XYSwappedMask2D>(swappedMask, eta);
		}
		
		/**
		 * This is an experimental algorithm that might be slightly faster than
		 * the original algorithm. Jasper van de Gronde is preparing an article about it.
		 * @param [in,out] flags The input array of flags to be dilated that will be overwritten
		 * by the dilatation of itself.
		 * @param [in] flagsSize Size of the @c flags array.
		 * @param [in] eta The η parameter that specifies the minimum number of good data
		 * that any subsequence should have (see class description for the definition).
		 */
		static void Operate2PassAlgorithm(bool *flags, const size_t flagsSize, num_t eta)
		{
			bool temp[flagsSize];
			num_t credit = 0.0;
			for(size_t i=0; i<flagsSize; ++i)
			{
				// credit ← max(0, credit) + w(f [i])
				const num_t w = flags[i] ? eta : eta-1.0;
				const num_t maxcredit0 = credit > 0.0 ? credit : 0.0;
				credit = maxcredit0 + w;
				temp[i] = (credit >= 0.0);
			}
			
			// The same iteration, but now backwards
			credit = 0.0;
			size_t i = flagsSize;
			while(i > 0)
			{
				--i;
				const num_t w = flags[i] ? eta : eta-1.0;
				const num_t maxcredit0 = credit > 0.0 ? credit : 0.0;
				credit = maxcredit0 + w;
				flags[i] = (credit >= 0.0) || temp[i];
			}
		}
	private:
		SIROperator() { }

		/**
		 * Performs a horizontal dilation directly on a mask. Algorithm is equal to Dilate().
		 * This is the implementation.
		 * 
		 * @param [in,out] mask The input flag mask to be dilated.
		 * @param [in] eta The η parameter that specifies the minimum number of good data
		 * that any subsequence should have.
		 */
		template<typename MaskLike>
		static void operateHorizontally(MaskLike &mask, num_t eta)
		{
			const unsigned
				width = mask.Width(),
				wSize = width+1;
			num_t
				*values = new num_t[width],
				*w = new num_t[wSize];
			unsigned
				*minIndices = new unsigned[wSize],
				*maxIndices = new unsigned[wSize];
			
			for(unsigned row=0;row<mask.Height();++row)
			{
				for(unsigned i=0 ; i<width ; ++i)
				{
					if(mask.Value(i, row))
						values[i] = eta;
					else
						values[i] = eta - 1.0;
				}
				
				w[0] = 0.0;
				unsigned currentMinIndex = 0;
				minIndices[0] = 0;
				for(unsigned i=1 ; i!=wSize ; ++i)
				{
					w[i] = w[i-1] + values[i-1];

					if(w[i] < w[currentMinIndex])
					{
						currentMinIndex = i;
					}
					minIndices[i] = currentMinIndex;
				}
				
				// Calculate the maximum suffixes
				unsigned currentMaxIndex = wSize-1;
				for(unsigned i=width-1 ; i!=0 ; --i)
				{
					maxIndices[i] = currentMaxIndex;
					if(w[i] > w[currentMaxIndex])
					{
						currentMaxIndex = i;
					}
				}
				maxIndices[0] = currentMaxIndex;
				
				// See if max sequence exceeds limit.
				for(unsigned i=0 ; i!=width ; ++i )
				{
					const num_t maxW = w[maxIndices[i]] - w[minIndices[i]];
					mask.SetValue(i, row, (maxW >= 0.0));
				}
			}
			// Free our temporaries
			delete[] maxIndices;
			delete[] minIndices;
			delete[] w;
			delete[] values;
		}
};

#endif

