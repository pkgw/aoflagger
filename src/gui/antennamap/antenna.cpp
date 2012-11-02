#include "antenna.h"
#include "antennamap.h"

namespace antennaMap
{

bool Antenna::Draw(Cairo::RefPtr<Cairo::Context> cairo, double offsetX, double offsetY, int layer, double timeInFrameSeconds)
{
	runTo(timeInFrameSeconds);
	double
		centerX = _map.UCSToPixelX(GetXInUCS()),
		centerY = _map.UCSToPixelY(GetYInUCS());
	cairo->set_line_width(2.0);
	
	switch(layer)
	{
		case 0:
			// The red circle that represents the value
			cairo->arc(centerX + offsetX, centerY + offsetY, 5.0 + _value * GetMaxCircleRadius(), 0.0, 2.0*M_PI);
			cairo->set_source_rgba(1.0, 0.0, 0.0, 0.25);
			cairo->fill();
			break;
		case 1:
			// A gray circle that represents the running max value
			cairo->arc(centerX + offsetX, centerY + offsetY, 5.0 + _runningMaxValue * GetMaxCircleRadius(), 0.0, 2.0*M_PI);
			cairo->set_source_rgba(0.5, 0.5, 0.5, 1.0);
			cairo->stroke();
			break;
		case 2:
			// A black with gray filled circle to specify the position of the antenna/station
			cairo->arc(centerX + offsetX, centerY + offsetY, 5.0, 0.0, 2.0*M_PI);
			cairo->set_source_rgba(0.5, 0.5, 0.5, 1.0);
			cairo->fill_preserve();
			
			cairo->set_source_rgba(0.0, 0.0, 0.0, 1.0);
			cairo->stroke();
			break;
	}
	return layer < 2;
}

}
