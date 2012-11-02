#ifndef XYSWAPPEDMASK2D_H
#define XYSWAPPEDMASK2D_H

#include <boost/shared_ptr.hpp>

#include "mask2d.h"

/**
 * This class wraps a mask and swappes the x and y axes. It provides only the
 * trivial @ref Mask2D functions, and swappes x and y in there, such that the original
 * width becomes the new height, etc. It is useful to convert an algorithm
 * that works originally only in one direction to work in the other direction
 * without rewriting it. If a template parameter is used, the overhead should
 * be negligable.
 * 
 * Note that this method uses references to the original mask. However, masks
 * are normally wrapped in a smart pointer. The caller should make sure the
 * mask exists as long as the XYSwappedMask2D exists.
 * 
 * @author Andre Offringa
 */
class XYSwappedMask2D
{
	public:
		inline XYSwappedMask2D(Mask2D &mask) : _mask(mask)
		{
		}
		
		inline bool Value(unsigned x, unsigned y) const
		{
			return _mask.Value(y, x);
		}
		
		inline void SetValue(unsigned x, unsigned y, bool newValue)
		{
			_mask.SetValue(y, x, newValue);
		}
		
		inline unsigned Width() const
		{
			return _mask.Height();
		}
		
		inline unsigned Height() const
		{
			return _mask.Width();
		}
		
	private:
		Mask2D &_mask;
};

#endif // XYSWAPPEDMASK2D_H
