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


#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/regex.hpp>
#include "session.hh"
#include <uriparser/Uri.h>
#include "markup.hh"
#include "revsys.hh"

/** Session manager

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

void yoyo()
{
    assert(false);
}

namespace {

void checkInSiteTop( const session& s, const boost::filesystem::path& p ) {
    using namespace boost::system::errc;
    if( !s.prefix(siteTop.value(s),p) ) {
        std::cerr << "error: " << siteTop.value(s)
                  << " is not a leading prefix of "
                  << p << std::endl;
        yoyo();
        boost::throw_exception(notLeadingPrefixError(
                siteTop.value(s).string(),
                p.string()));
    }
}

} // anonymous

urlVariable document("view","document to be displayed");

pathVariable siteTop("siteTop","root directory that contains the web site");

pathVariable cacheTop("cacheTop","root directory that contains the cached pages");

/* \todo Unless we communicate a callback to another site,
         we should be clean of requiring domainName...
         not true: We need domain name to display fully qualified src
         repository urls.
*/
urlVariable domainName("domainName","domain name of the web server");

timeVariable startTime("startTime","start time for the session");

notLeadingPrefixError::notLeadingPrefixError(
    const std::string& prefix, const std::string& leaf )
    : std::runtime_error(prefix
        + std::string(" is not a leading prefix of ") + leaf) {
}


undefVariableError::undefVariableError( const std::string& varname )
    : std::runtime_error(std::string("undefined variable in session ")
        + varname) {}


boost::shared_ptr<boost::program_options::option_description>
sessionVariable::option() {
    return boost::shared_ptr<boost::program_options::option_description>
        (new boost::program_options::option_description(opt));
}


std::string sessionVariable::value( const session& s ) const
{
    std::string val = s.valueOf(name);
    if( val.size() > 0 && val[0] == '"' ) return val.substr(1,val.size()-2);
    return val;
}


int intVariable::value( const session& s ) const
{
    return atoi(s.valueOf(name).c_str());
}


boost::filesystem::path pathVariable::value( const session& s ) const
{
    using namespace boost::filesystem;
    using namespace boost::system::errc;

    /* This code creates a lot of trouble with the matching dispatch patterns
       as well as generating dynamic pages when *document* is a pathVariable
       instead of an urlVariable. */
    path pathname = sessionVariable::value(s);
    if( pathname.is_complete() ) {
        pathname = s.abspath(pathname);
        if( !boost::filesystem::exists(pathname) ) {
            boost::throw_exception(boost::system::system_error(
                make_error_code(no_such_file_or_directory),pathname.string()));
        }
    } else {
        std::stringstream msg;
        msg << pathname << " is not an absolute path for "
            << sessionVariable::name;
        boost::throw_exception(boost::system::system_error(
            make_error_code(no_such_file_or_directory),msg.str()));
    }
    return pathname;
}


boost::posix_time::ptime timeVariable::value( const session& s ) const
{
    std::stringstream is(sessionVariable::value(s));
    boost::posix_time::ptime t;
    is >> t;
    return t;
}


url urlVariable::value( const session& s ) const
{
    return url(sessionVariable::value(s));
}


std::string session::sessionName;

session::session( const std::string& sn,
    std::ostream& o )
	: sessionId(""), ostr(&o), nErrs(0), feeds(NULL)
{
    sessionName = sn;

    using namespace boost::program_options;

    options_description genOptions("general");
    genOptions.add_options()
        ("help","produce help message");
    opts.add(genOptions);
    visible.add(genOptions);

    options_description localOptions("session");
    localOptions.add_options()
        ("config",value<std::string>(),(std::string("path to the configuration file (defaults to ") + configFile + std::string(")")).c_str())
        ("sessionDir",value<std::string>(),(std::string("directory where session files are stored (defaults to ") + sessionDir + std::string(")")).c_str())
        (sessionName.c_str(),value<std::string>(),"name of the session id variable (or cookie)");
    localOptions.add(domainName.option());
    localOptions.add(siteTop.option());
    localOptions.add(cacheTop.option());
    opts.add(localOptions);
    visible.add(localOptions);

    options_description hiddenOptions("hidden");
    hiddenOptions.add(startTime.option());
    hiddenOptions.add_options()
        (document.name,value<std::vector<boost::filesystem::path> >(),"input files");
    opts.add(hiddenOptions);
}


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
                system_error(error_code(),
                    std::string("file not found")+ p.string()));
        }

        /* \todo Note: it seems we cannot set the style passed to parse_config_file,
           thus even if we mask out *allow_guessing* style to the command line
           parser, we will still trigger an ambiguous option error here for all
           options starting with the same prefix.
           We also get an ambiguous option when two options have the same name. */
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


