#ifndef DUMPIMAGESACTION_H
#define DUMPIMAGESACTION_H

#include <boost/thread/mutex.hpp>
#include "action.h"
#include "../control/artifactset.h"
#include "../imagesets/imageset.h"

namespace rfiStrategy {
    class DumpImagesAction : public Action
    {
    public:
	DumpImagesAction () : _ident (0), _dumpcount (0) {}
	virtual ~DumpImagesAction () {}

	virtual std::string Description ()
	{
	    std::stringstream s;
	    s << "Dump current images to fileset #" << _ident;
	    return s.str ();
	}

	virtual void Perform (class ArtifactSet &artifacts, ProgressListener &)
	{
	    boost::mutex::scoped_lock lock (artifacts.IOMutex ());

	    _savetf (artifacts.OriginalData (), "ori");
	    _savetf (artifacts.RevisedData (), "rev");
	    _savetf (artifacts.ContaminatedData (), "cnt");
	    _dumpcount++;
	}

	virtual ActionType Type () const
	{
	    return DumpImagesActionType;
	}

	size_t Ident () const throw () { return _ident; }
	void SetIdent (size_t newIdent) throw () { _ident = newIdent; }

    private:
	size_t _ident, _dumpcount;

	void _savetf (TimeFrequencyData &tf, std::string subident)
	{
	    for (size_t i = 0; i < tf.ImageCount (); i++) {
		std::stringstream s;

		s << "dump_" << _ident << "_" << _dumpcount << "_"
		  << subident << ".fits";

		tf.GetImage (i)->SaveToFitsFile (s.str ());
	    }
	}
    };
}
#endif
