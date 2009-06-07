#ifndef guarddiff
#define guarddiff

#if 1
#include "document.hh"

class diff : public document {
public:
	/** \brief show difference between two texts side by side 

		\param  doc              document to do syntax coloring
		\param  input            showing on the left pane
		\param  diff             difference between left and right pane
		\param  inputIsLeftSide  true when input stream is left side
	 */
	static void showSideBySide( document *doc, std::istream& input, 
								std::istream& diff, bool inputIsLeftSide= true );

	void betweenFiles( const boost::filesystem::path& leftPath, 
					   const boost::filesystem::path& rightPath );

    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};
#endif

#endif