void session::loadsession( const std::string& id )
{
    sessionId = id;
    load(opts,stateFilePath(),sessionfile);
}


void session::check( const boost::filesystem::path& pathname ) const
{
    if( !boost::filesystem::exists(pathname)
        || !is_regular_file(pathname) ) {
        using namespace boost::system::errc;
        using namespace boost::filesystem;

        boost::throw_exception(boost::system::system_error(
                make_error_code(no_such_file_or_directory),pathname.string()));
    }
}

void session::appendfile( boost::filesystem::ofstream& strm,
    const boost::filesystem::path& pathname )
{
    using namespace boost::system;
    using namespace boost::filesystem;

    if( !boost::filesystem::exists(pathname) ) {
        createfile(strm,pathname);
    } else {
        using namespace boost::system::errc;

        strm.open(pathname,std::ios_base::out | std::ios_base::app);
        if( strm.fail() ) {
            boost::throw_exception(boost::system::system_error(
                make_error_code(no_such_file_or_directory),pathname.string()));
        }
    }
}


void session::createfile( boost::filesystem::ofstream& strm,
    const boost::filesystem::path& pathname )
{
    using namespace boost::system;
    using namespace boost::filesystem;

    create_directories(pathname.parent_path());
    strm.open(pathname,std::ios_base::out | std::ios_base::trunc);
    if( strm.fail() ) {
        using namespace boost::system::errc;
        boost::throw_exception(boost::system::system_error(
                make_error_code(no_such_file_or_directory),pathname.string()));
    }
}


slice<char> session::loadtext( const boost::filesystem::path& pathname )
{
    using namespace boost::filesystem;
    using namespace boost::system::errc;

    textMap::const_iterator found = texts.find(pathname);
    if( found != texts.end() ) {
        return found->second;
    }

    revisionsys *rev = revisionsys::findRev(*this, pathname);
    if( rev != NULL ) {
        boost::filesystem::path
            localpathname = rev->relative(pathname);
        texts[pathname] = rev->loadtext(localpathname, "HEAD");
        return texts[pathname];
    }
    return slice<char>();
}


RAPIDXML::xml_document<>*
session::loadxml( const boost::filesystem::path& p )
{
    xmlMap::const_iterator found = xmls.find(p);
    if( found != xmls.end() ) {
        return found->second;
    }
    slice<char> buffer = loadtext(p);
#if 0
    std::cerr << "loadxml(" << p << "):" << std::endl;
    std::cerr << "\"" << buffer.begin() << "\"" << std::endl;
#endif
    RAPIDXML::xml_document<> *doc = new RAPIDXML::xml_document<>();
    try {
        doc->parse<0>(buffer.begin());
        xmls[p] = doc;
    } catch( std::exception& e ) {
        delete doc;
        doc = NULL;
        /* We would rather return NULL here instead of rethrowing the exception
           because part of the callback fetch that requested the XML must
           write an alternate message. */
    }
    return doc;
}


boost::filesystem::path session::stateDir() const {
    std::string s = valueOf("sessionDir");
    if( s.empty() ) {
        s = sessionDir;
    }
    return boost::filesystem::path(s);
}


boost::filesystem::path session::stateFilePath() const
{
    return stateDir() / (sessionId + ".session");
}


url session::asUrl( const boost::filesystem::path& p ) const
{
    checkInSiteTop(*this,p);

    boost::filesystem::path name = boost::filesystem::path("/")
        / subdirpart(siteTop.value(*this),p);
    return url("","",name);
}


url session::asAbsUrl( const url& u, const boost::filesystem::path& base ) const
{
    url result = u;
    if( u.protocol.empty() ) {
        result.protocol = "http";
    }
    if( u.host.empty() ) {
        result.host = domainName.value(*this).string();
    }
    if( !u.pathname.is_complete() ) {
        result.pathname = base / u.pathname;
    }
    return result;
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
    variables::const_iterator look = vars.find(name);
    if( look == vars.end() || source <= look->second.source ) {
        vars[name] = valT(value,source);
    }
}


