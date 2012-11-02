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
#include "timefrequencydata.h"
#include "stokesimager.h"

#include "../util/ffttools.h"

Image2DCPtr TimeFrequencyData::GetAbsoluteFromComplex(const Image2DCPtr &real, const Image2DCPtr &imag) const
{
	return Image2DPtr(FFTTools::CreateAbsoluteImage(*real, *imag));
}
			
Image2DCPtr TimeFrequencyData::GetSum(const Image2DCPtr &left, const Image2DCPtr &right) const
{
	return StokesImager::CreateSum(left, right);
}

Image2DCPtr TimeFrequencyData::GetNegatedSum(const Image2DCPtr &left, const Image2DCPtr &right) const
{
	return StokesImager::CreateNegatedSum(left, right);
}

Image2DCPtr TimeFrequencyData::GetDifference(const Image2DCPtr &left, const Image2DCPtr &right) const
{
	return StokesImager::CreateDifference(left, right);
}

Image2DCPtr TimeFrequencyData::GetSinglePhaseFromDipolePhase(size_t xx, size_t yy) const
{
	return StokesImager::CreateAvgPhase(_images[xx], _images[yy]);
}

Image2DCPtr TimeFrequencyData::GetZeroImage() const
{
	return Image2D::CreateZeroImagePtr(_images[0]->Width(), _images[0]->Height());
}

Mask2DCPtr TimeFrequencyData::GetCombinedMask() const
{
	if(_flagging.empty())
		return GetSetMask<false>();
	else if(_flagging.size() == 1)
		return _flagging[0];
	else
	{
		std::vector<Mask2DCPtr>::const_iterator i = _flagging.begin();
		Mask2DPtr mask = Mask2D::CreateCopy(*i);
		++i;
		while(i!=_flagging.end())
		{
			for(unsigned y=0;y<mask->Height();++y) {
				for(unsigned x=0;x<mask->Width();++x) {
					bool v = (*i)->Value(x, y);
					if(v)
						mask->SetValue(x, y, true);
				}
			}
			++i;
		}
		return mask;
	}
}

TimeFrequencyData *TimeFrequencyData::CreateTFDataFromSingleComplex(enum PhaseRepresentation phase) const
{
	TimeFrequencyData *data;
	switch(phase)
	{
		case RealPart:
			data = new TimeFrequencyData(RealPart, _polarisationType, _images[0]);
			break;
		case ImaginaryPart:
			data = new TimeFrequencyData(ImaginaryPart, _polarisationType, _images[1]);
			break;
		case AmplitudePart:
			data = new TimeFrequencyData(AmplitudePart, _polarisationType, GetAbsoluteFromComplex(0, 1));
			break;
		case PhasePart:
			data = new TimeFrequencyData(PhasePart, _polarisationType, GetPhaseFromComplex(_images[0], _images[1]));
			break;
		default:
			throw BadUsageException("Creating TF data with non implemented phase parameters");
	}
	CopyFlaggingTo(data);
	return data;
}

