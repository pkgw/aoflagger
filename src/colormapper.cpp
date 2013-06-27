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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>

#include "msio/fitsfile.h"
#include "msio/image2d.h"
#include "msio/pngfile.h"

#include "util/ffttools.h"

using namespace std;

struct ImageInfo { unsigned index; long double variance; };

bool operator<(const ImageInfo &a, const ImageInfo &b) {
	return a.variance > b.variance; // note that a noise image is "smaller" than a clean image
}

void addFits(Image2D *red, Image2D *green, Image2D *blue, Image2D *mono, const std::string &filename);

void HLStoRGB(long double hue,long double lum,long double sat,long double &red,long double &green, long double &blue);
void WLtoRGB(long double wavelength,long double &red,long double &green, long double &blue);
inline void ScaledWLtoRGB(long double position,long double &red,long double &green, long double &blue)
{
	WLtoRGB(position*300.0+400.0,red,green,blue);
	if(red < 0.0) red = 0.0;
	if(red > 1.0) red = 1.0;
	if(green < 0.0) green = 0.0;
	if(green > 1.0) green = 1.0;
	if(blue < 0.0) blue = 0.0;
	if(blue > 1.0) blue = 1.0;
}
void ReportRMS(Image2D *image);

int main(int argc, char *argv[])
{
	int pindex = 1;
	// parameters
	bool useSpectrum = true;
	bool colormap = false;
	int removeNoiseImages = 0;
	bool fft = false;
	enum ScaleMethod { MaximumContrast, Constant } scaleMethod = MaximumContrast;
	long double scaleValue = 1.0;
	std::string subtractFile, outputFitsFile, outputPngFile;
	bool subtract = false, redblue = false, rms = false, individualMaximization = false, displayMax = false, singleImage = false;
	bool window = false, cutWindow = false, saveFits = false, savePng = false;
	size_t windowX = 0, windowY = 0, windowWidth = 0, windowHeight = 0;
	size_t cutWindowX = 0, cutWindowY = 0, cutWindowWidth = 0, cutWindowHeight = 0;
	size_t singleImageIndex = 0;

	while(pindex < argc && argv[pindex][0] == '-') {
		string parameter = argv[pindex]+1;
		if(parameter == "s") { useSpectrum = true; }
		else if(parameter == "c") { useSpectrum = false; }
		else if(parameter == "d") { ++pindex; subtractFile = argv[pindex]; subtract=true; }
		else if(parameter == "fft") { fft = true; }
		else if(parameter == "fi") { individualMaximization = true; }
		else if(parameter == "fits") {
			saveFits = true;
			++pindex; outputFitsFile = argv[pindex];
		}
		else if(parameter == "fm") { scaleMethod = MaximumContrast; }
		else if(parameter == "fv") { scaleMethod = Constant; ++pindex; scaleValue = atof(argv[pindex]); }
		else if(parameter == "m") { colormap = true; }
		else if(parameter == "max") { displayMax=true; }
		else if(parameter == "png")
		{
			savePng = true;
			++pindex; outputPngFile = argv[pindex];
		}
		else if(parameter == "r") { ++pindex; removeNoiseImages = atoi(argv[pindex]); }
		else if(parameter == "rb") { redblue=true; }
		else if(parameter == "rms") { rms=true; }
		else if(parameter == "si")
		{
			singleImage = true;
			++pindex; singleImageIndex = atoi(argv[pindex]);
		}
		else if(parameter == "w") {
			window = true;
			++pindex; windowX = atoi(argv[pindex]);
			++pindex; windowY = atoi(argv[pindex]);
			++pindex; windowWidth = atoi(argv[pindex]);
			++pindex; windowHeight = atoi(argv[pindex]);
		}
		else if(parameter == "wc") {
			cutWindow = true;
			++pindex; cutWindowX = atoi(argv[pindex]);
			++pindex; cutWindowY = atoi(argv[pindex]);
			++pindex; cutWindowWidth = atoi(argv[pindex]);
			++pindex; cutWindowHeight = atoi(argv[pindex]);
		}
		else {
			cerr << "Unknown parameter: -" << parameter << endl;
			return -1;
		}
		++pindex;
	}

	if(argc-pindex < 1) {
		cerr << "Usage: \n\t" << argv[0] << " [options] <input fits file>\n"
				"\toptions:\n"
				"\t-d <fitsfile> subtract the file from the image\n"
				"\t-fft perform fft before combining\n"
				"\t-fi maximize each individual image before summing\n"
				"\t-fits <file> store in fits file (does not preserve the headers)\n"
				"\t-fm scale colors for maximum contrast, upper 0.02% of the data will be oversaturated (default)\n"
				"\t-fv <value> scale so that <value> flux is full brightness\n"
				"\t-m add colormap to image\n"
				"\t-max display maximum of each image\n"
				"\t-png <file> save as png file\n"
				"\t-rb don't use frequency colored, but use red/blue map for positive/negative values\n"
				"\t-rms calculate and show the rms of the upperleft 10% data\n"
				"\t-s use spectrum (default)\n"
				"\t-si <index> select single image from each fits file\n"
				"\t-c use color circle\n"
				"\t-w <x> <y> <width> <height> select a window of each frame only\n"
				"\t-wc <x> <y> <width> <height> cut a window in each frame\n";
		return -1;
	}

	Image2D *red = 0;
	Image2D *green = 0;
	Image2D *blue = 0;
	Image2D *mono = 0;

	long double totalRed = 0.0, totalGreen = 0.0, totalBlue = 0.0;
	unsigned addedCount = 0;
	
	size_t inputCount = argc-pindex;
	for(unsigned inputIndex=pindex;inputIndex<(unsigned) argc;++inputIndex)
	{
		cout << "Opening " << argv[inputIndex] << "..." << endl;
		FitsFile fitsfile(argv[inputIndex]);
		fitsfile.Open(FitsFile::ReadOnlyMode);
		fitsfile.MoveToHDU(1);
		unsigned images = Image2D::GetImageCountInHUD(fitsfile);

		FitsFile *subtractFits = 0;
		if(subtract) {
			cout << "Opening " << subtractFile << "..." << endl;
			subtractFits = new FitsFile(subtractFile);
			subtractFits->Open(FitsFile::ReadOnlyMode);
			subtractFits->MoveToHDU(1);
			unsigned sImages = Image2D::GetImageCountInHUD(fitsfile);
			if(sImages < images)
				images = sImages;
		}

		std::vector<ImageInfo> variances;

		if(removeNoiseImages > 0) {
			cout << "Sorting images on noise level..." << endl;
			for(unsigned i=0;i<images;++i)
			{
				if(i % 8 == 0) {
					unsigned upper = i+9;
					if(upper > images) upper = images;
					cout << "Measuring noise level in images " << (i+1) << " - " << upper << "..." << endl;
				}
				Image2D *image = Image2D::CreateFromFits(fitsfile, i);
				struct ImageInfo imageInfo;
				imageInfo.variance = image->GetRMS();
				imageInfo.index = i;
				variances.push_back(imageInfo);
				delete image;
			}
			sort(variances.begin(), variances.end());

			cout << "The following images are removed because of too much noise: "
					<< variances.front().index;
			for(std::vector<ImageInfo>::const_iterator i=variances.begin()+1;i<variances.begin()+removeNoiseImages;++i)
			{
				cout << ", " << i->index;
			}
			cout << endl;
		}

		unsigned lowI, highI;
		if(singleImage)
		{
			lowI = singleImageIndex;
			highI = singleImageIndex+1;
		} else {
			lowI = 0;
			highI = images;
		}
			
		for(unsigned i=lowI;i<highI;++i)
		{
			if(i % 8 == 0) {
				unsigned upper = i+9;
				if(upper > images) upper = images;
				cout << "Adding image " << (i+1) << " - " << upper << "..." << endl;
			}
			bool skip = false;
			if(removeNoiseImages > 0) {
				for(std::vector<ImageInfo>::const_iterator j=variances.begin();j<variances.begin()+removeNoiseImages;++j)
				{
					if(j->index == i) { skip = true; break; }
				}
			}
			if(!skip) {
				long double wavelengthRatio;
				if(images*inputCount > 1)
					wavelengthRatio = (1.0 - (long double) (i+((int) inputIndex-(int) pindex)*images) / (images*inputCount-1.0));
				else
					wavelengthRatio = 0.5;
				std::cout << "ratio=" << wavelengthRatio << '\n';
				Image2D *image = Image2D::CreateFromFits(fitsfile, i);
				if(subtract)
				{
					Image2D *imageB = Image2D::CreateFromFits(*subtractFits, i);
					Image2D *AminB = Image2D::CreateFromDiff(*image, *imageB);
					delete image;
					image = AminB;
					delete imageB;
				}
				if(window)
				{
					Image2D *empty = Image2D::CreateZeroImage(image->Width(), image->Height());
					for(unsigned y=image->Height()-windowY-windowHeight;y<image->Height()-windowY;++y)
					{
						for(unsigned x=windowX;x<windowX+windowWidth;++x)
							empty->SetValue(x, y, image->Value(x, y));
					}
					delete image;
					image = empty;
				}
				if(cutWindow)
				{
					for(unsigned y=image->Height()-cutWindowY-cutWindowHeight;y<image->Height()-cutWindowY;++y)
					{
						for(unsigned x=cutWindowX;x<cutWindowX+cutWindowWidth;++x)
							image->SetValue(x, y, 0.0);
					}
				}
				if(fft) {
					Image2D *fft = FFTTools::CreateFFTImage(*image, FFTTools::Absolute);
					Image2D *fullfft = FFTTools::CreateFullImageFromFFT(*fft);
					delete image;
					delete fft;
					image = fullfft;
				}
				long double max;
				if(individualMaximization) {
					max = image->GetMaximum();
					if(max <= 0.0) max = 1.0;
				} else {
					max = 1.0;
				}
				if(displayMax)
					cout << "max=" << image->GetMinimum() << ":" << image->GetMaximum() << endl; 
				if(rms)
					ReportRMS(image);
				long double r=0.0,g=0.0,b=0.0;
				if(redblue) {
					r = 1.0;
					b = 1.0;
				} else if(useSpectrum)
					ScaledWLtoRGB(wavelengthRatio, r, g, b);
				else
					HLStoRGB(wavelengthRatio, 0.5, 1.0, r, g, b);
				totalRed += r;
				totalGreen += g;
				totalBlue += b;
				if(red == 0) {
					red = Image2D::CreateUnsetImage(image->Width(), image->Height());
					green = Image2D::CreateUnsetImage(image->Width(), image->Height());
					blue = Image2D::CreateUnsetImage(image->Width(), image->Height());
					mono = Image2D::CreateUnsetImage(image->Width(), image->Height());
				}
				size_t minY = image->Height(), minX = image->Width();
				if(red->Height() < minY) minY = red->Height();
				if(red->Width() < minX) minX = red->Width();
				for(unsigned y=0;y<minY;++y)
				{
					for(unsigned x=0;x<minX;++x)	
					{
						long double value = image->Value(x, y);
						mono->AddValue(x, y, value);
						if(redblue) {
							if(value > 0)
								red->AddValue(x, y, value/max);
							else
								blue->AddValue(x, y, value/(-max)); 
						}
						else {
							if(value < 0.0) value = 0.0;
							value /= max;
							if(colormap && (y < 96 && y >= 32 && x < images*8)) { 
								if(x >= i*8 && x < i*8+8) {
									red->SetValue(x, y, r * images);
									green->SetValue(x, y, g * images);
									blue->SetValue(x, y, b * images);
								}
							} else {
								red->AddValue(x, y, r * value);
								green->AddValue(x, y, g * value);
								blue->AddValue(x, y, b * value);
							}
						}
					}
				}
				++addedCount; 
				delete image;
			}
		}
		
		if(subtract) {
			subtractFits->Close();
			delete subtractFits;
		}
	}
		
	cout << "Scaling to ordinary units..." << endl;
	for(unsigned y=0;y<red->Height();++y) {
		for(unsigned x=0;x<red->Width();++x) {
			red->SetValue(x, y, red->Value(x, y) / addedCount);
			blue->SetValue(x, y, blue->Value(x, y) / addedCount);
			green->SetValue(x, y, green->Value(x, y) / addedCount);
			mono->SetValue(x, y, mono->Value(x, y) / addedCount);
		}
	}

	if(rms) {
		ReportRMS(mono);
	}
	
	if(saveFits)
	{
		cout << "Saving fits file..." << endl;
		mono->SaveToFitsFile(outputFitsFile);
	}

	if(savePng)
	{
		cout << "Normalizing..." << endl;
		long double maxRed, maxGreen, maxBlue;
		switch(scaleMethod) {
			default:
			case MaximumContrast:
				maxRed = red->GetTresholdForCountAbove(red->Width() * red->Height() / 5000);
				maxGreen = green->GetTresholdForCountAbove(green->Width() * green->Height() / 5000);
				maxBlue = blue->GetTresholdForCountAbove(blue->Width() * blue->Height() / 5000);
			break;
			case Constant:
				maxRed = scaleValue;
				maxGreen = scaleValue * totalGreen / totalRed;
				maxBlue = scaleValue * totalBlue / totalRed;
			break; 
		}
		if(maxRed <= 0.0) maxRed = 1.0;
		if(maxGreen <= 0.0) maxGreen = 1.0;
		if(maxBlue <= 0.0) maxBlue = 1.0;
		cout << "Contrast stretch value for red: " << maxRed << endl; 
		
		PngFile file(outputPngFile, red->Width(), red->Height());
		file.BeginWrite();

		cout << "Writing " << outputPngFile << "..." << endl;
		for(unsigned y=0;y<red->Height();++y)
		{
			for(unsigned x=0;x<red->Width();++x)	
			{
				unsigned
					r = (unsigned) ((red->Value(x, y) / maxRed) * 255.0),
					g = (unsigned) ((green->Value(x, y) / maxGreen) * 255.0),
					b = (unsigned) ((blue->Value(x, y) / maxBlue) * 255.0);
				if(r > 255) r = 255;
				if(g > 255) g = 255;
				if(b > 255) b = 255;
				file.PlotDatapoint(x, red->Height() - 1 - y, r, g, b, 255);
			}
		}
		file.Close();
	}
	
	delete red;
	delete green;
	delete blue;
	delete mono;

  return EXIT_SUCCESS;
}

