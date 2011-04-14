/* Copyright (c) 2010-2011, Fortylines LLC
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

#include "auth.hh"
#include "todo.hh"
#include "comments.hh"
#include "rfc2822tok.hh"

#ifndef CONFIG_FILE
#error "CONFIG_FILE should be defined when compiling this file"
#endif
#ifndef SESSION_DIR
#error "SESSION_DIR should be defined when compiling this file"
#endif

const char* session::configFile = CONFIG_FILE;
const char* session::sessionDir = SESSION_DIR;


class commentAction : public mailAsPost {
public:

    boost::filesystem::path filename() const {
	boost::filesystem::path result;
	if( constructed.title.compare(0,11,"Comment on ") == 0 ) {		
	    result = boost::filesystem::path(constructed.title.substr(11));
	}
	return result;
    }

};


char docPage[] = "document";

int main( int argc, char *argv[] )
{
    using namespace boost::program_options;
    using namespace boost::filesystem;

    session s("semillaId",std::cout);
    s.privileged(false);

    try {
	/* parse command line arguments */
	options_description opts;
	options_description visible;
	options_description genOptions("general");
	genOptions.add_options()
	    ("help","produce help message");
	
	opts.add(genOptions);
	visible.add(genOptions);
	session::addSessionVars(opts,visible);
	authAddSessionVars(opts,visible);
	commentAddSessionVars(opts,visible);
	s.restore(argc,argv,opts);

	std::stringstream msg;
#if 0
	std::copy(std::istream_iterator<char>(std::cin),
		  std::istream_iterator<char>(),
		  std::ostream_iterator<char>(msg));
#else
	while( !std::cin.eof() ) {
	    std::string line;
	    std::getline(std::cin,line);
	    msg << line << std::endl;
	}
#endif
	commentAction action;
	rfc2822Tokenizer tok(action);
	tok.tokenize(msg.str().c_str(),msg.str().size());

	boost::filesystem::path commented = action.filename();
	if( commented.empty() ) {	
	    std::cerr << "error: commented url is not specified!" << std::endl;
	    return 1;
	}
	appendCommentToFile comments(s,commented);
	comments.filters(action.unserialized());
       
#if 0
	void textMeta<document>(s,filename);
#endif

    } catch( std::exception& e ) {
	std::cerr << "!!! exception: " << e.what() << std::endl;
    }


    return 0;
}
