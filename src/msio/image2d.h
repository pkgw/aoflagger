/***************************************************************************
 *   Copyright (C) 2007 by Andre Offringa   *
 *   offringa@gmail.com   *
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
/** @file
 * This is the header file for the Image2D class.
 * @author André Offringa <offringa@gmail.com>
 */
#ifndef IMAGE2D_H
#define IMAGE2D_H

#include "../baseexception.h"
#include "colormap.h"
#include "types.h"

#include <boost/shared_ptr.hpp>

#include <exception>

typedef boost::shared_ptr<class Image2D> Image2DPtr;
typedef boost::shared_ptr<const class Image2D> Image2DCPtr;

/**
 * This class represents a two dimensional single-valued (=gray scale) image. It can be
 * read from and written to a @c .fits file and written to a @c .png file. A new Image2D can
 * be constructed with e.g. the CreateFromFits(), CreateEmptyImage() or CreateFromDiff() static methods.
 */
class Image2D {
	public:
		
		/**
		 * Creates an image containing unset values.
		 * @param width Width of the new image.
		 * @param height Height of the new image.
		 * @return The new image. Should be deleted by the caller.
		 */
		static Image2D *CreateUnsetImage(size_t width, size_t height)
		{
			return new Image2D(width, height);
		}
		
		/**
		 * As CreateUnsetImage(), but returns a smart pointer instead.
		 * @param width Width of the new image.
		 * @param height Height of the new image.
		 * @return A (unique) smart pointer to the new image.
		 */
		static Image2DPtr CreateUnsetImagePtr(size_t width, size_t height)
		{
			return Image2DPtr(CreateUnsetImage(width, height));
		}
		
		/**
		 * Creates an image containing zeros.
		 * @param width Width of the new image.
		 * @param height Height of the new image.
		 * @return The new created image. Should be deleted by the caller.
		 */
		static Image2D *CreateZeroImage(size_t width, size_t height);
		
		/**
		 * As CreateZeroImage(), but returns a smart pointer instead.
		 * @param width Width of the new image.
		 * @param height Height of the new image.
		 * @return The (unique) smart pointer to the new image.
		 */
		static Image2DPtr CreateZeroImagePtr(size_t width, size_t height)
		{
			return Image2DPtr(CreateZeroImage(width, height));
		}

		/**
		 * Destructor.
		 */
		~Image2D();
		
		/**
		 * Creates a new image by subtracting two images of the same size.
		 * @param imageA first image.
		 * @param imageB second image.
		 * @return The new created image. Should be deleted by the caller.
		 * @throws FitsIOException if the images do not match in size.
		 */
		static Image2D *CreateFromSum(const Image2D &imageA, const Image2D &imageB);
		static Image2DPtr CreateFromSum(const Image2DCPtr &imageA, const Image2DCPtr &imageB)
		{
			return Image2DPtr(CreateFromSum(*imageA, *imageB));
		}
		/**
		 * Creates a new image by subtracting two images of the same size.
		 * @param imageA first image.
		 * @param imageB second image.
		 * @return The new created image. Should be deleted by the caller.
		 * @throws FitsIOException if the images do not match in size.
		 */
		static Image2D *CreateFromDiff(const Image2D &imageA, const Image2D &imageB);
		static Image2DPtr CreateFromDiff(const Image2DCPtr &imageA, const Image2DCPtr &imageB)
		{
			return Image2DPtr(CreateFromDiff(*imageA, *imageB));
		}

		static Image2D *CreateCopy(const Image2D &image);
		static Image2DPtr CreateCopy(const Image2DCPtr &image)
		{
			return Image2DPtr(CreateCopy(*image));
		}
		static Image2DPtr CreateCopyPtr(const Image2D &image)
		{
			return Image2DPtr(CreateCopy(image));
		}

		/**
		 * Retrieves the average value of the image.
		 * @return The average value.
		 */
		num_t GetAverage() const;
		
		/**
		 * Returns the maximum value in the image.
		 * @return The maximimum value.
		 */
		num_t GetMaximum() const;
		
		/**
		 * Returns the maximum value in the specified range.
		 * @return The maximimum value.
		 */
		num_t GetMaximum(size_t xOffset, size_t yOffset, size_t width, size_t height) const;

		/**
		 * Returns the minimum value in the image.
		 * @return The minimum value.
		 */
		num_t GetMinimum() const;
		
