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

#include <cstdio>
#include "changelist.hh"
#include "markup.hh"


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
    sstm << executable << " log --pretty=oneline " 
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
    sstm << executable << " log --name-only -2 "; 
    
    char lcstr[256];
    FILE *summary = popen(sstm.str().c_str(),"r");
    assert( summary != NULL );
        
    bool itemStarted = false;
    bool descrStarted = false;
    checkin *ci = NULL;
    std::stringstream descr;
    while( fgets(lcstr,sizeof(lcstr),summary) != NULL ) {
	std::string line(lcstr);
	if( strncmp(lcstr,"commit",6) == 0 ) {
	    if( descrStarted ) {
		ci->descr = descr.str();		
		descrStarted = false;
	    }
	    itemStarted = true;
	    ci = hist.add();
	    lcstr[strlen(lcstr) - 1] = '\0'; // remove trailing '\n'
	    ci->title = std::string(lcstr).substr(7);
	    
	} else if ( line.compare(0,7,"Author:") == 0 ) {
	    ci->author = strip(line.substr(7));

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
		descr << lcstr;
	    }
	}
    }
    pclose(summary);
    if( descrStarted ) {
	ci->descr = descr.str();
	descrStarted = false;
    }
    boost::filesystem::current_path(boost::filesystem::initial_path());
}
