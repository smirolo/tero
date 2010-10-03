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


/** A session instance is used to store a set of (name,value) pairs
    persistent accross runs of the program and thus one of the building 
    block to implement a stateful application over stateless http connections.

    When restoring a session, (name,value) pairs are inserted in the following
    order. If a (name,value) pair already exists for a specific name, 
    the already existing value is retained.

    1. The command-line arguments are added to the session.
    2. If a "configFile" name does not exist at this point, a (configFile,
       filename) pair compiled into the binary executable is added.
    3. If the config file exists, the (name,value) pairs in that config file
       are added to the session.
    4. Parameters passed in environment variables through the CGI invokation
       are then parsed.
    5. If a "sessionId" and a derived file exists, the (name,value) pairs 
       in that session file are added to the session.
    6. The parsed CGI parameters are added to the session.
*/
class session {
public:

    /** Source from which a session variable was initialized. 
     */
    enum sourceType {
	unknown,
	queryenv,
	cmdline,
	sessionfile,
	configfile
    };

    struct valT {
	std::string value;
	sourceType source;

	valT() {}

        valT( const std::string& v )
	    : value(v), source(unknown) {}

        valT( const std::string& v, sourceType s )
	    : value(v), source(s) {}
    };

    typedef std::map<std::string,valT> variables;

protected:

    /** definition of variables that are valid and will be reported
	in the help message. */
    boost::program_options::options_description opts;
    
    /** Unique identifier for the session */
    std::string sessionId;

    /** Initialize the session instance from the content of a file.

	The source *st* should either be *sessionfile* or *configfile*.
     */
    void load( const boost::filesystem::path& p, sourceType st );

public:
    /** name of the "command" option as a positional argument 
     */
    const char *posCmd;

    /** All applications that keep state over multiple http connections
	use a "session token". The name for such session identifier can
	be specified when the session instance is created and thus help
	administrators resolve conflicts when multiple applications run 
	on the same server. */
    const std::string sessionName;

    /* \todo workout details of auth.cc first before making private. */
    /* map of variable names to values */
    variables vars;
    
public:
    
    session( const char* posCmd,
	     const std::string& sessionName,
	     const boost::program_options::options_description& opts );

    /** Transforms the path *p* into a fully qualified URL to access
	the file through an HTTP connection. */
    url asUrl( const boost::filesystem::path& p ) const;
        
    boost::filesystem::path contributorLog() const;

    /** absolute name to the user personal directory
     */
    boost::filesystem::path userPath() const;
    
    bool exists() const { return !sessionId.empty(); }

    void filters( variables& results, sourceType source ) const;
    
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

    /** name of the document (extended to commands) requested through 
	the positional argument on the executable command line.
     */
    const std::string& doc() const {
	return valueOf(posCmd);
    }

    void id( const std::string& v ) {
	sessionId = v;
    }

    const std::string& id() const {
	return sessionId;
    }

    void insert( const std::string& name, const std::string& value,
		 sourceType source = unknown );

    /** (name,value) will be stored into the session file and thus 
	persistent accross execution. */
    void state( const std::string& name, const std::string& value );

    /** returns the directory name where state persistent accross 
	executable run are stored. */
    boost::filesystem::path stateDir() const;

    /** returns the filename where persistent (name,value) pairs accross 
	executable run are stored. */
    boost::filesystem::path stateFilePath() const;

    /** Toggle between privileged (true) and unprivileged(false) modes.
     */
    void privileged( bool v );

    /** returns true if *left* is a prefix of *right*.
     */
    bool prefix( const boost::filesystem::path& left, 
		 const boost::filesystem::path& right ) const;
    
    /** \brief Load a session from persistent storage 
     */
    void restore( int argc, char *argv[] );

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
    
    /** Store session information into persistent storage 
     */
    void store();

    /** Remove the session file 
     */
    void remove();

    /** returns the path relative to *root* of *leaf* when both *root*
	and *leaf* are absolute paths and *root* is a leading prefix 
	of *leaf*.
    */
    boost::filesystem::path 
    subdirpart( const boost::filesystem::path& root,
		  const boost::filesystem::path& leaf ) const;


    const std::string& username() const {
	return valueOf("username");
    }

    /* Returns the value of a variable as an absolute pathname. 
     */
    boost::filesystem::path valueAsPath( const std::string& name ) const;

    /** returns the value of a variable
     */
    const std::string& valueOf( const std::string& name ) const;
};


#endif
