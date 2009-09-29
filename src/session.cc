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
#include "session.hh"

static std::string nullString("");
boost::filesystem::path session::storage;

std::string session::docAsUrl() const {
#if 0
	variables::const_iterator topUrl = vars.find("topUrl");
	assert( topUrl != vars.end() );
#endif
	variables::const_iterator doc = vars.find("document");
	assert( doc != vars.end() );
	return doc->second.substr(1);
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
    return boost::filesystem::path(srcTop->second 
				   + std::string("/personal/") 
				   + username);
}


boost::filesystem::path 
session::abspath( const std::string& name ) const {
    using namespace boost::filesystem;

    variables::const_iterator iter = vars.find(name);
    if( iter != vars.end() ) {
	path relpath(iter->second);
	path fromBuildTop(valueOf("buildTop"));
	fromBuildTop /= relpath;
	if( boost::filesystem::exists(fromBuildTop) ) { 
	    return fromBuildTop;
	}
	std::cerr << "not found: " << fromBuildTop << std::endl;

	path fromSrcTop(valueOf("srcTop"));
	fromSrcTop /= relpath;
	if( boost::filesystem::exists(fromSrcTop) ) { 
	    return fromSrcTop;
	}	
	std::cerr << "not found: " << fromSrcTop << std::endl;
    }
    /* We used to throw an exception at this point. That does
       not sit very well with dispatch::fetch() because the value
       of a "document" might not be an actual file. */
    return boost::filesystem::path();
}


boost::filesystem::path 
session::findFile( const boost::filesystem::path& name ) const {
    /** \todo using std::cerr statements here yelds to recursive
	call to the decorator. That should not happen. Only reasonable
	explanation if that cout and cerr buffers are shared...
     */
    boost::filesystem::path fromPwd(name);
    if( boost::filesystem::exists(fromPwd) ) { 
	return fromPwd;
    }

    variables::const_iterator v;
    v = vars.find("srcTop");
    assert( v != vars.end() );
    boost::filesystem::path srcTop(v->second);

    // string_type  leaf() const;
    v = vars.find("document");
    assert( v != vars.end() );
    boost::filesystem::path document(v->second);

    boost::filesystem::path fromTopSrc = srcTop;
    fromTopSrc /= document.branch_path();
    fromTopSrc /= name;

    if( boost::filesystem::exists(fromTopSrc) ) { 
	return fromTopSrc;
    }
    return boost::filesystem::path();
}


#ifndef CONFIG_FILE
#error CONFIG_FILE should be defined when compiling this file
#endif


void session::restore( const boost::program_options::variables_map& params )
{
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem;
    using namespace boost::program_options;

    /* 1. load global information from the configuration file on the server */
    variables_map configVars;
    ifstream istr(CONFIG_FILE);
    if( istr.fail() ) {
	boost::throw_exception(basic_filesystem_error<path>(std::string("file not found"),
							    CONFIG_FILE, 
							    boost::system::error_code()));
    }
    
    /* parse command line arguments */
    options_description	opts;
    opts.add_options()
		("binDir",value<std::string>(),"path to outside executables")
		("buildTop",value<std::string>(),"path to build root")
		("cacheTop",value<std::string>(),"path to packages repository")
		("srcTop",value<std::string>(),"path to document top")
		("themeDir",value<std::string>(),"path to user interface elements");
    boost::program_options::store(parse_config_file(istr,opts,true),configVars);
	
    for( variables_map::const_iterator param = configVars.begin(); 
	 param != configVars.end(); ++param ) {
		vars[param->first] = param->second.as<std::string>();
    }
	
    /* 2. initialize more configuration from the script input */
    for( variables_map::const_iterator param = params.begin(); 
	 param != params.end(); ++param ) {
	vars[param->first] = param->second.as<std::string>();
    }
    /* If no document is present, set document 
       as view in order to catch default dispatch 
       clause. */
    if( params["document"].empty() ) {
	vars["document"] = vars["view"];
    }

    session::variables::const_iterator v = vars.find("username");
    if( v != vars.end() ) {
	username = v->second;
    }
    
    /* 3. load session specific information */
    session::storage = vars["srcTop"] + std::string("/personal/sessions");
    if( vars.find("session") != vars.end() ) {
	id = atol(vars["session"].c_str());
	
	std::cerr << "session::restore..." << std::endl;     
	static boost::regex format("(.*):(.*)");
	bool found = false;
	
	std::cerr << "format done..." << std::endl;     
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
