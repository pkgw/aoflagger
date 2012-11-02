#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#include "../../msio/image2d.h"

#include "thresholdmitigater.h"

#include "../../util/aologger.h"

/**
 * The SSE version of the Vertical SumThreshold algorithm using intrinsics.
 *
 * The SumThreshold algorithm is applied on 4 time steps at a time. Since the SSE has
 * instructions that operate on 4 floats at a time, this could in theory speed up the
 * processing with a factor of 4. However, a lot of time is lost shuffling the data in the
 * right order/registers, and since a timestep consists of 1 byte booleans and 4 byte
 * floats, there's a penalty. Finally, since the 4 timesteps have to be processed exactly
 * the same way, conditional branches had to be replaced by conditional moves.
 *
 * The average profit of SSE intrinsics vs no SSE seems to be about a factor of 1.5 to 2-3,
 * depending on the Length parameter, but also depending on the number of flags (With
 * Length=256, it can make a factor of 3 difference). It might also vary on different
 * processors; e.g. on my Desktop Xeon with older gcc, the profit was less pronounced,
 * while my Intel i5 at home showed an avg factor of over 2 difference.
 *
 * The algorithm works with Length=1, but since that is a normal thresholding operation,
 * computing a sumthreshold has a lot of overhead, hence is not optimal at that size.
 */
template<size_t Length>
void ThresholdMitigater::VerticalSumThresholdLargeSSE(Image2DCPtr input, Mask2DPtr mask, num_t threshold)
{
	Mask2D *maskCopy = Mask2D::CreateCopy(*mask);
	const size_t width = mask->Width(), height = mask->Height();
	const __m128 zero4 = _mm_set_ps(0.0, 0.0, 0.0, 0.0);
	const __m128i zero4i = _mm_set_epi32(0, 0, 0, 0);
	const __m128i ones4 = _mm_set_epi32(1, 1, 1, 1);
	const __m128 threshold4Pos = _mm_set1_ps(threshold);
	const __m128 threshold4Neg = _mm_set1_ps(-threshold); 
	if(Length <= height)
	{
		for(size_t x=0;x<width;x += 4)
		{
			__m128 sum4 = _mm_set_ps(0.0, 0.0, 0.0, 0.0);
			__m128i count4 = _mm_set_epi32(0, 0, 0, 0);
			size_t yBottom;
			
			for(yBottom=0;yBottom<Length-1;++yBottom)
			{
				const bool *rowPtr = mask->ValuePtr(x, yBottom);
				
				// Assign each integer to one bool in the mask
				// Convert true to 0xFFFFFFFF and false to 0
				__m128 conditionMask = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(rowPtr[3], rowPtr[2], rowPtr[1], rowPtr[0]),
													zero4i));
				
				// Conditionally increment counters
				count4 = _mm_add_epi32(count4, _mm_and_si128(_mm_castps_si128(conditionMask), ones4));
				
				// Add values with conditional move
				__m128 m = _mm_and_ps(_mm_load_ps(input->ValuePtr(x, yBottom)), conditionMask);
				sum4 = _mm_add_ps(sum4, _mm_or_ps(m, _mm_andnot_ps(conditionMask, zero4)));
			}
			
			size_t yTop = 0;
			while(yBottom < height)
			{
				// ** Add the 4 sample at the bottom **
				
				// get a ptr
				const bool *rowPtr = mask->ValuePtr(x, yBottom);
				
				// Assign each integer to one bool in the mask
				// Convert true to 0xFFFFFFFF and false to 0
				__m128 conditionMask = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(rowPtr[3], rowPtr[2], rowPtr[1], rowPtr[0]),
													_mm_set_epi32(0, 0, 0, 0)));
				
				// Conditionally increment counters
				count4 = _mm_add_epi32(count4, _mm_and_si128(_mm_castps_si128(conditionMask), ones4));
				
				// Add values with conditional move
				sum4 = _mm_add_ps(sum4,
					_mm_or_ps(_mm_and_ps(_mm_load_ps(input->ValuePtr(x, yBottom)), conditionMask),
										_mm_andnot_ps(conditionMask, zero4)));
				
				// ** Check sum **
				
				// if sum/count > threshold || sum/count < -threshold
				__m128 avg4 = _mm_div_ps(sum4, _mm_cvtepi32_ps(count4));
				const unsigned flagConditions =
					_mm_movemask_ps(_mm_cmpgt_ps(avg4, threshold4Pos)) |
					_mm_movemask_ps(_mm_cmplt_ps(avg4, threshold4Neg));
				// | _mm_movemask_ps(_mm_cmplt_ps(count4, zero4i));
				
				// The assumption is that most of the values are actually not thresholded, hence, if
				// this is the case, we circumvent the whole loop at the cost of one extra comparison:
				if(flagConditions != 0)
				{
					union
					{
						bool theChars[4];
						unsigned theInt;
					} outputValues = { {
						(flagConditions&1)!=0,
						(flagConditions&2)!=0,
						(flagConditions&4)!=0,
						(flagConditions&8)!=0 } };

					for(size_t i=0;i<Length;++i)
					{
						unsigned *outputPtr = reinterpret_cast<unsigned*>(maskCopy->ValuePtr(x, yTop + i));
						
						*outputPtr |= outputValues.theInt;
					}
				}
				
				// ** Subtract the sample at the top **
				
				// get a ptr
				const bool *tRowPtr = mask->ValuePtr(x, yTop);
				
				// Assign each integer to one bool in the mask
				// Convert true to 0xFFFFFFFF and false to 0
				conditionMask = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(tRowPtr[3], tRowPtr[2], tRowPtr[1], tRowPtr[0]),
													zero4i));
				
				// Conditionally decrement counters
				count4 = _mm_sub_epi32(count4, _mm_and_si128(_mm_castps_si128(conditionMask), ones4));
				
				// Subtract values with conditional move
				sum4 = _mm_sub_ps(sum4,
					_mm_or_ps(_mm_and_ps(_mm_load_ps(input->ValuePtr(x, yTop)), conditionMask),
										_mm_andnot_ps(conditionMask, zero4)));
				
				// ** Next... **
				++yTop;
				++yBottom;
			}
		}
	}
	mask->Swap(*maskCopy);
	delete maskCopy;
}

