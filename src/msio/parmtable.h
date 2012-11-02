#ifndef PARM_TABLE_H
#define PARM_TABLE_H

#include <map>
#include <set>
#include <stdexcept>

#include <ms/MeasurementSets/MSTable.h>

#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>

#include "../util/aologger.h"

#include "image2d.h"
#include "timefrequencydata.h"

class ParmTable
{
	public:
		struct GainNameEntry
		{
			int index;
			int x, y;
			enum Component { Real, Imaginary } component;
			std::string antenna;
			
			GainNameEntry() :
				index(0), x(0), y(0), component(Real), antenna()
				{
				}
			GainNameEntry(const GainNameEntry &source) :
				index(source.index), x(source.x), y(source.y), component(source.component), antenna(source.antenna)
				{
				}
			void operator=(const GainNameEntry &source)
			{
				index = source.index;
				x = source.x;
				y = source.y;
				component = source.component;
				antenna = source.antenna;
			}
		};
		
		ParmTable(const std::string &path)
			: _path(path)
		{
			readNames();
		}
		
		std::set<std::string> GetAntennas() const
		{
			std::set<std::string> antennas;
			for(GainNameEntryMap::const_iterator i=_nameEntries.begin();i!=_nameEntries.end();++i)
			{
				const GainNameEntry &entry = i->second;
				antennas.insert(entry.antenna);
			}
			return antennas;
		}
		
		TimeFrequencyData Read(const std::string &antenna)
		{
			AOLogger::Debug << "Reading antenna " << antenna << "\n";
			
			// find the nameid's that we need to select
			const int
				r00 = FindEntry(0, 0, GainNameEntry::Real, antenna).index,
				r11 = FindEntry(1, 1, GainNameEntry::Real, antenna).index,
				i00 = FindEntry(0, 0, GainNameEntry::Imaginary, antenna).index,
				i11 = FindEntry(1, 1, GainNameEntry::Imaginary, antenna).index;
			AOLogger::Debug
				<< "Names: r00=" << r00 << ", "
				<< "r11=" << r11 << ", "
				<< "i00=" << i00 << ", "
				<< "i11=" << i11 << "\n";
				
			casa::Table table(_path);
			
			// Construct the images
			unsigned width, height;
			getImageDimensions(table, width, height, r00, r11, i00, i11);
			Image2DPtr
				xxReal = Image2D::CreateZeroImagePtr(width, height),
				yyReal = Image2D::CreateZeroImagePtr(width, height),
				xxImag = Image2D::CreateZeroImagePtr(width, height),
				yyImag = Image2D::CreateZeroImagePtr(width, height);
				
			// Read data
			casa::ROScalarColumn<unsigned int> nameIdColumn(table, "NAMEID");
			casa::ROScalarColumn<double>
				startX(table, "STARTX"),
				startY(table, "STARTY");
			casa::ROArrayColumn<double> values(table, "VALUES");
				
			int xPos=0, yPos=0;
			//double currentX=startX(0);
			double currentY=startY(0);
			unsigned r00Count=0, r11Count=0, i00Count=0, i11Count=0;
			unsigned curXShape=0;
			unsigned componentMatches = 0;
			for(unsigned row=0;row < table.nrow();++row)
			{
				int nameId = nameIdColumn(row);
				if(nameId == r00 || nameId == r11 || nameId == i00 || nameId == i11)
				{
					Image2DPtr destImage;
					if(nameId == r00) {
						destImage = xxReal;
						++r00Count;
					} else if(nameId==r11) {
						destImage = yyReal;
						++r11Count;
					} else if(nameId==i00) {
						destImage = xxImag;
						++i00Count;
					} else if(nameId==i11) {
						destImage = yyImag;
						++i11Count;
					}
					
					const unsigned curYShape = values.shape(row)[1];
					const unsigned xShape = values.shape(row)[0];
					if(xShape > curXShape)
						curXShape = xShape;
					
					const casa::Array<double> valueArray = values(row);
					casa::Array<double>::const_iterator vIter = valueArray.begin();
					for(unsigned x=0;x<xShape;++x) {
						for(unsigned y=0;y<curYShape;++y) {
							destImage->SetValue(yPos + y, xPos + x, *vIter);
							++vIter;
						}
					}
					
					++componentMatches;
					if(componentMatches >= 4)
					{
						if(startY(row) < currentY)
						{
							xPos += curXShape;
							yPos = 0;
							curXShape = 0;
						} else {
							yPos += curYShape;
						}
						//currentX=startX(row);
						currentY=startY(row);
						componentMatches = 0;
					}
				}
			}
			AOLogger::Debug
				<< "Counts: r00=" << r00Count << ", "
				<< "r11=" << r11Count << ", "
				<< "i00=" << i00Count << ", "
				<< "i11=" << i11Count << "\n";
			return TimeFrequencyData(AutoDipolePolarisation, xxReal, xxImag, yyReal, yyImag);
		}
		
