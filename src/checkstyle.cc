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

const char *licenseCodeTitles[] = {
    "unknown",
    "BSD"
};


checkfile::checkfile() : cached(false), 
			 licenseType(unknownLicense),
			 nbLines(0), nbCodeLines(0) {}

void checkfile::cache() {
#if 0
    static const boost::regex 
	bsdex("\\S?\\S?\\s*Copyright \\(c\\) (?P<date>(\\d+)(-\\d+)*), (?P<grantor>.*)"
"\\S?\\s*All rights reserved."
"\\S?\\s*"
"\\S?\\s*Redistribution and use in source and binary forms, with or without"
"\\S?\\s*modification, are permitted provided that the following conditions are met:"
"\\S?\\s*\\* Redistributions of source code must retain the above copyright"
"\\S?\\s*notice, this list of conditions and the following disclaimer."
"\\S?\\s*\\* Redistributions in binary form must reproduce the above copyright"
"\\S?\\s*notice, this list of conditions and the following disclaimer in the"
"\\S?\\s*documentation and/or other materials provided with the distribution."
"\\S?\\s*\\* Neither the name of (?P<brand>.*) nor the"
"\\S?\\s*names of its contributors may be used to endorse or promote products"
"\\S?\\s*derived from this software without specific prior written permission."
""
"\\S?\\s*THIS SOFTWARE IS PROVIDED BY (.*) ''AS IS'' AND ANY"
"\\S?\\s*EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED"
"\\S?\\s*WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE"
"\\S?\\s*DISCLAIMED. IN NO EVENT SHALL (.*) BE LIABLE FOR ANY"
"\\S?\\s*DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES"
"\\S?\\s*\\(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;"
"\\S?\\s*LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION\\) HOWEVER CAUSED AND"
"\\S?\\s*ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT"
"\\S?\\s*\\(INCLUDING NEGLIGENCE OR OTHERWISE\\) ARISING IN ANY WAY OUT OF THE USE OF THIS"
"\\S?\\s*SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE\\..");

#else

    static const boost::regex 
	bsdex("\\S?\\S?\\s*Copyright \\(c\\) ((\\d+)(-\\d+)*), (.*)\n"
"\\S?\\s*All rights reserved."
"\\S?\\s*"
"\\S?\\s*Redistribution and use in source and binary forms, with or without"
"\\S?\\s*modification, are permitted provided that the following conditions are met:"
"\\S?\\s*\\* Redistributions of source code must retain the above copyright"
"\\S?\\s*notice, this list of conditions and the following disclaimer."
"\\S?\\s*\\* Redistributions in binary form must reproduce the above copyright"
"\\S?\\s*notice, this list of conditions and the following disclaimer in the"
"\\S?\\s*documentation and/or other materials provided with the distribution."
"\\S?\\s*\\* Neither the name of (.*) nor the"
"\\S?\\s*names of its contributors may be used to endorse or promote products"
"\\S?\\s*derived from this software without specific prior written permission."
""
"\\S?\\s*THIS SOFTWARE IS PROVIDED BY (.*) ''AS IS'' AND ANY"
"\\S?\\s*EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED"
"\\S?\\s*WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE"
"\\S?\\s*DISCLAIMED. IN NO EVENT SHALL (.*) BE LIABLE FOR ANY"
"\\S?\\s*DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES"
"\\S?\\s*\\(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;"
"\\S?\\s*LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION\\) HOWEVER CAUSED AND"
"\\S?\\s*ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT"
"\\S?\\s*\\(INCLUDING NEGLIGENCE OR OTHERWISE\\) ARISING IN ANY WAY OUT OF THE USE OF THIS"
"\\S?\\s*SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE\\..*");
#endif

    boost::smatch m;
    std::string s(licenseText.begin(),licenseText.size());
    if( boost::regex_search(s,m,bsdex) ) {
	licenseType = BSDLicense;
#if 0
	std::cerr << "!!! 1: " << m.str(1) << std::endl;
	std::cerr << "!!! 4: " << m.str(4) << std::endl;
	std::cerr << "!!! 5: " << m.str(5) << std::endl;
#endif
    }
    cached = true;
}



void checkfile::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::filesystem; 

    url href;
    std::string name;
#if 1
    path projdir = s.root(pathname,"index.xml");
    if( s.prefix(projdir,pathname) ) {
	name = s.subdirpart(projdir,pathname).string();
	href = s.asUrl(pathname);
    } else {
	name = pathname.string();
    }
#else
    name = pathname.string();
#endif

    std::cout << html::tr()
	      << html::td() << html::a().href(href.string()) 
	      << name << html::a::end << html::td::end
	      << html::td() << license() << html::td::end
	      << html::td() << nbCodeLines << html::td::end
	      << html::td() << nbLines << html::td::end
	      << html::tr::end;
}

cppCheckfile::cppCheckfile() : state(start), comment(emptyLine)
			        {}

    
void cppCheckfile::newline(const char *line, 
			 int first, int last ) {	
    switch( state ) {
    case readLicense:
	licenseText += slice<const char>(&line[first],&line[last]);
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
	}
	break;
    }
}


void cppCheckfile::fetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace boost::filesystem; 

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
}


void shCheckfile::token( shToken token, const char *line, 
			 int first, int last, bool fragment )
{
}


void shCheckfile::fetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace boost::filesystem; 

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

