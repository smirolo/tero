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

#ifndef CONFIG_FILE
#error "CONFIG_FILE should be defined when compiling this file"
#endif
#ifndef SESSION_DIR
#error "SESSION_DIR should be defined when compiling this file"
#endif

const char* session::configFile = CONFIG_FILE;
const char* session::sessionDir = SESSION_DIR;


class todoModifFilter : public todoFilter {
public:
    explicit todoModifFilter( const boost::filesystem::path& m )
	: todoFilter(m) {}

    todoModifFilter( const boost::filesystem::path& m, postFilter* n  ) 
	: todoFilter(m,n) {}

    virtual void filters( const post& p ) {
	static const boost::regex 
	    guidPat("(\\[([a-z]|\\d){8}-([a-z]|\\d){4}-([a-z]|\\d){4}-([a-z]|\\d){4}-([a-z]|\\d){12}\\])");

	boost::smatch m;
	if( boost::regex_search(p.title,m,guidPat) ) {
	    std::cerr << "comment on " << m.str(1) << "..." << std::endl;
	} else {
	    std::cerr << "create..." << std::endl;
	}
    }
};


int main( int argc, char *argv[] )
{
    using namespace std;
    using namespace boost::program_options;
    using namespace boost::filesystem;

    session s("view","semillaId",std::cout);
    s.privileged(false);

    try {
	/* parse command line arguments */
	options_description opts;
	options_description genOptions("general");
	genOptions.add_options()
	    ("help","produce help message")
	    ("binDir",value<std::string>(),"path to outside executables");
	
	opts.add(genOptions);
	auth::addSessionVars(opts);

	s.restore(argc,argv,opts);

	todoModifFilter modif(".");
	if( !std::cin.eof() ) {
	    mailParser parser(modif);
	    parser.walk(s,std::cin);
	}
    } catch( exception& e ) {
	std::cerr << "!!! exception: " << e.what() << std::endl;
    }


    return 0;
}