		const GainNameEntry &FindEntry(int x, int y, enum GainNameEntry::Component c, const std::string &antenna) const
		{
			for(GainNameEntryMap::const_iterator i=_nameEntries.begin();i!=_nameEntries.end();++i)
			{
				const GainNameEntry &entry = i->second;
				if(entry.x == x && entry.y == y && entry.component == c && entry.antenna == antenna)
				{
					return entry;
				}
			}
			throw std::runtime_error("Entry not found");
		}

	private:
		void readNames()
		{
			casa::Table namesTable;
			if(_path.size()>0 && *_path.rbegin()!='/')
				namesTable = casa::Table(_path + "/NAMES");
			else
				namesTable = casa::Table(_path + "NAMES");
			
			casa::ROScalarColumn<casa::String> nameColumn(namesTable, "NAME");
			for(unsigned i=0;i!=namesTable.nrow();++i)
			{
				std::string name = nameColumn(i);
				addName(i, name);
			}
		}
		
		void getImageDimensions(casa::Table &table, unsigned &width, unsigned &height, int r00, int /*r11*/, int /*i00*/, int /*i11*/)
		{
			casa::ROScalarColumn<unsigned int> nameIdColumn(table, "NAMEID");
			casa::ROScalarColumn<double>
				startX(table, "STARTX"),
				startY(table, "STARTY");
			casa::ROArrayColumn<double> values(table, "VALUES");
			
			int maxX=0,yPos=0;
			int maxY=0;
			unsigned matches = 0;
			unsigned curXShape=0;
			double currentX=startX(0), currentY=startY(0);
			for(unsigned row=0;row < table.nrow();++row)
			{
				int nameId = nameIdColumn(row);
				if(nameId == r00)
				{
					const unsigned curYShape = values.shape(row)[1];
					if(values.shape(row)[0] > curXShape)
						curXShape = values.shape(row)[0];
					if(startX(row) < currentX)
						throw std::runtime_error("Table is not correctly ordered");
					yPos += curYShape;
					if(startY(row) < currentY)
					{
						maxX += curXShape;
						curXShape = 0;
						if(yPos > maxY) maxY = yPos;
						yPos = 0;
					}
					
					currentX=startX(row);
					currentY=startY(row);
					++matches;
				}
			}
			maxX += curXShape;
			if(yPos > maxY) maxY = yPos;
			
			width = maxY;
			height = maxX;
			AOLogger::Debug << "Rows in table: " << table.nrow() << "\n"
				"Matching rows: " << matches << "\n"
				"Number of blocks: " << maxX << " x " << maxY << "\n"
				"Image size: " << width << " x " << height << "\n";
		}
		
		void addName(unsigned index, const std::string &line)
		{
			size_t
				d1 = line.find(':');
			std::string type = line.substr(0, d1);
			if(type == "Gain")
			{
				GainNameEntry entry;
				size_t
					d2 = line.find(':', d1+1),
					d3 = line.find(':', d2+1),
					d4 = line.find(':', d3+1);
				entry.index = index;
				entry.x = atoi(line.substr(d1+1, d2-d1-1).c_str());
				entry.y = atoi(line.substr(d2+1, d3-d2-1).c_str());
				std::string component = line.substr(d3+1, d4-d3-1);
				if(component == "Real")
					entry.component = GainNameEntry::Real;
				else if(component == "Imag")
					entry.component = GainNameEntry::Imaginary;
				else
					throw std::runtime_error("Incorrect complex component type given");
				entry.antenna = line.substr(d4+1);
				_nameEntries.insert(std::pair<unsigned, GainNameEntry>(index, entry));
			}
		}
		
		std::string _path;
		typedef std::map<unsigned, GainNameEntry> GainNameEntryMap;
		GainNameEntryMap _nameEntries;
};

#endif
