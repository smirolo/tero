/* Copyright (c) 2009-2013, Fortylines LLC
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
#include <boost/property_tree/detail/rapidxml.hpp>
#include "webserve.hh"
#include "slice.hh"

#if BOOST_VERSION > 104600
#define RAPIDXML     boost::property_tree::detail::rapidxml
#else
#define RAPIDXML     rapidxml
#endif

/**
   Session Manager.

   A session manager is responsible to keep application state
   over http state-less transactions.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

namespace tero {

/** This error is thrown when we are trying to retrieve the value
    of a variable from the current session and that variable does
    not exist in the variables map.
*/
class undefVariableError : public std::runtime_error {
public:
    explicit undefVariableError( const std::string& varname );
};

/** This error is thrown when deriving prefix directories.
*/
class notLeadingPrefixError : public std::runtime_error {
public:
    explicit notLeadingPrefixError(
        const std::string& prefix, const std::string& leaf );
};


// forward declaration
class postFilter;
class session;

class sessionVariable {
public:
    const char* const name;

protected:
    const char *descr;
    boost::program_options::option_description opt;

public:
    sessionVariable( const char *n, const char *d )
        : name(n), descr(d),
          opt(name,boost::program_options::value<std::string>(),descr) {}

    boost::shared_ptr<boost::program_options::option_description>
    option();

    std::string value( const session& s ) const;
};


class intVariable : public sessionVariable {
public:
    intVariable( const char *name, const char *descr )
        : sessionVariable(name,descr) {}

    int value( const session& s ) const;
};


class pathVariable : public sessionVariable {
public:
    pathVariable( const char *name, const char *descr )
        : sessionVariable(name,descr) {}

    boost::filesystem::path value( const session& s ) const;
};


class timeVariable : public sessionVariable {
public:
    timeVariable( const char *name, const char *descr )
        : sessionVariable(name,descr) {}

    boost::posix_time::ptime value( const session& s ) const;
};


class urlVariable : public sessionVariable {
public:
    urlVariable( const char *name, const char *descr )
        : sessionVariable(name,descr) {}

    url value( const session& s ) const;
};

