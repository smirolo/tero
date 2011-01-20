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

#ifndef guardchangelist
#define guardchangelist

#include "composer.hh"
#include "post.hh"

/* Different displays of changes to an underlying source control repository.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

class historyref {
public:
    historyref() {}

    virtual url asUrl( const boost::filesystem::path& doc, 
		       const std::string& rev ) const = 0;
};

class diffref : public historyref {
public:
    diffref() {}

    virtual url asUrl( const boost::filesystem::path& doc, 
		       const std::string& rev ) const;
};

class checkinref : public historyref {
public:
    checkinref() {}

    virtual url asUrl( const boost::filesystem::path& doc, 
		       const std::string& rev ) const;
};

class checkin : public post {
public:
    typedef std::list<boost::filesystem::path> fileSet;

public:
    fileSet files;

    checkin() {}

    void addFile( const boost::filesystem::path& pathname ) {
	files.push_back(pathname);
    }
};


/** History of checkins on a project
 */
class history {
public:
    typedef std::list<checkin> checkinSet;
    
public:
    checkinSet checkins;

    checkin* add() { 
	checkin ci;
	checkins.push_back(ci);
	return &checkins.back();
    }
};


/** Base class definition for interacting with a source code control system.    
 */
class revisionsys {
public:
    typedef std::vector<revisionsys*> revsSet;
    static revsSet revs;

    boost::filesystem::path rootpath;

    /** Directory name in which revision control meta information 
	is stored (example: .git). 
    */
    const char* metadir;

public:
    explicit revisionsys( const char* m ) : metadir(m) {}

    /** Create a new repository from directory *pathname*. When *group*
	is true, all users in the same group as the creator have permissions
	to push changes into the repository otherwise only the creator
	can push changes.
     */
    virtual void create( const boost::filesystem::path& pathname,
			 bool group = false ) = 0;

    /** Add *pathname* to the changelist to be committed. 
     */
    virtual void add( const boost::filesystem::path& pathname ) = 0;

    /** Commit the default changelist.
     */
    virtual void commit( const std::string& msg ) = 0;

    virtual void diff( std::ostream& ostr, 
		       const std::string& leftCommit, 
		       const std::string& rightCommit, 
		       const boost::filesystem::path& pathname ) = 0;
    
    virtual void history( std::ostream& ostr,
			  const session& s, 
			  const boost::filesystem::path& pathname,
			  historyref& r ) = 0;
    
    virtual void checkins( ::history& hist,
			   const session& s, 
			   const boost::filesystem::path& pathname ) = 0;
    
    /** returns the revision system associated with a pathname 
     */
    static revisionsys*
    findRev( session& s, const boost::filesystem::path& pathname );

    /** returns the revision system associated with a specific metadir
     */
    static revisionsys*
    findRevByMetadir( session& s, const std::string& metadir );

};


/** Command to cancel web edits
*/
class cancel : public document {
public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};


/* Add a changed file to the default change list 
*/
class change : public document {
public:
    static void 
    addSessionVars( boost::program_options::options_description& opts );

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;
};


/** Diff between two revisions of a file under revision control
 */
class changediff : public document {
public:
    changediff() 
	: document(always) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;

};


/** Base class for commands displaying changelists.
 */
class changelist : public document {
};


/** Short history of changes to a project under revision control. 

    This will generate the same output as changehistory but with hyperlinks
    to an expended description of the checkin instead of a difference between
    the current file and the revision.
 */
class changecheckin : public changelist {
public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;	
};


/** History of changes to a file under revision control
 */
class changehistory : public changelist {
public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname ) const;	
};


#endif
