#include "pngreader.h"

#include <png.h>

rfiStrategy::BaselineData* rfiStrategy::PngReader::Read()
{
	FILE *fp = fopen(_path.c_str(), "rb");
	if(fp==0)
		throw std::runtime_error("Could not open file");

	png_structp png_ptr = png_create_read_struct
		(PNG_LIBPNG_VER_STRING, (png_voidp) 0, 0, 0);

	if (!png_ptr)
		throw std::runtime_error("Error creating png struct");

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, 	(png_infopp) 0, (png_infopp) 0);
		throw std::runtime_error("Error creating png read struct");
	}
    
	png_init_io(png_ptr, fp);
    
	png_read_info(png_ptr, info_ptr);

	unsigned width = png_get_image_width(png_ptr, info_ptr);
	unsigned height = png_get_image_height(png_ptr, info_ptr);
	unsigned color_type = png_get_color_type(png_ptr, info_ptr);
	unsigned bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	AOLogger::Debug << "Png file: " << width << 'x' << height << " colortype=" << color_type << ", bit_depth=" << bit_depth << '\n';

	png_read_update_info(png_ptr, info_ptr);

	/* read file */
	if(setjmp(png_jmpbuf(png_ptr)))
		throw std::runtime_error("Png error");

	png_bytep* row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	for (size_t y=0; y<height; y++)
		row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

	png_read_image(png_ptr, row_pointers);

	fclose(fp);
				
	Image2DPtr image = Image2D::CreateZeroImagePtr(width, height);
	std::ifstream file(_path.c_str());
	size_t bytesPerSample = 4;
	for(size_t f=0;f<height;++f)
	{
		for(size_t t=0;t<width;++t)
		{
			int
				r = row_pointers[f][t*bytesPerSample],
				g = row_pointers[f][t*bytesPerSample+1],
				b = row_pointers[f][t*bytesPerSample+2];
			image->SetValue(t, height-1-f, r+g+b);
		}
	}
	
	TimeFrequencyData tfData(TimeFrequencyData::AmplitudePart,
		SinglePolarisation,
		image);
	return new BaselineData(tfData, TimeFrequencyMetaDataCPtr());
}
