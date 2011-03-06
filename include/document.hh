/* Copyright (c) 2009-2011, Fortylines LLC
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of fortylines nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY Fortylines LLC ''AS IS'' AND ANY
   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL Fortylines LLC BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#ifndef guarddocument
#define guarddocument

#include "session.hh"
#include "decorator.hh"

/**
   Base document classes.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


/* When the dispatcher has found a matching pattern (and associated 
   document) for a pathname, before it goes on to call the fetch method,
   it will check the behavior of that document.
   
   *always*          call fetch regardless of pathname
   *whenFileExist*   call fetch only if *pathname* exists in siteTop.
   *whenNotCached*   call fetch only if there are no cached version
                     of *pathname*.
*/
enum callFetchType {
    always = 0,	
    whenFileExist,
    whenNotCached
};

typedef 
void (*callFetchFunc)( session& s, const boost::filesystem::path& pathname );

struct fetchEntry {
    boost::regex pat;
    callFetchType behavior;
    callFetchFunc callback;
};

/** Create a file (override it if it already exists). This function
    throws an exception if there is any error. */
void createfile( boost::filesystem::ofstream& strm,
		 const boost::filesystem::path& pathname );

/** Open a file for reading. This function
    throws an exception if there is any error. */
void openfile( boost::filesystem::ifstream& strm, 
	       const boost::filesystem::path& pathname );
 
   
/* Pick the appropriate subclass of *document* based on regular expressions
   applied to the document name.
 */
class dispatchDoc {
public:
    typedef std::map<std::string,std::string> variables;

protected:
    typedef std::list<fetchEntry> aliasSet;
    typedef std::map<std::string,aliasSet> presentationSet;

    presentationSet views;

public:
    dispatchDoc();

    static dispatchDoc *instance;

    void add( const std::string& varname, const boost::regex& r, 
	      callFetchFunc f, callFetchType b = always );

    bool fetch( session& s, const std::string& varname );

    /** returns true if a pattern has matched. */
    bool fetch( session& s, const std::string& varname,
		const boost::filesystem::path& pathname );

    /** \brief handler based on the type of document as filtered by dispatch.
     */
    const fetchEntry* 
    select( const std::string& name, const std::string& value ) const;

};


/** If the pathname is a directory, iterate through all files and call back
    walk() for file whose name match the regular expression *filematch*.
*/
class dirwalker {
protected:
    boost::regex filematch;

    virtual void first() const {}
    virtual void last() const {}

public:
    dirwalker() : filematch(".*") {}

    explicit dirwalker( const boost::regex& fm  ) 
	: filematch(fm) {}

    /** *name* initialized with pathname for the todo filters to set the uuid
	correctly on filters(). 
    */
    virtual void walk( session& s, std::istream& ins, 
		       const std::string& name ) const {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname );

};


/** Add meta information about the document to the session. It includes
    modification date, file revision as well as tags read in the file.
    
    This method is called before the template is processed because meta
    information needs to be propagated into different parts of the template
    and not only in the placeholder for the document.
*/
template<const char *varname>
void metaFetch( session& s, const boost::filesystem::path& pathname )
{
    session::variables::const_iterator found = s.vars.find(varname);
    if( found != s.vars.end() ) {    
	s.out() << found->second.value;
    } else {
	s.out() << pathname;
    }
}


template<const char *value>
void consMeta( session& s, const boost::filesystem::path& pathname )
{
    s.out() << value;
}


/** fetch meta information from *pathname* into session *s* 
    and display the one associated to *varname*. 
    (*whenFileExist*) 
*/
template<const char *value>
void textMeta( session& s, const boost::filesystem::path& pathname )
{
    using namespace boost::filesystem; 
    static const boost::regex valueEx("^(\\S+):\\s+(.*)");

    /* \todo should only load one but how does it sits with dispatchDoc
     that initializes s[varname] by default to "document"? */
    ifstream strm;
    openfile(strm,pathname);
    while( !strm.eof() ) {
	boost::smatch m;
	std::string line;
	std::getline(strm,line);
	if( boost::regex_search(line,m,valueEx) ) {
	    if( m.str(1) == std::string("Subject") ) {
		s.vars["title"] = session::valT(m.str(2));
	    } else {
		s.vars[m.str(1)] = session::valT(m.str(2));
	    }
	} else break;
    }
    strm.close();
    /* 
       std::time_t last_write_time( const path & ph );
       To convert the returned value to UTC or local time, 
       use std::gmtime() or std::localtime() respectively. */
    metaFetch<value>(s,pathname);
}


class text /* whenFileExist */  {
protected:
    /** fixed header printed before the main body of text.
     */
    std::string header;

    decorator *leftDec;
    decorator *rightDec;

public:
    text() 
	: leftDec(NULL), rightDec(NULL) {}

    explicit text( const std::string& h ) 
	: header(h), leftDec(NULL), rightDec(NULL) {}

    /* composer derives from text even though it can be used for non-text
       documents. */
    explicit text( callFetchType b ) 
	: leftDec(NULL), rightDec(NULL) {}

    text( decorator& l,  decorator& r ) 
	: leftDec(&l), rightDec(&r) {}

    /** \brief show difference between two texts side by side 

	\param  doc              document to do syntax coloring
	\param  input            showing on the left pane
	\param  diff             difference between left and right pane
	\param  inputIsLeftSide  true when input stream is left side
    */
    void showSideBySide( session& s, 
			 std::istream& input, std::istream& diff, 
			 bool inputIsLeftSide= true ) const;

    void fetch( session& s, const boost::filesystem::path& pathname );
};

/** Skip over the meta information 
 */
void skipOverTags( session& s, std::istream& istr );

void textFetch( session& s, const boost::filesystem::path& pathname );

void formattedFetch( session& s, const boost::filesystem::path& pathname );


#endif