/* utility routine for HLStoRGB */ 
long double HueToRGB(long double p,long double q,long double tc)
{
	/* range check: note values passed add/subtract thirds of range */ 
	if (tc < 0)
			tc += 1.0;

	if (tc > 1.0)
			tc -= 1.0;

	/* return r,g, or b value from this tridrant */ 
	if (tc < (1.0/6.0))
			return ( p + (q-p)*6.0*tc);
	if (tc < 0.5)
			return ( q );
	if (tc < 2.0/3.0)
			return ( p + (q-p)*6.0*(2.0/3.0 - tc));
	else
			return ( p );
}

void HLStoRGB(long double hue,long double lum,long double sat,long double &red,long double &green, long double &blue)
{

	if (sat == 0) {
			red=green=blue=lum;
		}
	else  {
			long double q,p;
			if (lum < 0.5)
				q = lum*(1.0 + sat);
			else
				q = lum + sat - (lum*sat);
			p = 2.0*lum-q;

			red = HueToRGB(p,q,hue+1.0/3.0);
			green = HueToRGB(p,q,hue);
			blue = HueToRGB(p,q,hue-1.0/3.0);
	}
}

void WLtoRGB(long double wavelength,long double &red,long double &green, long double &blue)
{
  if(wavelength >= 350.0 && wavelength <= 439.0) {
   red	= -(wavelength - 440.0) / (440.0 - 350.0);
   green = 0.0;
   blue	= 1.0;
  } else if(wavelength >= 440.0 && wavelength <= 489.0) {
   red	= 0.0;
   green = (wavelength - 440.0) / (490.0 - 440.0);
   blue	= 1.0;
  } else if(wavelength >= 490.0 && wavelength <= 509.0) {
   red = 0.0;
   green = 1.0;
   blue = -(wavelength - 510.0) / (510.0 - 490.0);
  } else if(wavelength >= 510.0 && wavelength <= 579.0) { 
   red = (wavelength - 510.0) / (580.0 - 510.0);
   green = 1.0;
   blue = 0.0;
  } else if(wavelength >= 580.0 && wavelength <= 644.0) {
   red = 1.0;
   green = -(wavelength - 645.0) / (645.0 - 580.0);
   blue = 0.0;
  } else if(wavelength >= 645.0 && wavelength <= 780.0) {
   red = 1.0;
   green = 0.0;
   blue = 0.0;
  } else {
   red = 1.0;
   green = 0.0;
   blue = 0.0;
  }
  if(wavelength >= 350.0 && wavelength <= 419.0) {
	  long double factor;
    factor = 0.3 + 0.7*(wavelength - 350.0) / (420.0 - 350.0);
		red *= factor;
		green *= factor;
		blue *= factor;
  } else if(wavelength >= 420.0 && wavelength <= 700.0) {
    // nothing to be done
  } else if(wavelength >= 701.0 && wavelength <= 780.0) {
	  long double factor;
    factor = 0.3 + 0.7*(780.0 - wavelength) / (780.0 - 700.0);
		red *= factor;
		green *= factor;
		blue *= factor;
  } else if(wavelength >= 780.0) {
    long double factor;
    factor = 0.3;
    red *= factor;
    green *= factor;
    blue *= factor;
  } else {
		red = 0.0;
		green = 0.0;
		blue = 0.0;
 }
}

