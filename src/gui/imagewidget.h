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
#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <gtkmm/drawingarea.h>

#include <cairomm/surface.h>

#include <vector>

#include "../msio/image2d.h"
#include "../msio/timefrequencydata.h"
#include "../msio/timefrequencymetadata.h"
#include "../msio/segmentedimage.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ImageWidget : public Gtk::DrawingArea {
	public:
		enum TFMap { BWMap, InvertedMap, HotColdMap, RedBlueMap, RedYellowBlueMap, BlackRedMap };
		enum Range { MinMax, Winsorized, Specified };
		enum ScaleOption { NormalScale, LogScale, ZeroSymmetricScale };
		
		ImageWidget();
		~ImageWidget();

		bool ShowOriginalMask() const { return _showOriginalMask; }
		void SetShowOriginalMask(bool newValue) { _showOriginalMask = newValue; }

		bool ShowAlternativeMask() const { return _showAlternativeMask; }
		void SetShowAlternativeMask(bool newValue) { _showAlternativeMask = newValue; }

		TFMap GetColorMap() const { return _colorMap; }
		void SetColorMap(TFMap colorMap) { _colorMap = colorMap; }
		
		void SetRange(enum Range range)
		{
			_range = range;
		}
		enum Range Range() const
		{
			return _range;
		}
		void SetScaleOption(ScaleOption option)
		{
			_scaleOption = option;
		}
		enum ScaleOption ScaleOption() const { return _scaleOption; }
		
		void Update(); 

		Image2DCPtr Image() const { return _image; }
		void SetImage(Image2DCPtr image) { _image = image; }

		Mask2DCPtr OriginalMask() const { return _originalMask; }
		void SetOriginalMask(Mask2DCPtr mask) { _originalMask = mask; }

		Mask2DCPtr AlternativeMask() const { return _alternativeMask; }
		void SetAlternativeMask(Mask2DCPtr mask) { _alternativeMask = mask; }

		Mask2DCPtr GetActiveMask() const;

		void SetHighlighting(bool newValue) { _highlighting = newValue; }
		class ThresholdConfig &HighlightConfig() { return *_highlightConfig; }
		bool HasImage() const { return _image != 0; }
		void SetHorizontalDomain(double start, double end)
		{
			_startHorizontal = start;
			_endHorizontal = end;
		}
		void SetVerticalDomain(double start, double end)
		{
			_startVertical = start;
			_endVertical = end;
		}
		void ResetDomains();
		double StartHorizontal() const { return _startHorizontal; }
		double EndHorizontal() const { return _endHorizontal; }
		double StartVertical() const { return _startVertical; }
		double EndVertical() const { return _endVertical; }
		void SetSegmentedImage(SegmentedImageCPtr segmentedImage) { _segmentedImage = segmentedImage; }
		TimeFrequencyMetaDataCPtr GetMetaData();
		void SetMetaData(TimeFrequencyMetaDataCPtr metaData) { _metaData = metaData; }

		sigc::signal<void, size_t, size_t> &OnMouseMovedEvent() { return _onMouseMoved; }
		sigc::signal<void> &OnMouseLeaveEvent() { return _onMouseLeft; }
		sigc::signal<void, size_t, size_t> &OnButtonReleasedEvent() { return _onButtonReleased; }
		
		num_t Max() const { return _max; }
		num_t Min() const { return _min; }
		
		void SetMax(num_t max) { _max = max; }
		void SetMin(num_t min) { _min = min; }
		
		void SavePdf(const std::string &filename);
		void SaveSvg(const std::string &filename);
		void SavePng(const std::string &filename);
		
		bool ShowXYAxes() const { return _showXYAxes; }
		void SetShowXYAxes(bool showXYAxes)
		{
			_showXYAxes = showXYAxes;
		}
		
		bool ShowColorScale() const { return _showColorScale; }
		void SetShowColorScale(bool showColorScale)
		{
			_showColorScale = showColorScale;
		}
		
		bool ShowXAxisDescription() const { return _showXAxisDescription; }
		void SetShowXAxisDescription(bool showXAxisDescription)
		{
			_showXAxisDescription = showXAxisDescription;
		}
		
		bool ShowYAxisDescription() const { return _showYAxisDescription; }
		void SetShowYAxisDescription(bool showYAxisDescription)
		{
			_showYAxisDescription = showYAxisDescription;
		}
		
		bool ShowZAxisDescription() const { return _showZAxisDescription; }
		void SetShowZAxisDescription(bool showZAxisDescription)
		{
			_showZAxisDescription = showZAxisDescription;
		}
		
		void Clear();
		
		void SetCairoFilter(Cairo::Filter filter)
		{
			_cairoFilter = filter;
		}
		
		void SetXAxisDescription(const std::string &description)
		{
			_xAxisDescription = description;
		}
		void SetYAxisDescription(const std::string &description)
		{
			_yAxisDescription = description;
		}
		void SetZAxisDescription(const std::string &description)
		{
			_zAxisDescription = description;
		}
		
		bool ManualXAxisDescription() const { return _manualXAxisDescription; }
		void SetManualXAxisDescription(bool manualDesc)
		{
			_manualXAxisDescription = manualDesc;
		}
		bool ManualYAxisDescription() const { return _manualYAxisDescription; }
		void SetManualYAxisDescription(bool manualDesc)
		{
			_manualYAxisDescription = manualDesc;
		}
		bool ManualZAxisDescription() const { return _manualZAxisDescription; }
		void SetManualZAxisDescription(bool manualDesc)
		{
			_manualZAxisDescription = manualDesc;
		}

	private:
		void findMinMax(Image2DCPtr image, Mask2DCPtr mask, num_t &min, num_t &max);
		void update(Cairo::RefPtr<Cairo::Context> cairo, unsigned width, unsigned height);
		void redrawWithoutChanges(Cairo::RefPtr<Cairo::Context> cairo, unsigned width, unsigned height);
		void downsampleImageBuffer(unsigned newWidth, unsigned newHeight);
		bool toUnits(double mouseX, double mouseY, int &posX, int &posY);
		bool onExposeEvent(GdkEventExpose* ev);
		bool onMotion(GdkEventMotion *event);
		bool onLeave(GdkEventCrossing *event);
		bool onButtonReleased(GdkEventButton *event);
		class ColorMap *createColorMap();

		bool _isInitialized;
		unsigned _initializedWidth, _initializedHeight;
		Cairo::RefPtr<Cairo::ImageSurface> _imageSurface;

		bool _showOriginalMask, _showAlternativeMask;
		enum TFMap _colorMap;
		TimeFrequencyMetaDataCPtr _metaData;
		Image2DCPtr _image;
		Mask2DCPtr _originalMask, _alternativeMask;
		bool _highlighting;
		class ThresholdConfig *_highlightConfig;
		double _leftBorderSize, _rightBorderSize, _topBorderSize, _bottomBorderSize;

		double _startHorizontal, _endHorizontal;
		double _startVertical, _endVertical;
		SegmentedImageCPtr _segmentedImage;
		class HorizontalPlotScale *_horiScale;
		class VerticalPlotScale *_vertScale;
		class ColorScale *_colorScale;
		enum ScaleOption _scaleOption;
		bool _showXYAxes;
		bool _showColorScale;
		bool _showXAxisDescription;
		bool _showYAxisDescription;
		bool _showZAxisDescription;
		num_t _max, _min;
		enum Range _range;
		Cairo::Filter _cairoFilter;
		std::string _xAxisDescription, _yAxisDescription, _zAxisDescription;
		bool _manualXAxisDescription;
		bool _manualYAxisDescription;
		bool _manualZAxisDescription;
		bool _mouseIsIn;

		sigc::signal<void, size_t, size_t> _onMouseMoved;
		sigc::signal<void> _onMouseLeft;
		sigc::signal<void, size_t, size_t> _onButtonReleased;
};

#endif