TimeFrequencyData *TimeFrequencyData::CreateTFDataFromDipoleComplex(enum PhaseRepresentation phase) const
{
	TimeFrequencyData *data;
	switch(phase)
	{
		case RealPart:
			data = new TimeFrequencyData(RealPart,
				GetRealPartFromDipole(XXPolarisation),
				GetRealPartFromDipole(XYPolarisation),
				GetRealPartFromDipole(YXPolarisation),
				GetRealPartFromDipole(YYPolarisation));
			break;
		case ImaginaryPart:
			data = new TimeFrequencyData(ImaginaryPart,
				GetImaginaryPartFromDipole(XXPolarisation),
				GetImaginaryPartFromDipole(XYPolarisation),
				GetImaginaryPartFromDipole(YXPolarisation),
				GetImaginaryPartFromDipole(YYPolarisation));
			break;
		case AmplitudePart:
			{
				Image2DCPtr
					xx = GetAmplitudePartFromDipole(XXPolarisation),
					xy = GetAmplitudePartFromDipole(XYPolarisation),
					yx = GetAmplitudePartFromDipole(YXPolarisation),
					yy = GetAmplitudePartFromDipole(YYPolarisation);
				data = new TimeFrequencyData(AmplitudePart, xx, xy, yx, yy);
			}
			break;
		case PhasePart:
			{
				Image2DCPtr
					xx = GetPhasePartFromDipole(XXPolarisation),
					xy = GetPhasePartFromDipole(XYPolarisation),
					yx = GetPhasePartFromDipole(YXPolarisation),
					yy = GetPhasePartFromDipole(YYPolarisation);
				data = new TimeFrequencyData(PhasePart, xx, xy, yx, yy);
			}
			break;
		default:
			throw BadUsageException("Creating TF data with non implemented phase parameters (not real/imaginary/amplitude)");
	}
	CopyFlaggingTo(data);
	return data;
}

TimeFrequencyData *TimeFrequencyData::CreateTFDataFromAutoDipoleComplex(enum PhaseRepresentation phase) const
{
	TimeFrequencyData *data;
	switch(phase)
	{
		case RealPart:
			data = new TimeFrequencyData(RealPart, AutoDipolePolarisation,
				GetRealPartFromAutoDipole(XXPolarisation),
				GetRealPartFromAutoDipole(YYPolarisation));
			break;
		case ImaginaryPart:
			data = new TimeFrequencyData(ImaginaryPart, AutoDipolePolarisation,
				GetImaginaryPartFromAutoDipole(XXPolarisation),
				GetImaginaryPartFromAutoDipole(YYPolarisation));
			break;
		case AmplitudePart:
			{
				Image2DCPtr
					xx = GetAmplitudePartFromAutoDipole(XXPolarisation),
					yy = GetAmplitudePartFromAutoDipole(YYPolarisation);
				data = new TimeFrequencyData(AmplitudePart, AutoDipolePolarisation, xx, yy);
			}
			break;
		case PhasePart:
			{
				Image2DCPtr
					xx = GetPhasePartFromAutoDipole(XXPolarisation),
					yy = GetPhasePartFromAutoDipole(YYPolarisation);
				data = new TimeFrequencyData(PhasePart, AutoDipolePolarisation, xx, yy);
			}
			break;
		default:
			throw BadUsageException("Creating TF data with non implemented phase parameters (not real/imaginary/amplitude)");
	}
	CopyFlaggingTo(data);
	return data;
}

TimeFrequencyData *TimeFrequencyData::CreateTFDataFromCrossDipoleComplex(enum PhaseRepresentation phase) const
{
	TimeFrequencyData *data;
	switch(phase)
	{
		case RealPart:
			data = new TimeFrequencyData(RealPart, CrossDipolePolarisation,
				GetRealPartFromCrossDipole(XYPolarisation),
				GetRealPartFromCrossDipole(YXPolarisation));
			break;
		case ImaginaryPart:
			data = new TimeFrequencyData(ImaginaryPart, CrossDipolePolarisation,
				GetImaginaryPartFromCrossDipole(XYPolarisation),
				GetImaginaryPartFromCrossDipole(YXPolarisation));
			break;
		case AmplitudePart:
			{
				Image2DCPtr
					xy = GetAmplitudePartFromCrossDipole(XYPolarisation),
					yx = GetAmplitudePartFromCrossDipole(YXPolarisation);
				data = new TimeFrequencyData(AmplitudePart, CrossDipolePolarisation, xy, yx);
			}
			break;
		case PhasePart:
			{
				Image2DCPtr
					xy = GetPhasePartFromAutoDipole(XYPolarisation),
					yx = GetPhasePartFromAutoDipole(YXPolarisation);
				data = new TimeFrequencyData(PhasePart, CrossDipolePolarisation, xy, yx);
			}
			break;
		default:
			throw BadUsageException("Creating TF data with non implemented phase parameters (not real/imaginary/amplitude)");
	}
	CopyFlaggingTo(data);
	return data;
}

