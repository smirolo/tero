/* Copyright (c) 2009, Sebastien Mirolo
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

   THIS SOFTWARE IS PROVIDED BY Sebastien Mirolo ''AS IS'' AND ANY
   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL Sebastien Mirolo BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include <unistd.h>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include "session.hh"
#include <uriparser/Uri.h>

/* For security, we want to store the absolute path to the config
   file into the executable binary. */
#ifndef CONFIG_FILE
#error CONFIG_FILE should be defined when compiling this file
#endif


undefVariableError::undefVariableError( const std::string& varname ) 
    : std::runtime_error(std::string("undefined variable in session ") 
			 + varname) {}

bool session::pathOptionsInit = false;

boost::program_options::options_description session::pathOptions("paths");

session::session() : id(0) {
    using namespace boost::program_options;

    if( !pathOptionsInit ) {
    pathOptions.add_options()
	("binDir",value<std::string>(),"path to outside executables")
	("buildTop",value<std::string>(),"path to build root")
	("siteTop",value<std::string>(),"path to the files published on the web site")
	("srcTop",value<std::string>(),"path to document top")
	("domainName",value<std::string>(),"domain name of the web server")
	("remoteSrcTop",value<std::string>(),"path to root of the project repositories")
	("remoteIndexFile",value<std::string>(),"path to project interdependencies")
	("themeDir",value<std::string>(),"path to user interface elements")
	("contractDb",value<std::string>(),"path to contracts database");

    pathOptionsInit = true;
    }
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


bool session::prefix( const boost::filesystem::path& left, 
		      const boost::filesystem::path& right ) const 
{
    return right.string().compare(0,left.string().size(),left.string()) == 0;
}


static std::string nullString("");
boost::filesystem::path session::storage;

url session::docAsUrl() const {
    variables::const_iterator doc = vars.find("document");
    assert( doc != vars.end() );
    return url(doc->second.substr(1));
}

const std::string& session::valueOf( const std::string& name ) const {
    variables::const_iterator iter = vars.find(name);
    if( iter == vars.end() ) {
	return nullString;
    }
    return iter->second;
}


boost::filesystem::path session::userPath() const {
    variables::const_iterator srcTop = vars.find("srcTop");
    assert( srcTop != vars.end() );
    return boost::filesystem::path(srcTop->second) 
	/ boost::filesystem::path("contrib") 
	/ username;
}

boost::filesystem::path session::contributorLog() const {
    return userPath() / boost::filesystem::path("hours");
}


boost::filesystem::path 
session::abspath( const boost::filesystem::path& relpath ) const {

    /* This is an absolute path so it is safe to return it as such. */
    if( boost::filesystem::exists(relpath) ) {
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
    return fromSrcTop;
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


void session::restore( const boost::program_options::variables_map& params )
{
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem;
    using namespace boost::program_options;

    /* If there are any QUERY_STRING, parse the parameters in it. */
    char *cQueryString = getenv("QUERY_STRING");
    if( cQueryString != NULL ) {
	int err = 0;
	int itemCount = 0;
	UriQueryListA *queryList = NULL;
	if( (err = uriDissectQueryMallocA(&queryList,
					  &itemCount,
					  cQueryString,
					  &cQueryString[strlen(cQueryString)])) 
	    == URI_SUCCESS ) {	
	    for( UriQueryListStructA *item = queryList;
		 item != NULL; item = item->next ) {
		query[item->key] = item->value;
		vars[item->key] = item->value;
	    }
	    uriFreeQueryListA(queryList);
	}
    }

    /* 1. initialize more configuration from the script input */
    for( variables_map::const_iterator param = params.begin(); 
	 param != params.end(); ++param ) {
	vars[param->first] = param->second.as<std::string>();
    }

    /* 2. load global information from the configuration file on the server */
    variables_map configVars;
    
    std::string config(CONFIG_FILE);
    boost::program_options::variables_map::const_iterator 
	configParam = params.find("config");
    if( configParam != params.end() ) {
	config = configParam->second.as<std::string>();
    }

    ifstream istr(config);
    if( istr.fail() ) {
	boost::throw_exception(basic_filesystem_error<path>(std::string("file not found"),
							    config, 
							    boost::system::error_code()));
    }
    
    boost::program_options::store(parse_config_file(istr,pathOptions,true),
				  configVars);
	
    for( variables_map::const_iterator param = configVars.begin(); 
	 param != configVars.end(); ++param ) {
	if( vars.find(param->first) == vars.end() ) {
	    vars[param->first] = param->second.as<std::string>();
	}
    }
	
    /* If no document is present, set document 
       as view in order to catch default dispatch 
       clause. */
    if( params["document"].empty() ) {
	vars["document"] = vars["view"];
    }
    /* Append a trailing '/' if the document is a directory
       to match Apache's rewrite rules. */    
    std::string document = abspath(vars["document"]).string();
    if( boost::filesystem::is_directory(document) 
	&& (document.size() == 0 
	    || document[document.size() - 1] != '/') ) {
	vars["document"] = document + '/';
    }

    session::variables::const_iterator v = vars.find("username");
    if( v != vars.end() ) {
	username = v->second;
    }
    
    /* 3. load session specific information */
    session::storage = vars["srcTop"] + std::string("/personal/sessions");
    if( vars.find("session") != vars.end() ) {
	id = atol(vars["session"].c_str());
	
	static boost::regex format("(.*):(.*)");
	bool found = false;
	
	if( boost::filesystem::exists(session::storage) ) {
	    /* 1. open session file */
	    ifstream sessions(session::storage);
	    if( sessions.fail() ) {
		boost::throw_exception(basic_filesystem_error<path>(std::string("error opening file"),
								    session::storage, 
								    error_code()));
	    }
	    
	    /* 2. search for id */
	    while( !sessions.eof() ) {
		smatch m;
		std::string line;
		getline(sessions,line);
		if( regex_search(line,m,format) ) {
		    if( id == atol(m[1].str().c_str()) ) {
			found = true;
			username = m[2].str();
			break;
		    }
		}
	    }
	    sessions.close();
	}
    }

    /* set the username to the value of LOGNAME in case no information
       can be retrieved for the session. It helps with keeping track
       of time spent with a shell command line. */
    if( username.empty() ) {
	char *logName = getenv("LOGNAME");
	if( logName != NULL ) {
	    username = logName;
	}
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
    if( id != 0 ) {
	ostr << "session " << id << " for " << username << std::endl;
    } else {
	ostr << "no session id" << std::endl;
    }
    for( variables::const_iterator var = vars.begin(); 
	 var != vars.end(); ++var ) {
	ostr << var->first << ": " << var->second << std::endl;
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

	/* \todo lock session file */
	/* 1. open session file to append */
    ofstream sessions(session::storage,std::ios::app);
    if( sessions.fail() ) {
		boost::throw_exception(basic_filesystem_error<path>(
							std::string("error opening file"),
							session::storage, 
							error_code()));
    }
	/* 2. write session information */
	sessions << id << ':' << username << std::endl;
	sessions.close();
	/* \todo unlock session file */
}


void session::start() {
    using namespace boost::filesystem;

    variables::const_iterator iter = vars.find("message");
    if( iter == vars.end() ) {
	std::stringstream buffer;
	buffer << "touch " << contributorLog();
	system(buffer.str().c_str());
    } else {
	ofstream file(contributorLog(),std::ios_base::app);
	if( file.fail() ) {
	    boost::throw_exception(basic_filesystem_error<path>(
				 std::string("error opening file"),
				 contributorLog(), 
				 boost::system::error_code()));
	}
	file << iter->second << ':' << std::endl;
	file.close();
    }
}


boost::posix_time::time_duration session::stop() {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;

    /* This local adjustor depends on the machine TZ settings
       and that seems the right thing to do in this context. */
    typedef boost::date_time::c_local_adjustor<ptime> local_adj;
    ptime start = local_adj::utc_to_local(
		       from_time_t(last_write_time(contributorLog())));
    ptime stop = second_clock::local_time();

    ofstream file(contributorLog(),std::ios_base::app);
    if( file.fail() ) {
	boost::throw_exception(basic_filesystem_error<path>(
				 std::string("error opening file"),
				 contributorLog(), 
				 error_code()));
    }
    time_duration aggregate = stop - start;
    file << start << ' ' << stop << ' ' << valueOf("message") << std::endl;
    file.close();

    return aggregate;
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
    return abspath(boost::filesystem::path(iter->second));
}
