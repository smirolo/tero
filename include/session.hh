/* Copyright (c) 2009, Fortylines LLC
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

#ifndef guardsession
#define guardsession

#include <map>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "webserve.hh"

/**
   Session Manager.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


/** This error is thrown when we are trying to retrieve the value 
    of a variable from the current session and that variable does
    not exist in the variables map.
*/
class undefVariableError : public std::runtime_error {
public:
    explicit undefVariableError( const std::string& varname );
};


/** A session instance is used to store "global variables" as defined
    when the CGI script is invoked and translate relative pathnames 
    into absolute pathnames to files on the server.
 */
class session {
public:
    typedef std::map<std::string,std::string> variables;

protected:
    static bool sessionOptionsInit;
    static boost::program_options::options_description sessionOptions;

public:
    /* \todo workout details of auth.cc first before making private. */
    /* map of variable names to values */
    variables vars;
    variables query;
    
public:

    static boost::filesystem::path storage;

    /* unique identifier for the session */
    int64_t id;

    std::string username;
    
    
    session();

    static void 
    addSessionVars( boost::program_options::options_description& opts );

    /** Transforms the path *p* into a fully qualified URL to access
	the file through an HTTP connection. */
    url asUrl( const boost::filesystem::path& p ) const;

    /** document name as an url 
     */
    url docAsUrl() const;
        
    boost::filesystem::path contributorLog() const;

    /** absolute name to the user personal directory
     */
    boost::filesystem::path userPath() const;
    
    bool exists() const { return id > 0; }
    
    /** \brief Value of a variable as an absolute pathname
	
	If the variable exists in the session and its value can
	be interpreted as an absolute path, that value is returned 
	as such else if the variable exists in the session and its value 
	can be interpreted as a relative path, the pwd, siteTop and srcTop
	directories are searched in order until a match is found.

	If no match can be found, there is no actual files associated
	with that variable, we assume the file will be generated and
	thus return siteTop + value.
    */
    boost::filesystem::path 
    abspath( const boost::filesystem::path& name ) const;

    /* returns the equivalent to path *p* within the build tree 
       rooted at siteTop. */
    boost::filesystem::path build( const boost::filesystem::path& p ) const;

    /** returns true if *left* is a prefix of *right*.
     */
    bool prefix( const boost::filesystem::path& left, 
		 const boost::filesystem::path& right ) const;
    
    /** \brief Load a session from persistent storage 
     */
    void restore( const boost::program_options::variables_map& params );

    /* look for a relative pathname *trigger* from *leaf* to the root
       of the filesystem and return the stem such that stem / *trigger*
       is the absolute pathname to the trigger. 
       If *trigger* cannot be found, the method returns an empty path.
    */
    boost::filesystem::path 
    root( const boost::filesystem::path& leaf,
	  const boost::filesystem::path& trigger ) const;

    std::string root() const {
	return std::string("");
    }
    
    /** \brief Display debug information for the session
     */
    void show( std::ostream& ostr );

    /* returns the equivalent to path *p* within the source tree 
       rooted at srcTop. */
    boost::filesystem::path src( const boost::filesystem::path& p ) const;

    /** returns the absolute pathname formed by srcTop/p.
     */
    boost::filesystem::path srcDir( const boost::filesystem::path& p ) const;

   /* session start time */
    void start();

    /* stop counter and return session time. */
    boost::posix_time::time_duration stop();
    
    /** Store session information into persistent storage 
     */
    void store();

    /** returns the path relative to *root* of *leaf* when both *root*
	and *leaf* are absolute paths and *root* is a leading prefix 
	of *leaf*.
    */
    boost::filesystem::path 
    subdirpart( const boost::filesystem::path& root,
		  const boost::filesystem::path& leaf ) const;

    /* Returns the value of a variable as an absolute pathname. */
    boost::filesystem::path valueAsPath( const std::string& name ) const;

    /** returns the value of a variable
     */
    const std::string& valueOf( const std::string& name ) const;
};


#endif
