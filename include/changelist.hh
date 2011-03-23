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


/** Add session variables related to changelist module. 
*/
void 
changelistAddSessionVars( boost::program_options::options_description& all,
			  boost::program_options::options_description& visible );



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




/** Command to cancel web edits
*/
void cancelFetch( session& s, const boost::filesystem::path& pathname );


void changeFetch( session& s, const boost::filesystem::path& pathname );


/** Diff between two revisions of a file under revision control (*always*)

 *primary* decorator to pass character output through.
 *secondary* decorator to pass character output through.
 */
void changediff( session& s, const boost::filesystem::path& pathname,
		 decorator *primary, decorator *secondary );


/** Short history of changes to a project under revision control. 

    This will generate the same output as changehistory but with hyperlinks
    to an expended description of the checkin instead of a difference between
    the current file and the revision.
 */
void changecheckinFetch( session& s, const boost::filesystem::path& pathname );


/** History of changes to a file under revision control
 */
void changehistoryFetch( session& s, const boost::filesystem::path& pathname );


/** Display a unified diff between a commit and the previous one. 
 */
void changeShowDetails( session& s, const boost::filesystem::path& pathname );


/** Populate *s.feeds* with the commit log of *pathname*.
 */
void 
feedRepositoryPopulate( session& s, const boost::filesystem::path& pathname );


/** Populate a feed from the commit log of *pathname* in a repository
 */
template<typename defaultWriter>
void feedRepository( session& s, const boost::filesystem::path& pathname ) {
    defaultWriter writer(s.out());
    if( !s.feeds ) {
	s.feeds = &writer;
    }

    feedRepositoryPopulate(s,pathname);

    if( s.feeds == &writer ) {
	s.feeds->flush();
	s.feeds = NULL;
    }
}


#endif
