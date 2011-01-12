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


#include <sys/types.h>
#include <unistd.h>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "session.hh"
#include <uriparser/Uri.h>
#include "markup.hh"

/** Session manager

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

/* For security, we want to store the absolute path to the config
   file into the executable binary. */
#ifndef CONFIG_FILE
#error CONFIG_FILE should be defined when compiling this file
#endif
#ifndef SESSION_DIR
#error SESSION_DIR should be defined when compiling this file
#endif


undefVariableError::undefVariableError( const std::string& varname ) 
    : std::runtime_error(std::string("undefined variable in session ") 
			 + varname) {}


void session::load( const boost::program_options::options_description& opts,
		    const boost::filesystem::path& p, sourceType st )
{
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem;
    using namespace boost::program_options;

    if( boost::filesystem::exists(p) ) {
	variables_map params;
	ifstream istr(p);
	if( istr.fail() ) {
	    boost::throw_exception(
		basic_filesystem_error<path>(std::string("file not found"),
					     p, 
					     boost::system::error_code()));
	}

	boost::program_options::store(parse_config_file(istr,opts,true),
				      params);
	
	for( variables_map::const_iterator param = params.begin(); 
	     param != params.end(); ++param ) {
	    if( vars.find(param->first) == vars.end() ) {
		insert(param->first,param->second.as<std::string>(),st);
	    }
	}
    }
}

boost::filesystem::path session::stateDir() const {
    std::string sessionDir = valueOf("sessionDir");
    if( sessionDir.empty() ) {
	sessionDir = SESSION_DIR;
    }
    return boost::filesystem::path(sessionDir);
}


boost::filesystem::path session::stateFilePath() const
{
    return stateDir() / (sessionId + ".session");
}


url session::asUrl( const boost::filesystem::path& p ) const
{
    boost::filesystem::path name = p;
    boost::filesystem::path siteTop = valueOf("siteTop");
    boost::filesystem::path srcTop = valueOf("srcTop");
    if( prefix(srcTop,p) ) {
	name = boost::filesystem::path("/") / subdirpart(srcTop,p);
    } else if( prefix(siteTop,p) ) {
	name = boost::filesystem::path("/") / subdirpart(siteTop,p);
    }
    return url("","",name);
}


void session::filters( variables& results, sourceType source ) const
{
    for( variables::const_iterator p = vars.begin();
	 p != vars.end(); ++p ) {
	if( p->second.source == source ) {
	    results[p->first] = p->second;
	}
    }
}


void session::insert( const std::string& name, const std::string& value,
		      sourceType source ) 
{
    if( vars.find(name) == vars.end() ) { 
	vars[name] = valT(value,source);
    }
}


void session::state( const std::string& name, const std::string& value ) 
{    
    vars[name] = valT(value,sessionfile);
    if( !exists() ) {
	using namespace boost::posix_time;

	std::stringstream s;
	s << boost::uuids::random_generator()();
	sessionId = s.str();

	using namespace boost::filesystem;
	variables::iterator iter = vars.find("message");
	if( iter != vars.end() ) iter->second.source = sessionfile;

	s.str("");
	s << second_clock::local_time();
	vars["startTime"] = valT(s.str(),sessionfile);
    }
}


bool session::prefix( const boost::filesystem::path& left, 
		      const boost::filesystem::path& right ) const 
{
    return right.string().compare(0,left.string().size(),left.string()) == 0;
}


void session::privileged( bool v ) {
    /* change uid. */
    /* \todo Setting the setuid flag. 
       The first digit selects attributes
           set user ID (4)
           set group ID (2) 
           sticky (1) 

       chmod 4755 semilla
       sudo chown root semilla
*/
    uid_t realId = getuid();
    uid_t effectiveId = geteuid();
#if 0
    std::cerr << "!!! real_uid=" << realId << ", effective_uid="
	      << effectiveId << std::endl;
#endif
    uid_t newId = v ? 0 : realId;
    if( setuid(newId) < 0 ) {
	std::cerr << "error: setuid to zero: " 
		  << ((errno == EINVAL) ? "invalid" : "eperm") << std::endl;	
    }
}


static std::string nullString("");


const std::string& session::valueOf( const std::string& name ) const {
    variables::const_iterator iter = vars.find(name);
    if( iter == vars.end() ) {
	return nullString;
    }
    return iter->second.value;
}


boost::filesystem::path session::userPath() const {
    variables::const_iterator srcTop = vars.find("srcTop");
    assert( srcTop != vars.end() );
    return boost::filesystem::path(srcTop->second.value) 
	/ boost::filesystem::path("contrib") 
	/ username();
}

