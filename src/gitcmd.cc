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

#include <cstdio>
#include <sys/stat.h>
#include "changelist.hh"
#include "markup.hh"
#include <boost/regex.hpp>

/** Execute git commands

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

void gitcmd::create( const boost::filesystem::path& pathname )
{
    using namespace boost::system;
    using namespace boost::filesystem;

    typedef std::list<std::string> configText;

    if( !exists(pathname) ) {
	create_directories(pathname);
    }

    /* The git command needs to be issued from within a directory where .git
       can be found by walking up the tree structure. */
    boost::filesystem::initial_path(); 
    boost::filesystem::current_path(pathname);

    std::stringstream cmd;
    cmd << executable << " init";

    int err = system(cmd.str().c_str());
    if( err ) {
	throw basic_filesystem_error<path>(std::string("create repository"),
					   pathname, 
					   error_code());

    }

    path hook(path(".git") / path("hooks") / path("post-update"));
    path hookSample(hook.string() + ".sample");
    if( is_regular_file(hookSample) ) {
        rename(hookSample,hook);
    }
    if( is_regular_file(hook) ) {
        chmod(hook.string().c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    }

    configText lines;
    path configPath(path(".git") / path("config"));
    int foundReceiveIdx = -1;
    int foundDenyCurrentBranchIdx = -1;
    {
	/* Look for the *denyCurrentBranch* configuration attribute
	   inside the *receive* block. */
	int n = -1;
	ifstream config(configPath);
	while( !config.eof() ) {
	    boost::smatch m;
	    std::string line;
	    std::getline(config,line);
	    lines.push_back(line);
	    ++n;
	    if( boost::regex_match(line,m,boost::regex("[receive]")) ) {
		foundReceiveIdx = n;
	    } else if( regex_match(line,m,
			   boost::regex("\\s*denyCurrentBranch = (\\S+)")) ) {
		foundDenyCurrentBranchIdx = n;
	    }
	    
	}
	config.close();
    }

    {
	/* Add "denyCurrentBranch = ignore" into the git config file
	   such that it is possible to push commits into the repository
	   even though it is not a bare .git repository. */
	ofstream config(configPath);
	configText::const_iterator foundReceive = lines.end();
	if( foundReceiveIdx >= 0 ) {
	    foundReceive = lines.begin();
	    std::advance(foundReceive,foundReceiveIdx);
	}
	configText::const_iterator foundDenyCurrentBranch = lines.end();
	if( foundDenyCurrentBranchIdx >= 0 ) {
	    foundDenyCurrentBranch = lines.begin();
	    std::advance(foundDenyCurrentBranch,foundDenyCurrentBranchIdx);
	}
	
	configText::const_iterator line = lines.begin();
	for( ; line != foundReceive; ++line ) {
	    config << *line << std::endl;
	}
	if( foundReceive != lines.end() ) {
	    for( ; line != foundDenyCurrentBranch; ++line ) {
		config << *line << std::endl;
	    }
	    config << "\tdenyCurrentBranch = ignore\n";
	    if( foundDenyCurrentBranch != lines.end() ) {
		++line;
	    }
	    for( ; line != lines.end(); ++line ) {
		config << *line << std::endl;
	    }
	} else {
	    config << "[receive]\n";
	    config << "\tdenyCurrentBranch = ignore\n";
	}
	config.close();
    }

    boost::filesystem::current_path(boost::filesystem::initial_path());
    rootpath = pathname;
}


void gitcmd::add( const boost::filesystem::path& pathname )
{
    using namespace boost::system;
    using namespace boost::filesystem;

   /* The git command needs to be issued from within a directory where .git
       can be found by walking up the tree structure. */
    boost::filesystem::initial_path(); 
    boost::filesystem::current_path(rootpath);

    if( pathname.string().compare(0,rootpath.string().size(),
				  rootpath.string()) == 0 ) {	
	std::string relative 
	    = (pathname.string().size() == rootpath.string().size()) ?
	    std::string(".") :
	    pathname.string().substr(rootpath.string().size() + 1);

	std::stringstream cmd;
	cmd << executable << " add " << relative;
	int err = system(cmd.str().c_str());
	if( err ) {
	    throw basic_filesystem_error<path>(std::string("add files"),
					       pathname, 
					       error_code());
	    
	}
    }

    boost::filesystem::current_path(boost::filesystem::initial_path());
}


void gitcmd::commit( const std::string& msg ) 
{
    using namespace boost::system;
    using namespace boost::filesystem;

   /* The git command needs to be issued from within a directory where .git
       can be found by walking up the tree structure. */
    boost::filesystem::initial_path(); 
    boost::filesystem::current_path(rootpath);

    std::stringstream cmd;
    cmd << executable << " commit -m '" << msg << "'";
    int err = system(cmd.str().c_str());
    if( err ) {
	throw basic_filesystem_error<path>(std::string("commit"),
					   "", 
					   error_code());
    }

    boost::filesystem::current_path(boost::filesystem::initial_path());
}


