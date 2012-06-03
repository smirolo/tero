/* Copyright (c) 2009-2012, Fortylines LLC
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

#ifndef guardrevsys
#define guardrevsys

#include "post.hh"
#include "changelist.hh"

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

	std::map<std::string,std::string> config;

public:
    explicit revisionsys( const char* m ) : metadir(m) {}

    virtual void loadconfig( session& s ) = 0;

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

	const std::string& configval( const std::string& key );

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

    virtual void checkins( const session& s,
			   const boost::filesystem::path& pathname,
			   postFilter& filter ) = 0;

    virtual void showDetails( std::ostream& ostr,			
			      const std::string& commit ) = 0;
    
    /** returns the revision system associated with a pathname 
     */
    static revisionsys*
    findRev( session& s, const boost::filesystem::path& pathname );

    /** returns the revision system associated with a specific metadir
     */
    static revisionsys*
    findRevByMetadir( session& s, const std::string& metadir );

};

#endif