TimeFrequencyData *TimeFrequencyData::CreateTFDataFromComplexCombination(const TimeFrequencyData &real, const TimeFrequencyData &imaginary)
{
	if(real.PhaseRepresentation() == ComplexRepresentation || imaginary.PhaseRepresentation() == ComplexRepresentation)
		throw BadUsageException("Trying to create complex from real/imaginary time frequency that is are already complex");
	if(real.Polarisation() != imaginary.Polarisation())
		throw BadUsageException("Combining real/imaginary time frequency data from different polarisations"); 
	switch(real.Polarisation())
	{
		case XXPolarisation:
		case XYPolarisation:
		case YXPolarisation:
		case YYPolarisation:
		case SinglePolarisation:
		case StokesIPolarisation:
		case StokesQPolarisation:
		case StokesUPolarisation:
		case StokesVPolarisation:
			return new TimeFrequencyData(real.Polarisation(), real._images[0], imaginary._images[0]);
		case DipolePolarisation:
			return new TimeFrequencyData(
				real._images[0], imaginary._images[0],
				real._images[1], imaginary._images[1],
				real._images[2], imaginary._images[2],
				real._images[3], imaginary._images[3]);
		case AutoDipolePolarisation:
			return new TimeFrequencyData(AutoDipolePolarisation,
				real._images[0], imaginary._images[0],
				real._images[1], imaginary._images[1]);
		case CrossDipolePolarisation:
			return new TimeFrequencyData(CrossDipolePolarisation,
				real._images[0], imaginary._images[0],
				real._images[1], imaginary._images[1]);
	}
	throw BadUsageException("Invalid polarisation type");
}

TimeFrequencyData *TimeFrequencyData::CreateTFDataFromDipoleCombination(const TimeFrequencyData &xx, const TimeFrequencyData &xy, const TimeFrequencyData &yx, const TimeFrequencyData &yy)
{
	if(xx.PhaseRepresentation() != xy.PhaseRepresentation() ||
		xx.PhaseRepresentation() != yx.PhaseRepresentation() ||
		xx.PhaseRepresentation() != yy.PhaseRepresentation())
		throw BadUsageException("Trying to create dipole time frequency combination from data with different phase representations!");

	TimeFrequencyData *data;
	switch(xx.PhaseRepresentation())
	{
		case PhasePart:
		case AmplitudePart:
		case RealPart:
		case ImaginaryPart:
			data = new TimeFrequencyData(xx.PhaseRepresentation(), xx._images[0], xy._images[0],yx._images[0], yy._images[0]);
			break;
		case ComplexRepresentation:
			data = new TimeFrequencyData(xx._images[0], xx._images[1], xy._images[0], xy._images[1], yx._images[0], yx._images[1], yy._images[0], yy._images[1]);
			break;
		default:
			throw BadUsageException("Invalid phase representation");
	}
	data->SetIndividualPolarisationMasks(xx.GetSingleMask(), xy.GetSingleMask(), yx.GetSingleMask(), yy.GetSingleMask());
	return data;
}

TimeFrequencyData *TimeFrequencyData::CreateTFDataFromAutoDipoleCombination(const TimeFrequencyData &xx, const TimeFrequencyData &yy)
{
	if(xx.PhaseRepresentation() != yy.PhaseRepresentation())
		throw BadUsageException("Trying to create auto dipole time frequency combination from data with different phase representations!");

	TimeFrequencyData *data;
	switch(xx.PhaseRepresentation())
	{
		case PhasePart:
		case AmplitudePart:
		case RealPart:
		case ImaginaryPart:
			data = new TimeFrequencyData(xx.PhaseRepresentation(), AutoDipolePolarisation, xx._images[0], yy._images[0]);
			break;
		case ComplexRepresentation:
			data = new TimeFrequencyData(AutoDipolePolarisation, xx._images[0], xx._images[1], yy._images[0], yy._images[1]);
			break;
		default:
			throw BadUsageException("Invalid phase representation");
	}
	data->SetIndividualPolarisationMasks(xx.GetSingleMask(), yy.GetSingleMask());
	return data;
}

