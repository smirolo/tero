#ifndef guarddocument
#define guarddocument

#include "session.hh"
#include "decorator.hh"

class document {
protected:
	void open( boost::filesystem::ifstream& strm, 
			   const boost::filesystem::path& pathname ) const;

public:
	virtual void fetch( session& s, const boost::filesystem::path& pathname ) = 0;
};


class dispatch {
public:
	typedef std::map<std::string,std::string> variables;

protected:
	typedef std::list<std::pair<boost::regex,document*> > aliasSet;
	typedef std::map<std::string,aliasSet> presentationSet;

	boost::filesystem::path root;
	presentationSet views;

public:
	explicit dispatch( const boost::filesystem::path& root );

	static dispatch *instance;

	void add( const std::string& varname, const boost::regex& r, std::ostream& d );

	void add( const std::string& varname, const boost::regex& r, document& d );

	void fetch( session& s, const std::string& varname );

	/** \brief handler based on the type of document as filtered by dispatch.
	 */
	document* select( const std::string& name, const std::string& value ) const;

};


class text : public document {
protected:
	decorator *leftDec;
	decorator *rightDec;

public:

	text( decorator& l,  decorator& r ) 
		: leftDec(&l), rightDec(&r) {}

	/** \brief show difference between two texts side by side 

		\param  doc              document to do syntax coloring
		\param  input            showing on the left pane
		\param  diff             difference between left and right pane
		\param  inputIsLeftSide  true when input stream is left side
	 */
	void showSideBySide( std::istream& input, std::istream& diff, 
						  bool inputIsLeftSide= true );

	virtual void fetch( session& s, const boost::filesystem::path& pathname );

};


#endif
