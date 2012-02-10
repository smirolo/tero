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
#include "markup.hh" 

/**
   Basic functions to display the content of a "document".

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

// forward declaration
template<typename charT, typename traitsT >
class basicDecorator;
typedef basicDecorator<char,std::char_traits<char> > decorator;


/* When the dispatcher has found a matching pattern (and associated 
   callback) for a pathname, before it goes on to call the fetch method,
   it will check if the document should be served regardless or only
   when the underlying file exists.
   
   *always*          call fetch regardless of pathname
   *notAFile*        call fetch regardless but does not transform pathname
                     into an absolute path (for printing  metaValue).
   *whenFileExist*   call fetch only if *pathname* exists in siteTop.
   *whenNotCached*   call fetch only if there are no cached version
                     of *pathname*.
*/
enum callFetchType {
    always        = 0x0,	
	notAFile      = 0x1,
    whenFileExist = 0x2,
    whenNotCached = 0x3
};

/** When the dispatcher has found a matching pattern (and associated 
   callback) for a pathname, before it goes on to call the fetch method,
   it will check if the document should be served to everyone or only
   to authenticated users.
*/
enum authFetchType {
	noAuth        = 0x0,
	whenAuth      = 0x4
};

/** When the dispatcher has found a matching pattern (and associated 
   callback) for a pathname, before it goes on to call the fetch method,
   it will check if the document is the next step in a process pipeline
   (ex. registration) or not and serve it accordingly.
*/
enum pipeFetchType {
	noPipe        = 0x0,
	whenPipe      = 0x8
};


/** Prototype for document callbacks
 */
typedef 
void (*callFetchFunc)( session& s, const boost::filesystem::path& pathname );


/** An entry in the dispatch table.
 */
struct fetchEntry {
    const char *name;
    boost::regex pat;
    uint8_t behavior;
    callFetchFunc callback;
};

inline
bool operator<( const fetchEntry& left, const fetchEntry& right ) {
    return strcmp(left.name,right.name) < 0;
}


extern urlVariable nextpage;

/** Add session variables related to generic documents.
 */
void docAddSessionVars( boost::program_options::options_description& opts,
						boost::program_options::options_description& visible );

   
/* Pick the appropriate presentation entry (callback) based on regular 
   expressions applied to a document name.
 */
class dispatchDoc {
protected:    
    fetchEntry* entries;
    size_t nbEntries;

    static dispatchDoc *singleton;

    void fetch( session& s, 
				const fetchEntry *doc, const boost::filesystem::path& value );

public:

    /** Initialize the dispatcher with a set of (name,pattern,callback) 
	enties.

	The entries have to be sorted by name in increasing alphabetical 
	order. Out of the patterns associated to a specific name, the first
	matching rule applies.
    */
    dispatchDoc( fetchEntry* entries, size_t nbEntries );

    /** returns the singleton instance
     */
    static dispatchDoc *instance() { 
	assert( singleton != NULL );
	return singleton;
    }

    /** Invoke the callback associated with *widget* and the matching
	pattern for *value*.
	
	This method returns true when a callback has been invoked.
    */
    bool fetch( session& s, 
		const std::string& widget, const url& value );

    /** Returns the entry associated with *name* and those pattern
	matches *value* or NULL if no entries could be found.
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


/** Always prints *value*.
 */
template<const char *value>
void consMeta( session& s, const boost::filesystem::path& pathname )
{
    s.out() << value;
}

/** Assign the last modification time of *pathname* to the time session 
    variable.
 */
void metaLastTime( session& s, const boost::filesystem::path& pathname );


/** Assign the owner name of *pathname* to the author session variable.
 */
void metaFileOwner( session& s, const boost::filesystem::path& pathname );


/** Prints *pathname* on the session output.
 */
void metaValue( session& s, const boost::filesystem::path& pathname );

/** Prints the content of *varname* or the relative url to *pathname*
    when *varname* cannot be found in the session.
*/
template<const char *varname>
void metaFetch( session& s, const boost::filesystem::path& pathname )
{
    session::variables::const_iterator look = s.find(varname);
    if( s.found(look) ) {    
       s.out() << look->second.value;
    } else {
       s.out() << s.subdirpart(siteTop.value(s),pathname);
    }
}


/** Fetch meta information from *pathname* into session *s* 
    and prints the value of meta information associated to *varname*.

    Text meta information consists of all entries of the form Name: Value
    at the beginning of a text file.    
*/
template<const char *varname>
void textMeta( session& s, const boost::filesystem::path& pathname )
{
    using namespace boost::filesystem; 
    static const boost::regex valueEx("^(\\S+):\\s+(.*)");

    /* \todo should only load one but how does it sits with dispatchDoc
     that initializes s[varname] by default to "document"? */
    ifstream strm;
    s.openfile(strm,pathname);
    while( !strm.eof() ) {
	boost::smatch m;
	std::string line;
	std::getline(strm,line);
	if( boost::regex_search(line,m,valueEx) ) {
	    if( m.str(1) == std::string("Subject") ) {
		s.insert("title",m.str(2));
	    } else if( m.str(1) == std::string("From") ) {
		s.insert("author",extractName(m.str(2)));
		s.insert("authorEmail",extractEmailAddress(m.str(2)));
	    } else {
		std::string name = m.str(1);
		name[0] = tolower(name[0]);
		s.insert(name,m.str(2));
	    }
	} else break;
    }
    strm.close();
    /* 
       std::time_t last_write_time( const path & ph );
       To convert the returned value to UTC or local time, 
       use std::gmtime() or std::localtime() respectively. */
    metaFetch<varname>(s,pathname);
}


class text /* whenFileExist */  {
protected:
    decorator *leftDec;
    decorator *rightDec;

public:
    text() 
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


/** Skip over the meta information (i.e. Name: Value) in a text file 
    and prints the remaining of the file "as is".
 */
void textFetch( session& s, const boost::filesystem::path& pathname );

/** Skip over the meta information (i.e. Name: Value) in a text file 
    and prints the remaining of the file highlighting url links.
 */
void formattedFetch( session& s, const boost::filesystem::path& pathname );

#endif