void gitcmd::diff( std::ostream& ostr, 
		   const std::string& leftCommit, 
		   const std::string& rightCommit, 
		   const boost::filesystem::path& pathname ) {
    /* The git command needs to be issued from within a directory where .git
       can be found by walking up the tree structure. */
    boost::filesystem::initial_path(); 
    boost::filesystem::current_path(rootpath);
    
    std::stringstream cmd;
    cmd << executable << " diff " << leftCommit << " " << rightCommit 
	<< " " << pathname;
    
    FILE *diffFile = popen(cmd.str().c_str(),"r");
    if( diffFile == NULL ) {
	return;
    }
    char line[256];
    while( fgets(line,sizeof(line),diffFile) != NULL ) {
	ostr << line;
    }
    pclose(diffFile);
    boost::filesystem::current_path(boost::filesystem::initial_path());
}


void gitcmd::history( std::ostream& ostr, 
		      const session& s,
		      const boost::filesystem::path& pathname,
		      historyref& ref ) {
    
    /* The git command needs to be issued from within a directory 
       where .git can be found by walking up the tree structure. */ 
    boost::filesystem::initial_path();
    boost::filesystem::current_path(rootpath);
    
    std::stringstream sstm;
    sstm << executable << " log --date=rfc --pretty=oneline " 
	 << relpath(pathname,rootpath);    
    
    char lcstr[256];
    FILE *summary = popen(sstm.str().c_str(),"r");
    assert( summary != NULL );
    
    while( fgets(lcstr,sizeof(lcstr),summary) != NULL ) {
	std::string line(lcstr);
	
	/* Parse the summary line in order to split the commit tag 
	   from the commit message. */
	
	size_t splitPos = line.find(' ');
	if( splitPos != std::string::npos ) {
	    std::string rightRevision = line.substr(0,splitPos);
	    std::string title = line.substr(splitPos);

	    /* \todo '\n' at end of line? */
	    ostr << html::a().href(ref.asUrl(s.asUrl(pathname).string(),
					     rightRevision).string()).title(title)
		 << rightRevision.substr(0,10) << "..." 
		 << html::a::end << "<br />";
	}
    }
    pclose(summary);
    
    boost::filesystem::current_path(boost::filesystem::initial_path());
}


void gitcmd::checkins( ::history& hist,
		       const session& s,
		       const boost::filesystem::path& pathname ) {    
    /* The git command needs to be issued from within a directory 
       where .git can be found by walking up the tree structure. */ 
    boost::filesystem::initial_path();
    boost::filesystem::current_path(rootpath);

    boost::filesystem::path project = s.subdirpart(s.valueOf("srcTop"),rootpath);

    /* shows only the last 2 commits */
    std::stringstream sstm;
    sstm << executable << " log --date=rfc --name-only -2 "; 
    
    char lcstr[256];
    FILE *summary = popen(sstm.str().c_str(),"r");
    assert( summary != NULL );
        
    bool itemStarted = false;
    bool descrStarted = false;
    checkin *ci = NULL;
    std::stringstream descr, title;
    while( fgets(lcstr,sizeof(lcstr),summary) != NULL ) {
	std::string line(lcstr);
	if( strncmp(lcstr,"commit",6) == 0 ) {
	    if( descrStarted ) {		
		ci->title = title.str();
		ci->descr = descr.str();		
		descrStarted = false;
	    }
	    itemStarted = true;
	    ci = hist.add();
	    lcstr[strlen(lcstr) - 1] = '\0'; // remove trailing '\n'
	    title.str("");
	    title << strip(line.substr(7)) << " ";
	    ci->guid = strip(line.substr(7));
	    
	} else if ( line.compare(0,7,"Author:") == 0 ) {
	    /* The author field is formatted as "First Last <emailAddress>". */
	    size_t first = 7;
	    size_t last = first;
	    while( last < line.size() ) {
		switch( line[last] ) {
		case '<':
		    ci->authorName = line.substr(first,last - first);
		    first = last + 1;
		    break;
		case '>':
		    ci->authorEmail = line.substr(first,last - first);
		    first = last + 1;
		    break;
		}
		++last;
	    }	    

	} else if ( line.compare(0,5,"Date:") == 0 ) {
	    try {
		ci->time = from_mbox_string(line.substr(5));
	    } catch( std::exception& e ) {
		std::cerr << "!!! exception " << e.what() << std::endl; 
	    }

	} else {
	    /* more description */
	    if( !descrStarted ) {		
		descr.str("");		
		descrStarted = true;
	    }
	    if( !isspace(line[0]) ) {
		/* We are dealing with a file that was part of this commit. */
		std::stringstream hrefs;
#if 0
		hrefs << s.asUrl(project / boost::filesystem::path(strip(line)));
#else
		hrefs << s.asUrl(boost::filesystem::path(strip(line)));
#endif
		ci->addFile(hrefs.str());
	    } else {
		size_t maxTitleLength = 80;
		if( title.str().size() < maxTitleLength ) {
		    size_t remain = (maxTitleLength - title.str().size()); 
		    if( line.size() > remain ) {
			title << strip(line.substr(0,remain)) << "...";
		    } else {
			title << strip(line);
		    }
		}
		descr << lcstr;
	    }
	}
    }
    pclose(summary);
    if( descrStarted ) {
	ci->title = title.str();
	ci->descr = descr.str();
	descrStarted = false;
    }
    boost::filesystem::current_path(boost::filesystem::initial_path());
}