boost::filesystem::path session::contributorLog() const {
    return userPath() / boost::filesystem::path("hours");
}


boost::filesystem::path 
session::abspath( const boost::filesystem::path& relpath ) const {

    /* This is an absolute path so it is safe to return it as such. */
    if( relpath.string()[0] == '/' && boost::filesystem::exists(relpath) ) {
	return relpath;
    }

    /* First we try to access the file from cwd. */
    boost::filesystem::path fromCwd 
	= boost::filesystem::current_path() / relpath;
    if( !relpath.is_complete() && boost::filesystem::exists(fromCwd) ) { 	
	return fromCwd;
    }	

    /* Second we try to access the file as a relative pathname 
       from siteTop. */
    boost::filesystem::path fromSiteTop(valueOf("siteTop"));
    fromSiteTop /= relpath;
    if( boost::filesystem::exists(fromSiteTop) ) {        
	return fromSiteTop;
    }

    /* Third we try to access the file as a relative pathname 
       from srcTop. */	
    boost::filesystem::path fromSrcTop(valueOf("srcTop"));
    fromSrcTop /= relpath;
    if( boost::filesystem::exists(fromSrcTop) ) {        
	return fromSrcTop;
    }	
    
    /* We used to throw an exception at this point. That does
       not sit very well with dispatch::fetch() because
       the value of a "document" might not be an actual file.
       Since the relative path of "document" will initialy 
       be derived from the web server request uri, it is 
       the most appropriate to return the path from siteTop
       in case the document could not be found.
       !!! We have to return from srcTop because that is how
       the website is configured for rss feeds. */
    return fromSiteTop;
}


boost::filesystem::path 
session::build( const boost::filesystem::path& p ) const
{
    boost::filesystem::path buildTop = valueOf("buildTop");
    boost::filesystem::path srcTop = valueOf("srcTop");
    if( prefix(srcTop,p) ) {
	return buildTop / p.string().substr(srcTop.string().size() + 1);
    }
    return p;
}


void session::restore( int argc, char *argv[],
		       const boost::program_options::options_description& o )
{
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem;
    using namespace boost::program_options;

    boost::program_options::options_description opts(o);
    opts.add_options()
	("config",value<std::string>(),"path to the configuration file (defaults to "CONFIG_FILE")")
	("sessionDir",value<std::string>(),"directory where session files are stored (defaults to "SESSION_DIR")")
	(sessionName.c_str(),value<std::string>(),"name of the session id variable (or cookie)")
	("username",value<std::string>(),"username")
	(posCmd,value<std::string>(),posCmd)

	("buildTop",value<std::string>(),"path to build root")
	("srcTop",value<std::string>(),"path to document top")
	("siteTop",value<std::string>(),"path to the files published on the web site")
	/* \todo only in payments? */
	("domainName",value<std::string>(),"domain name of the web server")

	("message,m",value<std::string>(),"Message inserted in the contributor's hours log")
	("startTime",value<std::string>(),"start time for the session");


    positional_options_description pd;     
    pd.add(posCmd, 1);

    /* 1. The command-line arguments are added to the session. */
    {
	variables_map params;
	command_line_parser parser(argc, argv);
	parser.options(opts);
	parser.positional(pd);    
	boost::program_options::store(parser.run(),params);  
	for( variables_map::const_iterator param = params.begin(); 
	     param != params.end(); ++param ) {	    
	    /* Without the strip(), there is a ' ' appended to the command
	       line on Linux apache2. */
	    std::string s = strip(param->second.as<std::string>());
	    if( !s.empty() ) {
		insert(param->first,s,cmdline);
	    }
	}
    }

    /* 2. If a "config" file name does not exist at this point, a (config,
       filename) pair compiled into the binary executable is added. */
    std::string config(CONFIG_FILE);
    variables::const_iterator cfg = vars.find("config");
    if( cfg != vars.end() ) {
	config = cfg->second.value;
    }

    /* 3. If the config file exists, the (name,value) pairs in that config file
       are added to the session. */
    load(opts,config,configfile);

    /* 4. Parameters passed in environment variables through the CGI invokation
       are then parsed. */
    variables_map cgiParams;
    cgi_parser parser;
    parser.options(opts);
    parser.positional(pd);
    boost::program_options::store(parser.run(),cgiParams);

    /* 5. If a "sessionName" and a derived file exists, the (name,value) pairs 
       in that session file are added to the session. */
    cgi_parser::querySet::const_iterator sid 
	= parser.query.find(sessionName);
    if( sid != parser.query.end() ) {
	sessionId = sid->second;
    }
    if( exists() ) {
	load(opts,stateFilePath(),sessionfile);
    }

    /* 6. The parsed CGI parameters are added to the session. */
    for( std::map<std::string,std::string>::const_iterator 
	     p = parser.query.begin(); p != parser.query.end(); ++p ) {    
	insert(p->first,p->second,queryenv);
    }
    
    /* set the username to the value of LOGNAME in case no information
       can be retrieved for the session. It helps with keeping track
       of time spent with a shell command line. */
    session::variables::const_iterator v = vars.find("username");
    if( v == vars.end() ) {
	char *logName = getenv("LOGNAME");
	if( logName != NULL ) {
	    insert("username",logName);
	}
    }


    /* Append a trailing '/' if the document is a directory
       to match Apache's rewrite rules. */    
    std::string document = abspath(valueOf(posCmd)).string();
    if( boost::filesystem::is_directory(document) 
	&& (document.size() == 0 
	    || document[document.size() - 1] != '/') ) {
	insert(posCmd,document + '/');
    }
}