		/**
		 * Returns the minimum value in the specified range.
		 * @return The minimum value.
		 */
		num_t GetMinimum(size_t xOffset, size_t yOffset, size_t width, size_t height) const;

		/**
		 * Returns the maximum finite value in the image.
		 * @return The maximimum value.
		 */
		num_t GetMaximumFinite() const;
		
		/**
		 * Returns the minimum finite value in the image.
		 * @return The minimum value.
		 */
		num_t GetMinimumFinite() const;
		
		/**
		 * Retrieves the value at a specific position.
		 * @param x x-coordinate
		 * @param y y-coordinate
		 * @return The value.
		 */
		inline num_t Value(size_t x, size_t y) const { return _dataPtr[y][x]; }
		
		/**
		 * Get the width of the image.
		 * @return Width of the image.
		 */
		inline size_t Width() const { return _width; }
		
		/**
		 * Get the height of the image.
		 * @return Height of the image.
		 */
		inline size_t Height() const { return _height; }
		
		/**
		 * Change a value at a specific position.
		 * @param x x-coordinate of value to change.
		 * @param y y-coordinate of value to change.
		 * @param newValue New value.
		 */
		inline void SetValue(size_t x, size_t y, num_t newValue)
		{
			_dataPtr[y][x] = newValue;
		}

		void SetValues(const Image2D &source);
		void SetValues(const Image2DCPtr &source)
		{
			SetValues(*source);
		}

		void SetAll(num_t value);
		
		inline void AddValue(size_t x, size_t y, num_t addValue)
		{
			_dataPtr[y][x] += addValue;
		}
		
		/**
		 * Check whether this image is completely zero.
		 * @return @c true if the value only contains zeros.
		 */
		bool ContainsOnlyZeros() const;
		
		/**
		 * Compute the sum of all values
		 */
		num_t Sum() const
		{
			num_t sum = 0.0;
			for(size_t y=0;y<_height;++y)
			{
				for(size_t x=0;x<_width;++x)
					sum += Value(x, y);
			}
			return sum;
		}
		
		void SetToAbs()
		{
			for(size_t y=0;y<_height;++y)
			{
				for(size_t x=0;x<_width;++x)
					SetValue(x, y,  fabsn(Value(x, y)));
			}
		}
		
		/**
		 * Retrieve a factor to multiply the values with to normalise them.
		 * @return Normalisation factor.
		 */
		num_t GetMaxMinNormalizationFactor() const;

		num_t GetStdDev() const;

		num_t GetRMS() const
		{
			return GetRMS(0, 0, _width, _height);
		}

		num_t GetMode() const;
		
		num_t GetRMS(size_t xOffset, size_t yOffset, size_t width, size_t height) const;

		/**
		 * Normalize the data so that the variance is 1.
		 */
		void NormalizeVariance();

		/**
		* Create a new image instance by reading a fitsfile.
		* @param file The fits file.
		* @param imageNumber The number of the image.
		* @return The new created image. Should be deleted by the caller.
		* @throws FitsIOException if something goes wrong during reading the .fits file.
		*/
		static Image2D *CreateFromFits(class FitsFile &file, int imageNumber);

		/**
		* Number of images that can be read from the current HUD block
		* in the fits file.
		* @param file Fits file.
		* @return Number of images.
		*/
		static long GetImageCountInHUD(class FitsFile &file);

		/**
		* Save the image to a fits file.
		* @param filename Fits filename.
		* @throws IOException if something goes wrong during writing
		*/
		void SaveToFitsFile(const std::string &filename) const;

		/**
		 * Count the number of values that are above a specified value.
		 */
		size_t GetCountAbove(num_t value) const;
		size_t GetCountBelowOrEqual(num_t value) const
		{
			return _width*_height - GetCountAbove(value);
		}

		/**
		 * Returns a threshold for which @c count values are above the
		 * the threshold. That is, GetCountAbove(GetTresholdForCountAbove(x)) = x.
		 */
		num_t GetTresholdForCountAbove(size_t count) const;

		/**
		 * Copies all values to the specified array. The array should be of size width*height.
		 */
		void CopyData(num_t *destination) const;

		/**
		 * Multiply all values with a factor.
		 */
		void MultiplyValues(num_t factor);
		
