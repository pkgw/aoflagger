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
#include "imageplanewindow.h"

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/stock.h>

#include "../msio/fitsfile.h"

#include "../strategy/algorithms/sinusfitter.h"

#include "../util/ffttools.h"
#include "../util/plot.h"
#include "../util/ffttools.h"

#include "imagepropertieswindow.h"

ImagePlaneWindow::ImagePlaneWindow()
  : _imager(512, 512), /*3x1024 */ _clearButton("Clear"),
	_applyWeightsButton("Apply weights"),
	_refreshCurrentButton("R"),
	_memoryStoreButton("MS"),
	_memoryRecallButton("MR"),
	_memoryMultiplyButton("Mx"),
	_memorySubtractButton("M-"),
	_sqrtButton("sqrt"),
	_plotHorizontalButton("H"), _plotVerticalButton("V"),
	_angularTransformButton("AT"),
	_saveFitsButton("F"),
	_propertiesButton("P"),
	_uvPlaneButton("UV"), _imagePlaneButton("Image"),
	_zoomMenuButton("zoom"),
	_zoomXd4Button(_zoomGroup, "x1/4"), _zoomXd2Button(_zoomGroup, "x1/2"),
	_zoomX1Button(_zoomGroup, "x1"), _zoomX2Button(_zoomGroup, "x2"), _zoomX4Button(_zoomGroup, "x4"),
	_zoomX8Button(_zoomGroup, "x8"), _zoomX16Button(_zoomGroup, "x16"),
	_zoomX32Button(_zoomGroup, "x32"), _zoomX64Button(_zoomGroup, "x64"),
	_zoomX128Button(_zoomGroup, "x128"),
	_zoom(1.0L), _displayingUV(true),
	_propertiesWindow(0)
{
	set_default_size(400,400);

	// Add the plane radio buttons
	Gtk::RadioButtonGroup group;
	_topBox.pack_start(_uvPlaneButton, false, true);
	_uvPlaneButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onUVPlaneButtonClicked));
	_uvPlaneButton.set_group(group);
	_uvPlaneButton.set_active(true);
	_uvPlaneButton.set_tooltip_text("Switch to the UV plane");

	_topBox.pack_start(_imagePlaneButton, false, true);
	_imagePlaneButton.set_group(group);
	_imagePlaneButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onImagePlaneButtonClicked));
	_imagePlaneButton.set_tooltip_text("Switch to the image plane");

	// Add the clear button
	_topBox.pack_start(_clearButton, false, true);
	_clearButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onClearClicked));
	_clearButton.set_tooltip_text("Sets the current images to zero (both image and uv plane)");

	_topBox.pack_start(_applyWeightsButton, false, true);
	_applyWeightsButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onApplyWeightsClicked));
	_applyWeightsButton.set_tooltip_text("Divides each pixel by the number of times a sample was added to the pixel");

	// Add the zoom buttons
	_topBox.pack_start(_zoomMenuButton, false, true);
	_zoomMenuButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomMenuButtonClicked));
	
	_zoomMenu.append(_zoomXd4Button);
	_zoomXd4Button.signal_toggled().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));

	_zoomMenu.append(_zoomXd2Button);
	_zoomXd2Button.signal_toggled().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));

	_zoomMenu.append(_zoomX1Button);
	_zoomX1Button.set_active(true);
	_zoomX1Button.signal_toggled().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));

	_zoomMenu.append(_zoomX2Button);
	_zoomX2Button.signal_toggled().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));

	_zoomMenu.append(_zoomX4Button);
	_zoomX4Button.signal_toggled().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));

	_zoomMenu.append(_zoomX8Button);
	_zoomX8Button.signal_toggled().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));

	_zoomMenu.append(_zoomX16Button);
	_zoomX16Button.signal_toggled().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));

	_zoomMenu.append(_zoomX32Button);
	_zoomX32Button.signal_toggled().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));

	_zoomMenu.append(_zoomX64Button);
	_zoomX64Button.signal_toggled().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));

	_zoomMenu.append(_zoomX128Button);
	_zoomX128Button.signal_toggled().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));
	
	_zoomMenu.show_all_children();

	_topBox.pack_start(_refreshCurrentButton, false, true);
	_refreshCurrentButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onRefreshCurrentClicked));
	_refreshCurrentButton.set_tooltip_text("Refreshes the image so that it matches the current uv plane (any layout changes applied will be lost)");

	_topBox.pack_start(_memoryStoreButton, false, true);
	_memoryStoreButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onMemoryStoreClicked));
	_memoryStoreButton.set_tooltip_text("Store current visible image in the image memory");

	_topBox.pack_start(_memoryRecallButton, false, true);
	_memoryRecallButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onMemoryRecallClicked));
	_memoryRecallButton.set_tooltip_text("Recall a previously stored image from memory");

	_topBox.pack_start(_memoryMultiplyButton, false, true);
	_memoryMultiplyButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onMemoryMultiplyClicked));
	_memoryMultiplyButton.set_tooltip_text("Multiply the current visible image with the image in memory");

	_topBox.pack_start(_memorySubtractButton, false, true);
	_memorySubtractButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onMemorySubtractClicked));
	_memorySubtractButton.set_tooltip_text("Subtract current visible image from memory image");

	_topBox.pack_start(_sqrtButton, false, true);
	_sqrtButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onSqrtClicked));
	_sqrtButton.set_tooltip_text("Take the square root of all values");

	_topBox.pack_start(_plotHorizontalButton, false, true);
	_plotHorizontalButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onPlotHorizontally));
	_plotHorizontalButton.set_tooltip_text("Make plot of amplitudes over x-axis");
	
	_topBox.pack_start(_plotVerticalButton, false, true);
	_plotVerticalButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onPlotVertically));
	_plotVerticalButton.set_tooltip_text("Make plot of amplitudes over y-axis");
	
	_topBox.pack_start(_angularTransformButton, false, true);
	_angularTransformButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onAngularTransformButton));
	_angularTransformButton.set_tooltip_text("Perform an angular transform");
	
	_topBox.pack_start(_saveFitsButton, false, true);
	_saveFitsButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onSaveFitsButton));
	_saveFitsButton.set_tooltip_text("Save the current visible image in a FITS-file");
	
	_topBox.pack_start(_propertiesButton, false, true);
	_propertiesButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onPropertiesButton));
	_propertiesButton.set_tooltip_text("Imaging properties...");
	
	_box.pack_start(_topBox, false, true);
	
	_box.pack_start(_imageWidget);
	_imageWidget.add_events(Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON_PRESS_MASK);
	_imageWidget.OnButtonReleasedEvent().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onButtonReleased));
	_imageWidget.SetRange(ImageWidget::MinMax);

	add(_box);
	_box.show_all();

	onZoomButtonClicked();
}