boost::filesystem::path 
session::root( const boost::filesystem::path& leaf,
	       const boost::filesystem::path& trigger ) const
{
    using namespace boost::filesystem;
    std::string srcTop = valueOf("srcTop");
    path dirname = leaf;
    if( !is_directory(dirname) ) {
	dirname = dirname.parent_path();
    }
#if 0
    std::cerr << "root(" << leaf << "," << trigger << "), start=" << dirname;
#endif
    bool foundProject = boost::filesystem::exists(dirname.string() / trigger);
    while( !foundProject && (dirname.string() != srcTop) ) {
	dirname.remove_leaf();
	if( dirname.string().empty() ) {
	    boost::throw_exception(basic_filesystem_error<path>(
			std::string("no trigger from path up"),
			leaf, 
			boost::system::error_code())); 
	}
	foundProject = boost::filesystem::exists(dirname.string() / trigger);
    }
    return foundProject ? dirname : path("");
}


void session::show( std::ostream& ostr ) {

    static const char *sourceTypeNames[] = {
	"unknown",
	"queryenv",
	"cmdline",
	"sessionfile",
	"configfile"
    };

    if( exists() ) {
	ostr << "session " << sessionId << " for " << username() << std::endl;
    } else {
	ostr << "no session id" << std::endl;
    }
    for( variables::const_iterator var = vars.begin(); 
	 var != vars.end(); ++var ) {
	ostr << var->first << ": " << var->second.value 
	     << " [" << sourceTypeNames[var->second.source] << "]" << std::endl;
    }
}


boost::filesystem::path 
session::src( const boost::filesystem::path& p ) const
{
    boost::filesystem::path buildTop = valueOf("buildTop");
    boost::filesystem::path srcTop = valueOf("srcTop");
    if( prefix(buildTop,p) ) {
	return srcTop / p.string().substr(buildTop.string().size() + 1);
    }
    return p;
}


boost::filesystem::path 
session::srcDir( const boost::filesystem::path& p ) const
{
    boost::filesystem::path srcTop = valueOf("srcTop");    
    return srcTop / p;
}


void session::store() {
    using namespace boost::system;
    using namespace boost::filesystem;

    /* 1. open session file based on session id. */
    ofstream sessions(stateFilePath());
    if( sessions.fail() ) {
	boost::throw_exception(basic_filesystem_error<path>(
			       std::string("error opening file"),
			       stateFilePath(), 
			       error_code()));
    }
    /* 2. write session information */
    for( variables::const_iterator v = vars.begin(); v != vars.end(); ++v ) {
	if( v->second.source == sessionfile ) {
	    sessions << v->first << '=' << v->second.value << std::endl; 
	}
    }
    sessions.close();
}


void session::remove() {
    boost::filesystem::remove(stateFilePath());
    sessionId = "";
}


boost::filesystem::path 
session::subdirpart( const boost::filesystem::path& root,
		     const boost::filesystem::path& leaf ) const {
    return leaf.string().substr(root.string().size() 
	+ (root.string()[root.string().size() - 1] != '/' ? 1 : 0));
}


boost::filesystem::path 
session::valueAsPath( const std::string& name ) const {
    variables::const_iterator iter = vars.find(name);
    if( iter == vars.end() ) {
	boost::throw_exception(undefVariableError(name));
    }
    return abspath(boost::filesystem::path(iter->second.value));
}
