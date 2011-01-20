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


void createfile( boost::filesystem::ofstream& strm,
		 const boost::filesystem::path& pathname );


/* Base class to generate HTML presentation of documents.
 */
class document {
public:
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

    callFetchType behavior;

protected:
    void open( boost::filesystem::ifstream& strm, 
	       const boost::filesystem::path& pathname ) const;
    
public:
    document() : behavior(always) {}

    explicit document( callFetchType b ) : behavior(b) {}

    virtual void fetch( session& s, 
			const boost::filesystem::path& pathname ) const = 0;

};


/* Pick the appropriate subclass of *document* based on regular expressions
   applied to the document name.
 */
class dispatchDoc {
public:
    typedef std::map<std::string,std::string> variables;

protected:
    typedef std::list<std::pair<boost::regex,const document*> > aliasSet;
    typedef std::map<std::string,aliasSet> presentationSet;

    presentationSet views;

public:
    dispatchDoc();

    static dispatchDoc *instance;

    void add( const std::string& varname, const boost::regex& r, 
	      const document& d );

    bool fetch( session& s, const std::string& varname );

    /** returns true if a pattern has matched. */
    bool fetch( session& s, const std::string& varname,
		const boost::filesystem::path& pathname );

    /** \brief handler based on the type of document as filtered by dispatch.
     */
    const document* select( const std::string& name, const std::string& value ) const;

};


/** If the pathname is a directory, iterate through all files and call back
    walk() for file whose name match the regular expression *filematch*.
*/
class dirwalker : public document {
protected:
    boost::regex filematch;

    virtual void first() const {}
    virtual void last() const {}

public:
    dirwalker() : filematch(".*") {}

    explicit dirwalker( const boost::regex& fm  ) 
	: filematch(fm) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;

    /** *name* initialized with pathname for the todo filters to set the uuid
	correctly on filters(). 
    */
    virtual void walk( session& s, std::istream& ins, 
		       const std::string& name ) const {}
};

/** Add meta information about the document to the session. It includes
    modification date, file revision as well as tags read in the file.
    
    This method is called before the template is processed because meta
    information needs to be propagated into different parts of the template
    and not only in the placeholder for the document.
*/

class meta : public document {
protected:
    const std::string varname;

public:
    explicit meta( const std::string& n )
	: document(always), varname(n) {}

    meta( const std::string& n, callFetchType b )
	: document(b), varname(n) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};



/** associate a constant to a meta.
*/
class consMeta : public meta {
protected:
    const std::string value;

public:
    consMeta( const std::string& n, const std::string& v ) 
	: meta(n,always), value(v) {}
    
    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};


/** fetch meta information from *pathname* into session *s* 
    and display the one associated to *varname*. 
*/
class textMeta : public meta {
public:
    explicit textMeta( const std::string& v ) 
	: meta(v,whenFileExist) {}
    
    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};

class text : public document {
protected:
    /** fixed header printed before the main body of text.
     */
    std::string header;

    decorator *leftDec;
    decorator *rightDec;

    void skipOverTags( session& s, std::istream& istr ) const;

public:
    text() 
	: document(whenFileExist), leftDec(NULL), rightDec(NULL) {}

    explicit text( const std::string& h ) 
	: document(whenFileExist), header(h), leftDec(NULL), rightDec(NULL) {}

    /* composer derives from text even though it can be used for non-text
       documents. */
    explicit text( callFetchType b ) 
	: document(b), leftDec(NULL), rightDec(NULL) {}

    text( decorator& l,  decorator& r ) 
	: document(whenFileExist), leftDec(&l), rightDec(&r) {}

    /** \brief show difference between two texts side by side 

	\param  doc              document to do syntax coloring
	\param  input            showing on the left pane
	\param  diff             difference between left and right pane
	\param  inputIsLeftSide  true when input stream is left side
    */
    void showSideBySide( session& s, 
			 std::istream& input, std::istream& diff, 
			 bool inputIsLeftSide= true ) const;

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};


#endif
