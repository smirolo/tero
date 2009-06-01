#ifndef guardcomposer
#define guardcomposer

#include "document.hh"

class composer : public document {
public:
    enum fileNotFoundBehavior {
	create,
	error
    };

protected:
    fileNotFoundBehavior behavior;
    boost::filesystem::path fixed;

	/** Embed the content of a variable into a page. 
	 */
	virtual void embed( session& s, const std::string& value );

public:
    composer() {}

    explicit composer( fileNotFoundBehavior b ) : behavior(b) {}

	composer( const boost::filesystem::path& f,
			  fileNotFoundBehavior b ) 
		: behavior(b), fixed(f) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};


#endif