		/**
		 * Will set all values to lhs - this.
		 */
		void SubtractAsRHS(const Image2DCPtr &lhs);

		/**
		 * Flips the image round the diagonal, i.e., x becomes y and y becomes x.
		 */
		Image2DPtr CreateXYFlipped() const
		{
			Image2D *image = new Image2D(_height, _width);
			for(unsigned y=0;y<_height;++y)
			{
				for(unsigned x=0;x<_width;++x)
					image->_dataPtr[x][y] = _dataPtr[y][x];
			}
			return Image2DPtr(image);
		}
		
		void SwapXY()
		{
			Image2DPtr swapped = CreateXYFlipped();
			Swap(swapped);
		}
		
		/**
		 * Swaps the contents of the two masks. This can be used as a move assignment operator, as it
		 * only swaps pointers; hence it is fast.
		 */
		void Swap(Image2D &source)
		{
			std::swap(source._width, _width);
			std::swap(source._stride, _stride);
			std::swap(source._height, _height);
			std::swap(source._dataPtr, _dataPtr);
			std::swap(source._dataConsecutive, _dataConsecutive);
		}
		
		/**
		 * Swaps the contents of the two masks. This can be used as a move assignment operator, as it
		 * only swaps pointers; hence it is fast.
		 */
		void Swap(const Image2DPtr &source)
		{
			Swap(*source);
		}
		
		/**
		 * Resample the image horizontally by decreasing the width
		 * with an integer factor.
		 */
		Image2DPtr ShrinkHorizontally(size_t factor) const;

		/**
		 * Resample the image vertically by decreasing the height
		 * with an integer factor.
		 */
		Image2DPtr ShrinkVertically(size_t factor) const;

		/**
		 * Resample the image horizontally by increasing the width
		 * with an integer factor.
		 */
		Image2DPtr EnlargeHorizontally(size_t factor, size_t newWidth) const;

		/**
		 * Resample the image vertically by increasing the width
		 * with an integer factor.
		 */
		Image2DPtr EnlargeVertically(size_t factor, size_t newHeight) const;

		Image2DPtr Trim(size_t startX, size_t startY, size_t endX, size_t endY) const;
		
		void SetTrim(size_t startX, size_t startY, size_t endX, size_t endY);
		
		void CopyFrom(const Image2DCPtr &source, size_t destX, size_t destY)
		{
			size_t
				x2 = source->_width + destX,
				y2 = source->_height + destY;
			if(x2 > _width) x2 = _width;
			if(y2 > _height) y2 = _height;
			for(size_t y=destY;y<y2;++y)
			{
				for(size_t x=destX;x<x2;++x)
					SetValue(x, y, source->Value(x-destX, y-destY));
			}
		}
		
		/**
		 * Returns a pointer to one row of data. This can be used to step
		 * quickly over the data in x direction. Note that the next row
		 * is not exactly at "one times width", because the number of
		 * samples in a row is made divisable by four. This makes it
		 * possible to execute SSE instruction easily.
		 * 
		 * If you want to skip over a whole row, use the Stride() method
		 * to determine the intrinsicly used width of one row.
		 * 
		 * @see Stride()
		 */
		inline num_t *ValuePtr(unsigned x, unsigned y)
		{
			return &_dataPtr[y][x];
		}
		
		/**
		 * Returns a constant pointer to one row of data. This can be used to
		 * step quickly over the data in x direction. Note that the next row
		 * is not exactly at "one times width", because the number of
		 * samples in a row is made divisable by four. This makes it
		 * possible to execute SSE instruction easily.
		 * 
		 * If you want to skip over a whole row, use the Stride() method
		 * to determine the intrinsicly used width of one row.
		 * 
		 * @see Stride()
		 */
		inline const num_t *ValuePtr(unsigned x, unsigned y) const
		{
			return &_dataPtr[y][x];
		}
		
		/**
		 * This value specifies the intrinsic width of one row. It is
		 * normally the first number that is >= Width() and divisable by
		 * four. When using the ValuePtr(unsigned, unsigned) method,
		 * this value can be used to step over one row.
		 * 
		 * @see ValuePtr(unsigned, unsigned)
		 */
		inline size_t Stride() const
		{
			return _stride;
		}
		
	private:
		Image2D(size_t width, size_t height);
		size_t _width, _height;
		size_t _stride;
		num_t **_dataPtr, *_dataConsecutive;
};

#endif
