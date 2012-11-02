#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

struct NewStarEntry
{
	string name;
	double I;
	string RA, DEC;
	string ID;
	double Q, U, V;
	double longSize, shortSize;
	double PA, SI, RM, reserved;
};

bool readEntry(istream &stream, class NewStarEntry &output)
{
	string token;
	stream >> token;
	if(token == "end") return false;
	output.name = token;
	stream
		>> output.I >> output.RA >> output.DEC >> output.ID >> output.Q >> output.U >> output.V
		>> output.longSize >> output.shortSize >> output.PA >> output.SI >> output.RM >> output.reserved;

	return true;
}

int main(int, char *[])
{
	cout
		<< "This program will convert a newstar skymodel to a bbs skymodel.\n"
		<< "Copy the NewStar-formatted sources to the terminal, end with a line with \"end\":" << endl;

	vector<struct NewStarEntry*> entries;
	NewStarEntry *entry = new NewStarEntry();
	while(readEntry(cin, *entry))
	{
		entries.push_back(entry);
		entry = new NewStarEntry();
	}
	delete entry;

	string filename;
	getline(cin, filename); // read garbage (return) after "end"

	cout << "Read " << entries.size() << " entries." << endl;
	cout << "Name of output file: ";
	getline(cin, filename);
	ofstream ostr(filename.c_str());

	ostr <<
		"############################################################\n"
		"# " << filename << "\n"
		"# (Name, Type, Ra, Dec, I, Q, U, V, ReferenceFrequency=’55.468e6’, "
		"SpectralIndexDegree=’0’, SpectralIndex:0=’0.0’, SpectralIndex:1=’0.0’, "
		"MajorAxis, MinorAxis, Orientation) = format\n";

	for(vector<struct NewStarEntry*>::iterator i=entries.begin();i!=entries.end();++i)
	{
		const NewStarEntry *entry = *i;
		ostr
			<< entry->name << ", POINT, " << entry->RA << ", " << entry->DEC << ", "
			<< (entry->I*0.005) << ", "
			<< (entry->Q*entry->I*0.01*0.005) << ", "
			<< (entry->U*entry->I*0.01*0.005) << ", "
			<< (entry->V*entry->I*0.01*0.005)
			<< "\n";
		delete *i;
	}
	ostr.close();
	return 0;
}
