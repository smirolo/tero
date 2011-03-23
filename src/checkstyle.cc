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

#include "checkstyle.hh"
#include "markup.hh"
#include "slice.hh"
#include "decorator.hh"

/** Check source files according to coding standards.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


const char *licenseCodeTitles[] = {
    "unknown",
    "BSD"
};


void checker::cache() {
    static const boost::regex 
	bsdex("\\S?\\S?\\s*Copyright \\(c\\) (\\d+(?:-\\d+)?), (.*)\\s+"
    "\\S?\\s*All rights reserved.\\s+"
"\\S?\\s+"
"\\S?\\s*Redistribution and use in source and binary forms, with or without\\s+"
	      "\\S?\\s*modification, are permitted provided that the following conditions are met:\\s+"
"\\S?\\s*\\* Redistributions of source code must retain the above copyright\\s+"
	      "\\S?\\s*notice, this list of conditions and the following disclaimer.\\s+"
	      "\\S?\\s*\\* Redistributions in binary form must reproduce the above copyright\\s+"
	      "\\S?\\s*notice, this list of conditions and the following disclaimer in the\\s+"
	      "\\S?\\s*documentation and/or other materials provided with the distribution.\\s+"

"\\S?\\s*\\* Neither the name of (.*) nor the\\s+"
"\\S?\\s*names of its contributors may be used to endorse or promote products\\s+"
	      "\\S?\\s*derived from this software without specific prior written permission.\\s+"
	      "\\S?\\s+"
"\\S?\\s*THIS SOFTWARE IS PROVIDED BY (.*) ''AS IS'' AND ANY\\s+"
"\\S?\\s*EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\\s+"
"\\S?\\s*WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\\s+"
"\\S?\\s*DISCLAIMED. IN NO EVENT SHALL (.*) BE LIABLE FOR ANY\\s+"
"\\S?\\s*DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\\s+"
"\\S?\\s*\\(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\\s+"
"\\S?\\s*LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION\\) HOWEVER CAUSED AND\\s+"
"\\S?\\s*ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\\s+"
"\\S?\\s*\\(INCLUDING NEGLIGENCE OR OTHERWISE\\) ARISING IN ANY WAY OUT OF THE USE OF THIS\\s+"
"\\S?\\s*SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE\\..*");

    boost::smatch m;
    std::string s(licenseText.begin(),licenseText.size());
    if( boost::regex_search(s,m,bsdex) ) {
	licenseType = BSDLicense;
	dates = m.str(1);
	grantor = m.str(2);
    }
    cached = true;
}


cppChecker::cppChecker() 
    : comment(emptyLine),    
      tokenizer(*this)

{
}

    
void cppChecker::newline(const char *line, 
			 int first, int last ) {	
    switch( state ) {
    case readLicense:
	licenseText += slice<const char>(&line[first],&line[last]);
	break;
    default:
	/* Nothing to do except prevent gcc from complaining. */   
	break;
    }
    ++nbLines;
    if( comment == codeLine ) ++nbCodeLines;
    comment = emptyLine;
}


void cppChecker::token( cppToken token, const char *line, 
			  int first, int last, bool fragment ) {
    switch( token ) {
    case cppComment:
	switch( state ) {
	case start:
	    state = readLicense;
	case readLicense:
	    licenseText += slice<const char>(&line[first],&line[last]);
	    if( !fragment ) state = doneLicense;
	    break;
	default:
	    /* Nothing to do except prevent gcc from complaining. */
	    break;
	}
	/* \todo we donot mark comment lines correctly but it does not
	   matter because if there is any code on the line, it will
	   be correctly marked as a "codeLine". */
	if( fragment ) comment = commentLine;
	break;
    default:
	comment = codeLine;
	switch( state ) {
	case readLicense:
	    state = doneLicense;
	    break;
	default:
	    /* Nothing to do except prevent gcc from complaining. */
	    break;
	}
	break;
    }
}


void shChecker::newline(const char *line, int first, int last ) 
{
    switch( state ) {
    case readLicense:
	licenseText += slice<const char>(&line[first],&line[last]);
	break;
    default:
	/* Nothing to do except prevent gcc from complaining. */   
	break;
    }
    ++nbLines;
    /* \todo count codeLines... */
}


void shChecker::token( shToken token, const char *line, 
			 int first, int last, bool fragment )
{
    switch( token ) {
    case shComment:
	switch( state ) {
	case start: {
	    boost::smatch m;
	    std::string s(&line[first],&line[last]);
	    if( !boost::regex_search(s,m,boost::regex("Copyright")) ) break;
	    state = readLicense;	    	    
	}
	case readLicense:
	    licenseText += slice<const char>(&line[first],&line[last]);
	    if( !fragment ) state = doneLicense;
	    break;
	default:
	    /* Nothing to do except prevent gcc from complaining. */   
	    break;
	}
	break;
    case shCode:
	state = doneLicense;
	break;
    default:
	/* Nothing to do excepts shutup gcc warnings. */
	break;
    }
}


void checkstyle::addDir( session& s, 
			 const boost::filesystem::path& pathname ) const {
}


void checkstyle::addFile( session& s, 
			  const boost::filesystem::path& pathname ) const {
    using namespace boost::filesystem; 

    if( state == start ) {
	s.out() << html::p() 
		<< html::table();
	s.out() << html::tr()
		  << html::th() << html::th::end
		  << html::th() << "license" << html::th::end
		  << html::th() << "code lines" << html::th::end
		  << html::th() << "total lines" << html::th::end
		  << html::tr::end;
	state = toplevelFiles;
    }
    dispatchDoc::instance()->fetch(s,"check",pathname);
}

void checkstyle::flush( session& s ) const 
{
    if( state != start ) {
	s.out() << html::table::end
		  << html::p::end;
    }
}


void checkstyleFetch( session& s, const boost::filesystem::path& pathname )
{
    checkstyle p;
    p.fetch(s,pathname);
}
