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
    variables::const_iterator topSrc = vars.find("topSrc");
    assert( topSrc != vars.end() );
    return boost::filesystem::path(topSrc->second 
				   + std::string("/personal/") 
				   + username);
}


boost::filesystem::path session::pathname( const std::string& name ) const {
	variables::const_iterator iter = vars.find(name);
	if( iter != vars.end() ) {
		return findFile(iter->second);
	}
	return boost::filesystem::path();
}


boost::filesystem::path session::findFile( const boost::filesystem::path& name ) const {
	boost::filesystem::path fromPwd(name);
	if( boost::filesystem::exists(fromPwd) ) { 
		return fromPwd;
	}

	variables::const_iterator v;
	v = vars.find("topSrc");
	assert( v != vars.end() );
	boost::filesystem::path topSrc(v->second);

	// string_type  leaf() const;
	v = vars.find("document");
	assert( v != vars.end() );
	boost::filesystem::path document(v->second);

    boost::filesystem::path fromTopSrc = topSrc;
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
		("topSrc",value<std::string>(),"path to document top")
		("uiDir",value<std::string>(),"path to user interface elements");
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

	/* 3. load session specific information */
	session::storage = vars["topSrc"] + std::string("/personal/sessions");
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