TimeFrequencyData *TimeFrequencyData::CreateTFDataFromCrossDipoleCombination(const TimeFrequencyData &xy, const TimeFrequencyData &yx)
{
	if(xy.PhaseRepresentation() != yx.PhaseRepresentation())
		throw BadUsageException("Trying to create cross dipole time frequency combination from data with different phase representations!");

	TimeFrequencyData *data;
	switch(xy.PhaseRepresentation())
	{
		case PhasePart:
		case AmplitudePart:
		case RealPart:
		case ImaginaryPart:
			data = new TimeFrequencyData(xy.PhaseRepresentation(), AutoDipolePolarisation, xy._images[0], yx._images[0]);
			break;
		case ComplexRepresentation:
			data = new TimeFrequencyData(AutoDipolePolarisation, xy._images[0], xy._images[1], yx._images[0], yx._images[1]);
			break;
		default:
			throw BadUsageException("Invalid phase representation");
	}
	data->SetIndividualPolarisationMasks(xy.GetSingleMask(), yx.GetSingleMask());
	return data;
}

void TimeFrequencyData::SetImagesToZero()
{
	if(!IsEmpty())
	{
		Image2DPtr zeroImage = Image2D::CreateZeroImagePtr(ImageWidth(), ImageHeight());
		Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(ImageWidth(), ImageHeight());
		for(std::vector<Image2DCPtr>::iterator i=_images.begin();i!=_images.end();++i)
			(*i) = zeroImage;
		for(std::vector<Mask2DCPtr>::iterator i=_flagging.begin();i!=_flagging.end();++i)
			(*i) = mask;
	}
}

void TimeFrequencyData::MultiplyImages(long double factor)
{
	for(std::vector<Image2DCPtr>::iterator i=_images.begin();i!=_images.end();++i)
	{
		Image2DPtr newImage = Image2D::CreateCopy(*i);
		newImage->MultiplyValues(factor);
		(*i) = newImage;
	}
}

void TimeFrequencyData::JoinMask(const TimeFrequencyData &other)
{
	if(other._flagging.size() == _flagging.size())
	{
		for(size_t i=0;i<_flagging.size();++i)
		{
			Mask2DPtr mask = Mask2D::CreateCopy(_flagging[i]);
			mask->Join(other._flagging[i]);
			_flagging[i] = mask;
		}
	} else if(other._flagging.size() == 1)
	{
		for(size_t i=0;i<_flagging.size();++i)
		{
			Mask2DPtr mask = Mask2D::CreateCopy(_flagging[i]);
			mask->Join(other._flagging[0]);
			_flagging[i] = mask;
		}
	} else if(_flagging.size() == 1)
	{
		Mask2DPtr mask = Mask2D::CreateCopy(_flagging[0]);
		mask->Join(other.GetSingleMask());
		_flagging[0] = mask;
	} else if(other._flagging.empty())
	{
		// Nothing to be done; other has no flags
	}	else if(_flagging.empty())
	{
		for(std::vector<Mask2DCPtr>::const_iterator i=other._flagging.begin();
			i!=other._flagging.end();++i)
		{
			_flagging.push_back(*i);
		}
	}
	else
		throw BadUsageException("Joining time frequency flagging with incompatible structures");
}

Image2DCPtr TimeFrequencyData::GetPhaseFromComplex(const Image2DCPtr &real, const Image2DCPtr &imag) const
{
	return FFTTools::CreatePhaseImage(real, imag);
}