/** name of the document (extended to commands) requested through
    the positional argument on the executable command line.
*/
extern urlVariable document;
extern urlVariable domainName;
extern pathVariable siteTop;
extern pathVariable cacheTop;
extern timeVariable startTime;

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

    /* For security, we want to store the absolute path to the config
       file into the executable binary. */
    static const char* configFile;
    static const char* sessionDir;


    /** Source from which a session variable was initialized.

        The order of this enum is used by *insert* to replace
        pre-existing values or not.
     */
    enum sourceType {
        cmdline,
        configfile,
        sessionfile,
        queryenv,
        unknown
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

    typedef std::vector<url> inputsType;
    inputsType inputs;

protected:

    typedef std::map<boost::filesystem::path,slice<char> > textMap;
    typedef std::map<boost::filesystem::path,
                     RAPIDXML::xml_document<>* > xmlMap;

    /** Set of all text files currently cached in memory.
     */
    textMap texts;

    xmlMap xmls;

    bool ascgi;

    /** Unique identifier for the session */
    std::string sessionId;

    /** IP address of the client making the request */
    std::string remoteAddr;

    /** Initialize the session instance from the content of a file.

        The source *st* should either be *sessionfile* or *configfile*.
     */
    void load( const boost::program_options::options_description& opts,
        const boost::filesystem::path& p, sourceType st );

protected:

    /* \todo workout details of auth.cc first before making private. */
    /* map of variable names to values */
    variables vars;

    std::ostream *ostr;

    friend class sessionVariable;

    /* \todo protected comments.cc use of "href" */
public:
    boost::program_options::options_description opts;
    boost::program_options::options_description visible;

    /** returns the value of a variable
     */
    const std::string& valueOf( const std::string& name ) const;


public:
    /** All applications that keep state over multiple http connections
        use a "session token". The name for such session identifier can
        be specified when the session instance is created and thus help
        administrators resolve conflicts when multiple applications run
        on the same server. */
    static std::string sessionName;

    /** number of errors encountered while generating a page. */
    unsigned int nErrs;

    postFilter *feeds;

    session( const std::string& sn, std::ostream& o );

    /** Transforms the path *p* into a fully qualified URL to access
        the file through an HTTP connection. */
    url asUrl( const boost::filesystem::path& p ) const;

    /** Transforms a url into an absolute url including protocol and domain */
    url asAbsUrl( const url& u, const boost::filesystem::path& base ) const;

    bool errors() const { return nErrs > 0; }

    bool exists() const { return !sessionId.empty(); }

    void filters( variables& results, sourceType source ) const;

    /** Throws an exception if *pathname* does not exists.
     */
    void check( const boost::filesystem::path& pathname ) const;

    /** Append to a file.

        This function throws an exception if there is any error.
    */
    void appendfile( boost::filesystem::ofstream& strm,
        const boost::filesystem::path& pathname );

    /** Create a file (override it if it already exists).

        This function throws an exception if there is any error.
    */
    void createfile( boost::filesystem::ofstream& strm,
        const boost::filesystem::path& pathname );

    void loadsession( const std::string& id );

    /** Load and cache a text file in memory. Two back-to-back calls
        will return the same null-terminated buffer.
     */
    slice<char> loadtext( const boost::filesystem::path& p );

    RAPIDXML::xml_document<> *loadxml( const boost::filesystem::path& p );

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

    boost::filesystem::path
    abspath( const url& name ) const;

    session::variables::const_iterator find( const std::string name ) const {
        return vars.find(name);
    }

    bool found( session::variables::const_iterator p ) const {
        return p != vars.end();
    }

    /* \todo used in deauth. */
    void id( const std::string& v ) {
        sessionId = v;
    }

    const std::string& id() const {
        return sessionId;
    }

    const std::string& client() const {
        return remoteAddr;
    }

    void insert( const std::string& name, const std::string& value,
        sourceType source = unknown );

    std::ostream& out() { return *ostr; }

    std::ostream& out( std::ostream& o ) {
        std::ostream* prev = ostr;
        ostr = &o;
        return *prev;
    }

    /** Remove all (name,value) pairs which were not set at the time
        the session was restored from command-line, session file, etc.
        Technically this functionality could be implemented by clearing
        the set of variables and restoring the session passing the same
        config arguments. This special-purpose implementation should
        be much more efficient.
    */
    void reset();

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

    /** returns the longest prefix of path *p* which is also a directory
        that exists on the filesystem. */
    boost::filesystem::path
    prefixdir( const boost::filesystem::path& p ) const;

    /** \brief Load a session from persistent storage
     */
    void restore( int argc, char *argv[] );

    /* Look for a relative *trigger* from *leaf* to *siteTop*
       and return the stem such that stem / *trigger* is the absolute
       url to the trigger.

       If *trigger* cannot be found, the method returns an empty pathname.
    */
    url root( const url& leaf,
        const boost::filesystem::path& trigger, bool keepTrigger ) const;

    /* look for a relative pathname *trigger* from *leaf* to the root
       of the filesystem and return the stem such that stem / *trigger*
       is the absolute pathname to the trigger.
       If *trigger* cannot be found, the method returns an empty path.
    */
    boost::filesystem::path root( const boost::filesystem::path& leaf,
        const boost::filesystem::path& trigger,
        bool keepTrigger = false ) const;

    std::string root() const {
        return std::string("");
    }

    bool runAsCGI() const { return ascgi; }

    /** \brief Display debug information for the session
     */
    void show( std::ostream& ostr ) const;

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

    /* Returns the value of a variable as an absolute pathname.
     */
    boost::filesystem::path valueAsPath( const std::string& name ) const;

    /** returns a cached url href from an url *href*. The directory portion
        of the *href* remains unchanged. A relative url will be converted
        to an equivalent cached url. An absolute url will be converted to
        an equivalent cached url.
     */
    url cacheName( const url& href ) const;

    /** returns an absolute path from a relative or absolute url *href*.
     */
    boost::filesystem::path absCacheName( const url& link ) const;

};

}

#endif