template<size_t Length>
void ThresholdMitigater::HorizontalSumThresholdLargeSSE(Image2DCPtr input, Mask2DPtr mask, num_t threshold)
{
	// The idea of the horizontal SSE version is to read four ('y') rows and
	// process them simultaneously. 
	
	// Currently, this SSE horizontal version is not significant faster
	// (less than ~3%) than the
	// Non-SSE horizontal version. This has probably to do with 
	// rather randomly reading through the set (first (0,0)-(0,3), then (1,0)-(1,3), etc)
	// this introduces cache misses and/or many smaller reading requests
	
	
	Mask2D *maskCopy = Mask2D::CreateCopy(*mask);
	const size_t width = mask->Width(), height = mask->Height();
	const __m128 zero4 = _mm_set_ps(0.0, 0.0, 0.0, 0.0);
	const __m128i zero4i = _mm_set_epi32(0, 0, 0, 0);
	const __m128i ones4 = _mm_set_epi32(1, 1, 1, 1);
	const __m128 threshold4Pos = _mm_set1_ps(threshold);
	const __m128 threshold4Neg = _mm_set1_ps(-threshold); 
	if(Length <= width)
	{
		for(size_t y=0;y<height;y += 4)
		{
			__m128 sum4 = _mm_set_ps(0.0, 0.0, 0.0, 0.0);
			__m128i count4 = _mm_set_epi32(0, 0, 0, 0);
			size_t xRight;
			
			const bool
				*rFlagPtrA = mask->ValuePtr(0, y+3),
				*rFlagPtrB = mask->ValuePtr(0, y+2),
				*rFlagPtrC = mask->ValuePtr(0, y+1),
				*rFlagPtrD = mask->ValuePtr(0, y);
			const num_t
				*rValPtrA = input->ValuePtr(0, y+3),
				*rValPtrB = input->ValuePtr(0, y+2),
				*rValPtrC = input->ValuePtr(0, y+1),
				*rValPtrD = input->ValuePtr(0, y);
				
			for(xRight=0;xRight<Length-1;++xRight)
			{
				// Assign each integer to one bool in the mask
				// Convert true to 0xFFFFFFFF and false to 0
				__m128 conditionMask = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(*rFlagPtrA, *rFlagPtrB, *rFlagPtrC, *rFlagPtrD),
													zero4i));
				
				// Conditionally increment counters (nr unflagged samples)
				count4 = _mm_add_epi32(count4, _mm_and_si128(_mm_castps_si128(conditionMask), ones4));
				
				// Load 4 samples
				__m128 v = _mm_set_ps(*rValPtrA,
															*rValPtrB,
															*rValPtrC,
															*rValPtrD);
				
				// Add values with conditional move
				sum4 = _mm_add_ps(sum4, _mm_or_ps(_mm_and_ps(v, conditionMask),
																					_mm_andnot_ps(conditionMask, zero4)));
				
				++rFlagPtrA;
				++rFlagPtrB;
				++rFlagPtrC;
				++rFlagPtrD;

				++rValPtrA;
				++rValPtrB;
				++rValPtrC;
				++rValPtrD;
			}
			
			size_t xLeft = 0;
			const bool
				*lFlagPtrA = mask->ValuePtr(0, y+3),
				*lFlagPtrB = mask->ValuePtr(0, y+2),
				*lFlagPtrC = mask->ValuePtr(0, y+1),
				*lFlagPtrD = mask->ValuePtr(0, y);
			const num_t
				*lValPtrA = input->ValuePtr(0, y+3),
				*lValPtrB = input->ValuePtr(0, y+2),
				*lValPtrC = input->ValuePtr(0, y+1),
				*lValPtrD = input->ValuePtr(0, y);
				
			while(xRight < width)
			{
				// ** Add the sample at the right **
				
				// Assign each integer to one bool in the mask
				// Convert true to 0xFFFFFFFF and false to 0
				__m128 conditionMask = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(*rFlagPtrA, *rFlagPtrB, *rFlagPtrC, *rFlagPtrD),
													zero4i));
				
				// Conditionally increment counters
				count4 = _mm_add_epi32(count4, _mm_and_si128(_mm_castps_si128(conditionMask), ones4));
				
				// Load 4 samples
				__m128 v = _mm_set_ps(*rValPtrA,
															*rValPtrB,
															*rValPtrC,
															*rValPtrD);
				
				// Add values with conditional move (sum4 += (v & m) | (m & ~0) ).
				sum4 = _mm_add_ps(sum4, _mm_or_ps(_mm_and_ps(v, conditionMask), 
																					_mm_andnot_ps(conditionMask, zero4)));
				
				// ** Check sum **
				
				// if sum/count > threshold || sum/count < -threshold
				__m128 count4AsSingle = _mm_cvtepi32_ps(count4);
				const unsigned flagConditions =
					_mm_movemask_ps(_mm_cmpgt_ps(_mm_div_ps(sum4, count4AsSingle), threshold4Pos)) |
					_mm_movemask_ps(_mm_cmplt_ps(_mm_div_ps(sum4, count4AsSingle), threshold4Neg));
				
				/*if((flagConditions & 8) != 0)
				{
					float vf;
					_mm_store_ss(&vf, v);
					std::cout << vf << "=" << input->Value(xRight, y+3) << '|';
					float fsum;
					_mm_store_ss(&fsum, sum4);
					float f;
					_mm_store_ss(&f, _mm_div_ps(sum4, count4AsSingle));
					std::cout << xLeft << ":" << input->Value(xRight-1, y+3) << '+' << input->Value(xRight, y+3) << '=' << fsum << "/..=" << f << ',';
				}*/
					
				if((flagConditions & 1) != 0)
					maskCopy->SetHorizontalValues(xLeft, y, true, Length);
				if((flagConditions & 2) != 0)
					maskCopy->SetHorizontalValues(xLeft, y+1, true, Length);
				if((flagConditions & 4) != 0)
					maskCopy->SetHorizontalValues(xLeft, y+2, true, Length);
				if((flagConditions & 8) != 0)
					maskCopy->SetHorizontalValues(xLeft, y+3, true, Length);
				
				// ** Subtract the sample at the left **
				
				// Assign each integer to one bool in the mask
				// Convert true to 0xFFFFFFFF and false to 0
				conditionMask = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(*lFlagPtrA, *lFlagPtrB, *lFlagPtrC, *lFlagPtrD),
													zero4i));
				
				// Conditionally decrement counters
				count4 = _mm_sub_epi32(count4, _mm_and_si128(_mm_castps_si128(conditionMask), ones4));
				
				// Load 4 samples
				v = _mm_set_ps(*lValPtrA,
											 *lValPtrB,
											 *lValPtrC,
											 *lValPtrD);
				
				// Subtract values with conditional move
				sum4 = _mm_sub_ps(sum4,
					_mm_or_ps(_mm_and_ps(v, conditionMask), _mm_andnot_ps(conditionMask, zero4)));
				
				// ** Next... **
				++xLeft;
				++xRight;
				
				++rFlagPtrA;
				++rFlagPtrB;
				++rFlagPtrC;
				++rFlagPtrD;
				
				++lFlagPtrA;
				++lFlagPtrB;
				++lFlagPtrC;
				++lFlagPtrD;

				++rValPtrA;
				++rValPtrB;
				++rValPtrC;
				++rValPtrD;
				
				++lValPtrA;
				++lValPtrB;
				++lValPtrC;
				++lValPtrD;
			}
		}
	}
	mask->Swap(*maskCopy);
	delete maskCopy;
}

template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<1>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<2>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<4>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<8>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<16>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<32>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<64>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<128>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<256>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);


template
void ThresholdMitigater::HorizontalSumThresholdLargeSSE<1>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::HorizontalSumThresholdLargeSSE<2>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::HorizontalSumThresholdLargeSSE<4>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::HorizontalSumThresholdLargeSSE<8>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::HorizontalSumThresholdLargeSSE<16>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::HorizontalSumThresholdLargeSSE<32>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::HorizontalSumThresholdLargeSSE<64>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::HorizontalSumThresholdLargeSSE<128>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::HorizontalSumThresholdLargeSSE<256>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