void ReportRMS(Image2D *image)
{
	unsigned squareWidth = image->Width()/5;
	unsigned squareHeight = image->Height()/5;
	cout << "Calculating rms... " << endl;
	cout << "Total RMS=" << image->GetRMS()*1000.0L << "mJ" << endl;
	cout << "Center RMS=" << image->GetRMS(image->Width()/2-squareWidth/2,image->Height()/2-squareHeight/2, squareWidth, squareHeight)*1000.0L << "mJ" << endl;
	cout << "Upperleft RMS=" << image->GetRMS(0, 0, squareWidth, squareHeight)*1000.0L << "mJ" << endl;
	cout << "Upperright RMS=" << image->GetRMS(image->Width()-squareWidth, 0, squareWidth, squareHeight) * 1000.0L << "mJ" << endl;
	cout << "Lowerleft RMS=" << image->GetRMS(0, image->Height()-squareHeight, squareWidth, squareHeight) * 1000.0L << "mJ" << endl;
	cout << "Lowerright RMS=" << image->GetRMS(image->Width()-squareWidth, image->Height()-squareHeight, squareWidth, squareHeight) *1000.0L << "mJ" << endl;
	cout << "Minimum intensity=" << image->GetMinimum() * 1000.0L << "mJ" << endl;
	cout << "Maximum intensity=" << image->GetMaximum() * 1000.0L << "mJ" << endl;
}
