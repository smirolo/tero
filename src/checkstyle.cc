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

#include "checkstyle.hh"
#include "markup.hh"
#include "slice.hh"
#include "decorator.hh"


const char *licenseCodeTitles[] = {
    "unknown",
    "BSD"
};


checkfile::checkfile( std::ostream& o ) : document(o),
					  cached(false), 
					  licenseType(unknownLicense),
					  nbLines(0), nbCodeLines(0) {}

void checkfile::cache() {
    static const boost::regex 
	bsdex("\\S?\\S?\\s*Copyright \\(c\\) ((\\d+)(-\\d+)*), (.*)\\s+"
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
    }
    cached = true;
}



void checkfile::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::filesystem; 

    url href;
    std::string name;
    path projdir = s.root(pathname,"dws.xml");
    if( s.prefix(projdir,pathname) ) {
	name = s.subdirpart(projdir,pathname).string();
	href = s.asUrl(pathname);
    } else {
	name = pathname.string();
    }

    *ostr << html::tr()
	 << html::td() << html::a().href(href.string()) 
	 << name << html::a::end << html::td::end
	 << html::td() << license() << html::td::end
	 << html::td() << nbCodeLines << html::td::end
	 << html::td() << nbLines << html::td::end
	 << html::tr::end;
}

cppCheckfile::cppCheckfile( std::ostream& o ) 
    : checkfile(o), state(start), comment(emptyLine)
{
}

    
void cppCheckfile::newline(const char *line, 
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


void cppCheckfile::token( cppToken token, const char *line, 
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


void cppCheckfile::fetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace boost::filesystem; 

    state = start;
    comment = emptyLine;
    cached = false; 
    licenseType = unknownLicense;
    nbLines = 0;
    nbCodeLines = 0;
    licenseText = slice<const char>();
    
    cppTokenizer tokenizer(*this);

    ifstream file;
    size_t fileSize = file_size(pathname);
    char buffer[ fileSize + 1 ];
    
    open(file,pathname);
    file.read(buffer,fileSize);
    buffer[fileSize] = '\0';
    file.close();

    tokenizer.tokenize(buffer,fileSize);

    checkfile::fetch(s,pathname);
}


void shCheckfile::newline(const char *line, int first, int last ) 
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


void shCheckfile::token( shToken token, const char *line, 
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


void shCheckfile::fetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace boost::filesystem; 

    cached = false; 
    licenseType = unknownLicense;
    nbLines = 0;
    nbCodeLines = 0;
    licenseText = slice<const char>();
    state = start;

    shTokenizer tokenizer(*this);

    ifstream file;
    size_t fileSize = file_size(pathname);
    char buffer[ fileSize + 1 ];
    
    open(file,pathname);
    file.read(buffer,fileSize);
    buffer[fileSize] = '\0';
    file.close();

    tokenizer.tokenize(buffer,fileSize);
    checkfile::fetch(s,pathname);
}


void checkstyle::addDir( const session& s, 
			 const boost::filesystem::path& pathname ) {
}


void checkstyle::addFile( const session& s, 
			  const boost::filesystem::path& pathname ) {
    using namespace boost::filesystem; 

    if( state == start ) {
	*ostr << html::p() 
		  << "<table>";
	*ostr << html::tr()
		  << html::th() << html::th::end
		  << html::th() << "license" << html::th::end
		  << html::th() << "code lines" << html::th::end
		  << html::th() << "total lines" << html::th::end
		  << html::tr::end;
	state = toplevelFiles;
    }
    document *doc = dispatchDoc::instance->select("check",pathname.string());
    doc->fetch((session&)s,pathname);
}

void checkstyle::flush() {
    if( state != start ) {
	*ostr << "</table>"
		  << html::p::end;
    }
}