void session::reset()
{
    typedef std::list<variables::iterator> eraseSet;

    eraseSet erased;
    for( variables::iterator  nv = vars.begin();
         nv != vars.end(); ++nv ) {
        if( nv->second.source >= unknown ) {
            erased.push_back(nv);
        }
    }
    for( eraseSet::const_iterator e = erased.begin(); e != erased.end(); ++e ) {
        vars.erase(*e);
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


boost::filesystem::path
session::prefixdir( const boost::filesystem::path& p ) const {
    boost::filesystem::path dir = p;
    while( !boost::filesystem::is_directory(dir) ) {
        /* boost is returning root/log as the parent path
           for root/log/ (note the trailing backslash).
           Of course it isn't what we expected here. */
        dir.remove_leaf();
    }
    return dir;
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

  // This code is used to debug initial permission problems:
    struct passwd *pw;
    struct group *grp;
  // real:
    pw = getpwuid(getuid());
    grp = getgrgid(getgid());
  // effective:
    pw = getpwuid(geteuid());
    grp = getgrgid(getegid());
*/
    uid_t realId = getuid();
    uid_t newId = v ? 0 : realId;
    if( setuid(newId) < 0 ) {
        std::cerr << "error: setuid to zero: "
                  << ((errno == EINVAL) ? "invalid" : "eperm") << std::endl;
    }
}


static std::string nullString("");


const std::string& session::valueOf( const std::string& name ) const {
    variables::const_iterator iter = find(name);
    if( found(iter) ) {
        return iter->second.value;
    }
    return nullString;
}

boost::filesystem::path
session::abspath( const boost::filesystem::path& p ) const {
    using namespace boost::filesystem;

    char rpath[PATH_MAX];
    if( !p.is_complete() ) {
        /* If *relative* is not yet an absolute path, we prefix it
           with the current directory which is assumed to exits. */
        realpath(current_path().string().c_str(),rpath);
        return path(rpath) / p;
    } else if( boost::filesystem::exists(p) ) {
        /* If *p* is an absolute path, we return it as a realpath.
           session::subdir works with text strings and thus links
           have to resolved and pathnames normalized. */
        realpath(p.string().c_str(),rpath);
        path result(rpath);
        return result;
    }
    /* If *p* is an absolute path with no physical existance
       in the filesystem at this point, we return it "as is". */
    return p;
}


boost::filesystem::path
session::abspath( const url& relative ) const
{
    using namespace boost::filesystem;

    path result(relative.pathname);
    if( relative.protocol != "file" ) {
        if( relative.pathname.is_complete() ) {
            result = siteTop.value(*this) / relative.pathname;
        } else {
            char rpath[PATH_MAX];
            realpath(current_path().string().c_str(),rpath);
            result = path(rpath) / relative.pathname;
            /* We used to check *result* is in *siteTop* for security.
               That does not sit well with templates which could be outside
               *siteTop* for security reasons as well. So we relaxed
               pathname checks here to accomodate. */
        }
    }
    return result;
}


void session::restore( int argc, char *argv[] )
{
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem;
    using namespace boost::program_options;

    /* We assume that if SCRIPT_FILENAME is defined
       we are running as a cgi. */
    ascgi = ( getenv("SCRIPT_FILENAME") != NULL );

    remoteAddr = "unknown";
    char *_remoteAddr = getenv("REMOTE_ADDR");
    if( _remoteAddr ) {
        boost::smatch m;
        if( regex_match(std::string(_remoteAddr),m,
                boost::regex("\\d+\\.\\d+\\.\\d+\\.\\d+")) ) {
            remoteAddr = _remoteAddr;
        }
    }

    positional_options_description pd;
    pd.add(document.name, -1);

    /* 1. The command-line arguments are added to the session. */
    {
        variables_map params;
        command_line_parser parser(argc, argv);
        parser.options(opts);
        parser.positional(pd);
        boost::program_options::store(parser.run(),params);
        for( variables_map::const_iterator param = params.begin();
             param != params.end(); ++param ) {
            if( param->first == document.name ) {
                typedef std::vector<boost::filesystem::path> inputfilesType;
                inputfilesType inputfiles
                    = param->second.as<inputfilesType>();
                const char* protocol;
                for( inputfilesType::const_iterator inp = inputfiles.begin();
                     inp != inputfiles.end(); ++inp ) {
                    /* I tried to chose the input argument type as either
                       filenames (run in shell) or urls (run as cgi). In many
                       cases, the dispatcher patterns work the same for both.
                       It does not for the homepage (/) and other patterns
                       with no wildcards.
                       The code below is a compromise in convinience.
                       Directories are typed as url even when we run in a shell.
                    */
                    boost::filesystem::path p;
                    if( boost::filesystem::is_regular_file(*inp) ) {
                        protocol = "file";
                        p = abspath(*inp);
                    } else {
                        protocol = "";
                        p = *inp;
                    }
#if 0
                    if( boost::filesystem::is_directory(p) ) {
                        /* We add an extra '/' at the end of directory names
                           such that the url pattern matcher works correctly. */
                        p /= boost::filesystem::path("/");
                    }
#endif
                    inputs.push_back(url(protocol,"",p));
                }
                insert(document.name,inputs[0].string());
            } else {
                /* Without the strip(), there is a ' ' appended to the command
                   line on Linux apache2. */
                std::string s = strip(param->second.as<std::string>());
                if( !s.empty() ) {
                    insert(param->first,s,cmdline);
                }
            }
        }
    }

    /* 2. If a "config" file name does not exist at this point, a (config,
       filename) pair compiled into the binary executable is added. */
    std::string config(configFile);
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
   sessionId = valueOf(sessionName);
   if( sessionId.empty() ) {
       cgi_parser::querySet::const_iterator sid 
           = parser.query.find(sessionName);
       if( sid != parser.query.end() ) {
           sessionId = sid->second;
       }
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
   std::string docname = document.value(*this).string();
   if( boost::filesystem::is_directory(docname)
       && (docname.size() == 0
           || docname[docname.size() - 1] != '/') ) {
       sourceType source = unknown;
       variables::const_iterator iter = vars.find(document.name);
       if( iter != vars.end() ) {
           source = iter->second.source;
       }
       insert(document.name,docname + '/',source);
   }

   if( valueOf(cacheTop.name).empty() ) {
       /* does not use cacheTop.value(*this) in order to avoid throwing
          an exception. Use cmdline such that reset() does not remove it. */
       insert(cacheTop.name,
           boost::filesystem::current_path().string(),cmdline);
   }

   bool printHelp = false;
   if( !runAsCGI() ) {
       for( int i = 1; i < argc; ++i ) {
           if( strncmp(argv[i],"--help",6) == 0 ) {
               printHelp = true;
           }
       }
       if( argc <= 1 ) {
           printHelp = true;
       }
       if( printHelp ) {
           using namespace std;
           boost::filesystem::path binname 
               = boost::filesystem::basename(argv[0]);
           cout << binname << "[options] pathname" << endl << endl
                << "Version" << endl
                << "  " << binname << " version " /* \todo << VERSION */ << endl << endl;
           cout << "Options" << endl
                << visible << endl;
           cout << "Further Documentation" << endl
                << "Semilla relies on session variables (ex. *siteTop*)"
                << " to find relevent pieces of information to build"
                << " a page."
                << " Session variables can be defined as:\n"
                << "   * Arguments on the command-line\n"
                << "   * Name=Value pairs in a global configuration file\n"
                << "   * Parameters passed through CGI environment "
                << "variables\n"
                << "   * Name=Value pairs in a unique session file\n"
                << "When a variable is defined in more than one set, "
                << "command-line arguments have precedence over the global "
                << "configuration file which has precedence over"
                << " environment variables which in turn as precedence"
                << " over the session file." << endl;
       }
   }
}


boost::filesystem::path
session::root( const boost::filesystem::path& leaf,
    const boost::filesystem::path& trigger, bool keepTrigger ) const
{
    using namespace boost::filesystem;
    using namespace boost::system::errc;

    checkInSiteTop(*this,leaf);

    path dirname = leaf;
    if( !is_directory(dirname) ) {
        dirname = dirname.parent_path();
    }
    bool foundProject = boost::filesystem::exists(dirname.string() / trigger);
    while( !foundProject && (dirname.string() != siteTop.value(*this)) ) {
        dirname.remove_leaf();
        assert( !dirname.string().empty() );
        foundProject = boost::filesystem::exists(dirname.string() / trigger);
    }
    return foundProject ? (keepTrigger ? (dirname.string() / trigger) : dirname) : path("");
}


void session::show( std::ostream& ostr ) const {

    static const char *sourceTypeNames[] = {
        "cmdline",
        "configfile",
        "sessionfile",
        "queryenv",
        "unknown"
    };

    if( exists() ) {
        ostr << "session " << sessionId << std::endl;
    } else {
        ostr << "no session id" << std::endl;
    }
    for( variables::const_iterator var = vars.begin();
         var != vars.end(); ++var ) {
        ostr << var->first << ": " << var->second.value
             << " [" << sourceTypeNames[var->second.source] << "]" << std::endl;
    }
}


void session::store() {
    using namespace boost::system;
    using namespace boost::filesystem;

    /* 1. open session file based on session id. */
    ofstream sessions(stateFilePath());
    if( sessions.fail() ) {
        using namespace boost::system::errc;
        boost::throw_exception(boost::system::system_error(
                make_error_code(no_such_file_or_directory),
                stateFilePath().string()));
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
session::subdirpart( const boost::filesystem::path& rootp,
    const boost::filesystem::path& leafp ) const
{
    using namespace boost::filesystem;

#if 0
    std::cerr << "[subdirpart] (" << rootp << "," << leafp << ")" << std::endl;
#endif

    char realp[FILENAME_MAX];
    path root, leaf;
    if( boost::filesystem::exists(rootp) ) {
        realpath(rootp.string().c_str(),realp);
        root = path(realp);
    } else {
        root = rootp;
    }
    if( boost::filesystem::exists(leafp) ) {
        realpath(leafp.string().c_str(),realp);
        if( boost::filesystem::is_directory(leafp) ) {
            /* realpath() will remove the trailing '/' on directory names.
               We need to keep it through if we want to generate accurate urls.
            */
            size_t len = strlen(realp);
            if( len + 1 < FILENAME_MAX ) {
                realp[len] = '/';
                realp[len + 1] = '\0';
            }
        }
        leaf = path(realp);
    } else {
        leaf = leafp;
    }

    if( leaf.string().compare(0,root.string().size(),root.string()) != 0 ) {
        boost::throw_exception(notLeadingPrefixError(root.string(),
                leaf.string()));
    }
    return leaf.string().substr(std::min(leaf.string().size(),
            root.string().size()
            + (root.string()[root.string().size() - 1] != '/' ? 1 : 0)));
}


boost::filesystem::path
session::valueAsPath( const std::string& name ) const {
    variables::const_iterator iter = vars.find(name);
    if( iter == vars.end() ) {
        boost::throw_exception(undefVariableError(name));
    }
    boost::filesystem::path absolute
        = abspath(boost::filesystem::path(iter->second.value));
    return absolute;
}

url
session::cacheName( const url& href ) const
{
    /* If we were to blindly add a .html extension to all cached url,
       we could never generate cached versions of index.rss, etc.
       We only append an .html extension to cached files which correspond
       to regular files on the sites. If we were to mark generated
       files such as index.rss as "always" in the dispatch table, we would
       not generate them at all. They thus are marked as "whenNotCached"
       and a late file exists test is done here. */
    boost::filesystem::path name(href.pathname);
    boost::filesystem::path result = name;
    boost::filesystem::path absname = abspath(href);
    if( boost::filesystem::is_directory(absname) ) {
        result = name / "index.html";
    } else if( boost::filesystem::is_regular_file(absname) ) {
        std::set<boost::filesystem::path> softs;
#if 0
        /* Until we figure a reliable way to include soft extensions
           in Apache2 RewriteRule/RewriteCond, they are disabled. */
        softs.insert(".blog");
        softs.insert(".corp");
#endif
        if( softs.find(name.extension()) != softs.end() ) {
            result.replace_extension(".html");
        } else {
            result = name.string() + ".html";
        }
    } else if( name.extension().empty() ) {
        /* need to add .html to "dates", "tags" but not index.rss. */
        result = name.string() + ".html";
    }
    return url(href.protocol,href.host,href.port,result);
}


boost::filesystem::path
session::absCacheName( const url& href ) const
{
#if 0
    url cached = cacheName(url(href.protocol,href.host,href.port,
#else
            // \todo temporary kludge
    url cached = cacheName(url("",href.host,href.port,
#endif
            boost::filesystem::path("/")
            / subdirpart(siteTop.value(*this),abspath(href))));
    return cacheTop.value(*this) / cached.pathname;
}

