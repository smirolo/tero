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

#include <iostream>
#include <boost/regex.hpp>
#include <boost/filesystem/fstream.hpp>
#include "composer.hh"

/** Document composition

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

pathVariable themeDir("themeDir",
		      "directory that contains the user interface templates");

void 
composerAddSessionVars( boost::program_options::options_description& opts,
			boost::program_options::options_description& visible )
{
    using namespace boost::program_options;
    
    options_description localOpts("composer");
    localOpts.add(themeDir.option());
    opts.add(localOpts);    
    visible.add(localOpts);

    options_description hiddenOpts("hidden");
    hiddenOpts.add_options()
	("document",value<std::string>(),"document");
    opts.add(hiddenOpts);    
}


void embed( session& s, const std::string& value ) {
    using namespace boost::filesystem;
    std::ostream& prevDisp = s.out();
    try {
	dispatchDoc::instance()->fetch(s,value);
    } catch( const basic_filesystem_error<path>& e ) {
	s.out(prevDisp);
	++s.nErrs;
	std::cerr << "[embed of '" << value << "'] " 
		  << e.what() << std::endl;	
	s.out() << "<p>" << e.what() << "</p>" << std::endl;
    }
}


