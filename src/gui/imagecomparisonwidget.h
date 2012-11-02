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
#ifndef IMAGECOMPARISONWIDGET_H
#define IMAGECOMPARISONWIDGET_H

#include <gtkmm/drawingarea.h>

#include <vector>

#include "../msio/image2d.h"
#include "../msio/timefrequencydata.h"
#include "../msio/timefrequencymetadata.h"
#include "../msio/segmentedimage.h"

#include "imagewidget.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ImageComparisonWidget : public ImageWidget {
	public:
		enum TFImage { TFOriginalImage, TFRevisedImage, TFContaminatedImage };
		ImageComparisonWidget();
		~ImageComparisonWidget();
		void SetNewData(const class TimeFrequencyData &image, TimeFrequencyMetaDataCPtr metaData);

		TimeFrequencyData GetActiveData() const
		{
			TimeFrequencyData data(getActiveDataWithOriginalFlags());
			data.SetNoMask();
			if(ShowOriginalMask())
			{
			  if(ShowAlternativeMask())
				{
					data.SetMask(_original);
					data.JoinMask(_contaminated);
				} else
					data.SetMask(_original);
			} else {
			      if(ShowAlternativeMask())
					data.SetMask(_contaminated);
			}
			if(StartHorizontal() != 0.0 || EndHorizontal() != 1.0 || StartVertical() != 0.0 || EndVertical() != 1.0)
			   data.Trim(round(StartHorizontal() * data.ImageWidth()), round(StartVertical() * data.ImageHeight()),
									 round(EndHorizontal() * data.ImageWidth()), round(EndVertical() * data.ImageHeight())); 
			return data;
		}

		TimeFrequencyData &OriginalData() { return _original; }
		const TimeFrequencyData &OriginalData() const { return _original; }

		TimeFrequencyData &RevisedData() { return _revised; }
		const TimeFrequencyData &RevisedData() const { return _revised; }

		void SetRevisedData(const TimeFrequencyData &data)
		{
			_revised = data;
		  updateVisualizedImage();
		}
		const TimeFrequencyData &ContaminatedData() const { return _contaminated; }
		TimeFrequencyData &ContaminatedData() { return _contaminated; }
		void SetContaminatedData(const TimeFrequencyData &data)
		{
			_contaminated = data;
			SetAlternativeMask(data.GetSingleMask());
		  updateVisualizedImage();
		} 
		void SetVisualizedImage(TFImage visualizedImage)
		{
		  _visualizedImage = visualizedImage;
		  updateVisualizedImage();
		}
		void ClearBackground();
	private:
		void updateVisualizedImage();
		const TimeFrequencyData getActiveDataWithOriginalFlags() const
		{
			switch(_visualizedImage)
			{
				case TFOriginalImage:
				default:
					return _original;
				case TFRevisedImage:
					return _revised;
				case TFContaminatedImage:
					return _contaminated;
			}
		}
		enum TFImage _visualizedImage;
		TimeFrequencyData _original, _revised, _contaminated;
		TimeFrequencyMetaDataCPtr _metaData;
};

#endif
