/* Copyright (c) 2009, Sebastien Mirolo
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of codespin nor the
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

	std::cerr << "git cmd: " << cmd.str() << std::endl;

	FILE *diffFile = popen(cmd.str().c_str(),"r");
	if( diffFile == NULL ) {
		std::cerr << "error opening command: " << cmd.str() << std::endl;
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
					  const boost::filesystem::path& pathname ) {

	/* The git command needs to be issued from within a directory 
	   where .git can be found by walking up the tree structure. */ 
	boost::filesystem::initial_path();
	boost::filesystem::current_path(rootpath);

	std::stringstream sstm;
	sstm << executable << " show --summary --pretty=oneline " << pathname; 

	char line[256];
	FILE *summary = popen(sstm.str().c_str(),"r");
	assert( summary != NULL );

	while( fgets(line,sizeof(line),summary) != NULL ) {
		ostr << line;
	}
	pclose(summary);

	boost::filesystem::current_path(boost::filesystem::initial_path());
}