ImagePlaneWindow::~ImagePlaneWindow()
{
	if(_propertiesWindow != 0)
		delete _propertiesWindow;
}

void ImagePlaneWindow::AddData(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData)
{
	_imager.Image(data, metaData);
	_lastMetaData = metaData;
	Update();
}

void ImagePlaneWindow::AddData(const TimeFrequencyData &data, class SpatialMatrixMetaData *spatialMetaData)
{
	_imager.Image(data, spatialMetaData);
	Update();
}

void ImagePlaneWindow::onClearClicked()
{
	_imager.Empty();
	Update();
}

void ImagePlaneWindow::onUVPlaneButtonClicked()
{
	if(!_displayingUV)
	{
		Update();
	}
}

void ImagePlaneWindow::onImagePlaneButtonClicked()
{
	if(_displayingUV)
		Update();
}

void ImagePlaneWindow::onZoomButtonClicked()
{
	double zoom = 1;
	if(_zoomXd4Button.get_active()) zoom = 0.25;
	else if(_zoomXd2Button.get_active()) zoom = 0.5;
	else if(_zoomX2Button.get_active()) zoom = 2;
	else if(_zoomX4Button.get_active()) zoom = 4;
	else if(_zoomX8Button.get_active()) zoom = 8;
	else if(_zoomX16Button.get_active()) zoom = 16;
	else if(_zoomX32Button.get_active()) zoom = 32;
	else if(_zoomX64Button.get_active()) zoom = 64;
	else if(_zoomX128Button.get_active()) zoom = 128;
	if(_zoom != zoom)
	{
		_imager.Empty();
		_imager.SetUVScaling(0.0001L * (long double) zoom); // TODO
		Update();
		_zoom = zoom;
	}
}

