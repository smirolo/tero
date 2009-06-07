#ifndef guardchangelist
#define guardchangelist

#include "composer.hh"

/* Cancel edits
*/
class cancel : public document {
public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};


class revisionsys {
public:
	boost::filesystem::path rootpath;

public:
	virtual void diff( std::ostream& ostr, 
					   const std::string& leftCommit, 
					   const std::string& rightCommit, 
					   const boost::filesystem::path& pathname ) = 0;

	virtual void history( std::ostream& ostr, 
						  const boost::filesystem::path& pathname ) = 0;

	/* Sets the path to the root of the source code control system.
	 */
	void rootPath( const boost::filesystem::path& p ) {
		rootpath = p;
	}
};

/** \brief generate git commands to the revision control system.
 */
class gitcmd : public revisionsys {
protected:
	boost::filesystem::path executable;

public:
	gitcmd( const boost::filesystem::path& exec ) 
		: executable(exec) {}

	void diff( std::ostream& ostr, 
			  const std::string& leftCommit, 
			  const std::string& rightCommit, 
			   const boost::filesystem::path& pathname );

	void history( std::ostream& ostr, 
				  const boost::filesystem::path& pathname );

}; 


/* Add a changed file to the default change list 
*/
class change : public document {
public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};


/** \brief Diff between two revisions of a file under revision control
 */
class changediff : public composer {
protected:
	revisionsys *revision;

	/** Embed the content of a variable into a page. 
	 */
	virtual void embed( session& s, const std::string& value );

public:
	changediff( const boost::filesystem::path& f, revisionsys *r ) 
		: composer(f,error), revision(r) {}

};


class changelist : public document {
public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};


/** \brief History of changes to a file under revision control
 */
class changehistory : public document {
protected:
	revisionsys *revision;

public:
	explicit changehistory( revisionsys *r ) : revision(r) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname );	
};


#endif