void ImagePlaneWindow::Update()
{
	if(_uvPlaneButton.get_active())
	{
		if(_imager.HasUV()) {
			_imageWidget.SetImage(Image2D::CreateCopyPtr(_imager.RealUVImage()));
			_imageWidget.Update();
			_displayingUV = true;
		}
	}
	else
	{
		if(!_imager.HasFFT() && _imager.HasUV())
			_imager.PerformFFT();

		if(_imager.HasFFT()) {
			_imageWidget.SetImage(Image2D::CreateCopyPtr(_imager.FTReal()));
			_imageWidget.Update();
			printStats();
			_displayingUV = false;
		}
	}
}

void ImagePlaneWindow::onApplyWeightsClicked()
{
	_imager.ApplyWeightsToUV();
	Update();
}

void ImagePlaneWindow::onRefreshCurrentClicked()
{
	Update();
}

void ImagePlaneWindow::onMemoryStoreClicked()
{
	_memory = _imageWidget.Image();
}

void ImagePlaneWindow::onMemoryRecallClicked()
{
	_imageWidget.SetImage(_memory);
	_imageWidget.Update();
}

void ImagePlaneWindow::onMemoryMultiplyClicked()
{
	if(_memory != 0)
	{
		Image2DPtr multiplied = Image2D::CreateCopy(_memory);
		Image2DCPtr old = _imageWidget.Image();
		for(size_t y=0;y<multiplied->Height();++y)
		{
			for(size_t x=0;x<multiplied->Width();++x)
			{
				multiplied->SetValue(x, y, multiplied->Value(x, y) * old->Value(x, y));
			}
		}
		_imageWidget.SetImage(multiplied);
		_imageWidget.Update();
		printStats();
	}
}

void ImagePlaneWindow::onMemorySubtractClicked()
{
	if(_memory != 0)
	{
		Image2DPtr subtracted = Image2D::CreateCopy(_memory);
		Image2DCPtr old = _imageWidget.Image();
		for(size_t y=0;y<subtracted->Height();++y)
		{
			for(size_t x=0;x<subtracted->Width();++x)
			{
				subtracted->SetValue(x, y, subtracted->Value(x, y) - old->Value(x, y));
			}
		}
		_imageWidget.SetImage(subtracted);
		_imageWidget.Update();
		printStats();
	}
}

void ImagePlaneWindow::onSqrtClicked()
{
	if(_imageWidget.HasImage())
	{
		Image2DPtr sqrtImage = Image2D::CreateCopy(_imageWidget.Image());
		FFTTools::SignedSqrt(sqrtImage);
		_imageWidget.SetImage(sqrtImage);
		_imageWidget.Update();
		printStats();
	}
}

void ImagePlaneWindow::onPlotHorizontally()
{
	if(_imageWidget.HasImage())
	{
		Plot plot("Image-horizontal-axis.pdf");
		plot.SetXAxisText("RA index");
		plot.SetYAxisText("Amplitude");
		//plot.SetLogScale(false, true);
		plot.StartLine();
		Image2DCPtr image = _imageWidget.Image();
		for(size_t x=0;x<image->Width();++x)
		{
			num_t sum = 0.0;
			for(size_t y=0;y<image->Height();++y)
			{
				sum += image->Value(x, y);
			}
			plot.PushDataPoint(x, sum);
		}
		plot.Close();
		plot.Show();
	}
}

void ImagePlaneWindow::onPlotVertically()
{
	if(_imageWidget.HasImage())
	{
		Plot plot("Image-vertical-axis.pdf");
		plot.SetXAxisText("Declination index");
		plot.SetYAxisText("Amplitude");
		//plot.SetLogScale(false, true);
		plot.StartLine();
		Image2DCPtr image = _imageWidget.Image();
		for(size_t y=0;y<image->Height();++y)
		{
			num_t sum = 0.0;
			for(size_t x=0;x<image->Width();++x)
			{
				sum += image->Value(x, y);
			}
			plot.PushDataPoint(y, sum);
		}
		plot.Close();
		plot.Show();
	}
}

void ImagePlaneWindow::printStats()
{
	if(_imageWidget.HasImage())
	{
		num_t topLeftRMS = _imageWidget.Image()->GetRMS(0, 0, _imageWidget.Image()->Width()/3, _imageWidget.Image()->Height()/3);
		std::cout << "RMS=" << _imageWidget.Image()->GetRMS()
			<< ", max=" << _imageWidget.Image()->GetMaximum()
			<< ", min=" << _imageWidget.Image()->GetMinimum()
			<< ", top left RMS=" << topLeftRMS
			<< ", SNR=" << _imageWidget.Image()->GetMaximum()/topLeftRMS
			<< std::endl;
	}
}

void ImagePlaneWindow::onButtonReleased(size_t x, size_t y)
{
	if(_imageWidget.HasImage() && _lastMetaData != 0)
	{
		int 
			width = _imageWidget.Image()->Width(),
			height = _imageWidget.Image()->Height();
			
		int left = x - 3, right = x + 3, top = y - 3, bottom = y + 3;
		if(left < 0) left = 0;
		if(right >= width) right = width - 1;
		if(top < 0) top = 0;
		if(bottom >= height) bottom = height - 1;
		
		const BandInfo band = _lastMetaData->Band();
		num_t frequencyHz = band.channels[band.channels.size()/2].frequencyHz;
		num_t rms = _imageWidget.Image()->GetRMS(left, top, right-left, bottom-top);
		num_t max = _imageWidget.Image()->GetMaximum(left, top, right-left, bottom-top);
		num_t xRel = x-width/2.0, yRel = y-height/2.0;
		const numl_t
			dist = sqrtnl(xRel*xRel + yRel*yRel),
			delayRa = _lastMetaData->Field().delayDirectionRA,
			delayDec = _lastMetaData->Field().delayDirectionDec;
		std::cout << "Clicked at: " << xRel << "," << yRel << '\n';
		double
			distanceRad = _imager.ImageDistanceToDecRaDistance(dist);
		std::cout << "RMS=" << rms << ", max=" << max
			<< ", angle=" << (SinusFitter::Phase(xRel, -yRel)*180.0/M_PI) << ", dist=" << dist << "\n"
			<< "Distance ~ "
			<< distanceRad << " rad = "
			<< Angle::ToString(distanceRad) << " = "
			<< (1.0/_imager.ImageDistanceToFringeSpeedInSamples(dist, frequencyHz, _lastMetaData)) << " samples/fringe.\n";
		numl_t
			centerX = cosn(delayRa) * delayDec,
			centerY = -sinn(delayRa) * delayDec,
			dx = _imager.ImageDistanceToDecRaDistance(-xRel) + centerX,
			dy = _imager.ImageDistanceToDecRaDistance(yRel) + centerY,
			ra = 2.0*M_PInl - SinusFitter::Phase(dx, dy),
			dec = sqrtnl(dx*dx + dy*dy);
		std::cout << "Delay = " << RightAscension::ToString(delayRa) << ", " << Declination::ToString(delayDec) << " (@" << dx << "," << dy << ")\n";
		std::cout << "RA = " << RightAscension::ToString(ra) << ", DEC = " << Declination::ToString(dec) << "\n";
	}
}

void ImagePlaneWindow::onAngularTransformButton()
{
	Image2DPtr transformedImage = FFTTools::AngularTransform(_imageWidget.Image());
	_imageWidget.SetImage(transformedImage);
	_imageWidget.Update();
}

void ImagePlaneWindow::onPropertiesButton()
{
	if(_propertiesWindow == 0)
	{
		_propertiesWindow = new ImagePropertiesWindow(_imageWidget, "Display properties for imager window");
		_propertiesWindow->show();
	} else {
		_propertiesWindow->show();
		_propertiesWindow->raise();
	}
}

void ImagePlaneWindow::onSaveFitsButton()
{
	Gtk::FileChooserDialog dialog("Select a measurement set");
	dialog.set_transient_for(*this);

	//Add response buttons the the dialog:
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button("Save", Gtk::RESPONSE_OK);

	Glib::RefPtr<Gtk::FileFilter> fitsFilter = Gtk::FileFilter::create();
	fitsFilter->set_name("Flexible Image Transport System (*.fits)");
	fitsFilter->add_pattern("*.fits");
	fitsFilter->add_mime_type("image/fits");
	dialog.add_filter(fitsFilter);
		
	if(dialog.run() == Gtk::RESPONSE_OK)
	{
		const std::string filename = dialog.get_filename();
		Image2DCPtr image = _imageWidget.Image();
		image->SaveToFitsFile(filename);
	}
}

void ImagePlaneWindow::onZoomMenuButtonClicked()
{
	//_zoomMenu.popup(0, gtk_get_current_event_time());
	_zoomMenu.popup(0, 0);
}
