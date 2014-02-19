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

#include "document.hh"
#include "checkstyle.hh"
#include "markup.hh"
#include "slice.hh"
#include "decorator.hh"

/** Check source files according to coding standards.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


namespace {

class errTokWriter : public tero::errTokListener {
public:
	typedef std::map<int,std::string> annotationsType;
	annotationsType& annotations;
	boost::filesystem::path filename;
	bool record;
	int lineNum;
	std::string frag;

public:
	explicit errTokWriter( annotationsType& a,
						   const boost::filesystem::path& f ) 
		: annotations(a), filename(f), record(false) {}

    void newline( const char *line, int first, int last ) {
    }
    
    void token( tero::errToken token, const char *line, 
				int first, int last, bool fragment ) {
		if( fragment ) {
			frag += std::string(&line[first],last - first);
		} else {
			std::string tokval;
			if( !frag.empty() ) {
				tokval = frag + std::string(&line[first],last - first);
				frag = "";
			} else {
				tokval = std::string(&line[first],last - first);
			}
			switch( token ) {
			case tero::errFilename:
				record = ( filename == boost::filesystem::path(tokval) );
				break;;
			case tero::errLineNum:
				if( record ) {				
					lineNum = atoi(tokval.c_str());					
				}
				break;
			case tero::errMessage:
				if( record ) {
					annotations[lineNum] += tokval;
				}
				break;
			default:
                /* Nothing to do excepts shutup gcc warnings. */
                break;
			}
		}
    }
};

}; // anonymous


namespace tero {

const char *licenseCodeTitles[] = {
    "unknown",
    "MIT",
    "BSD 2-Clause",
    "BSD 3-Clause",
    "Proprietary"
};


void checker::cache() {

    static const boost::regex licenses_regex[] = {
        // MIT license
        // -----------
boost::regex(
"\\s*Copyright \\(c\\) (\\d+(?:-\\d+)?), ([^#]*)"
" Permission is hereby granted, free of charge, to any person obtaining a copy"
" of this software and associated documentation files \\(the \"Software\"\\),"
" to deal in the Software without restriction, including without limitation"
" the rights to use, copy, modify, merge, publish, distribute, sublicense,"
" and/or sell copies of the Software, and to permit persons to whom the"
" Software is furnished to do so, subject to the following conditions:"
" The above copyright notice and this permission notice shall be included in"
" all copies or substantial portions of the Software\\."
" THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR"
" IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,"
" FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT\\. IN NO EVENT SHALL THE"
" AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER"
" LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,"
" OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN"
" THE SOFTWARE\\.\\s*"),

        // BSD 2-Clause
        // ------------
        boost::regex(
"\\s*Copyright \\(c\\) (\\d+(?:-\\d+)?), ([^#]*)"
" All rights reserved\\."
" Redistribution and use in source and binary forms, with or without"
" modification, are permitted provided that the following conditions are met:"
" 1\\. Redistributions of source code must retain the above copyright notice,"
" this list of conditions and the following disclaimer\\."
" 2\\. Redistributions in binary form must reproduce the above copyright"
" notice, this list of conditions and the following disclaimer in the"
" documentation and/or other materials provided with the distribution\\."
" THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS"
" \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED"
" TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR"
" PURPOSE ARE DISCLAIMED\\. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR"
" CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,"
" EXEMPLARY, OR CONSEQUENTIAL DAMAGES \\(INCLUDING, BUT NOT LIMITED TO,"
" PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;"
" OR BUSINESS INTERRUPTION\\) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,"
" WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT \\(INCLUDING NEGLIGENCE OR"
" OTHERWISE\\) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF"
" ADVISED OF THE POSSIBILITY OF SUCH DAMAGE\\.\\s*"),

        // BSD 3-Clause
        // ------------
        boost::regex(
"\\s*Copyright \\(c\\) (\\d+(?:-\\d+)?), ([^#]*)"
" All rights reserved\\."
" Redistribution and use in source and binary forms, with or without"
" modification, are permitted provided that the following conditions are met:"
" 1\\. Redistributions of source code must retain the above copyright notice,"
" this list of conditions and the following disclaimer\\."
" 2\\. Redistributions in binary form must reproduce the above copyright"
" notice, this list of conditions and the following disclaimer in the"
" documentation and/or other materials provided with the distribution\\."
" 3\\. Neither the name of the copyright holder nor the names of its"
" contributors may be used to endorse or promote products derived from this"
" software without specific prior written permission\\."
" THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\""
" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE"
" IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE"
" ARE DISCLAIMED\\. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE"
" LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR"
" CONSEQUENTIAL DAMAGES \\(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF"
" SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS"
" INTERRUPTION\\) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN"
" CONTRACT, STRICT LIABILITY, OR TORT \\(INCLUDING NEGLIGENCE OR OTHERWISE\\)"
" ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE"
" POSSIBILITY OF SUCH DAMAGE\\.\\s*"),

        // Proprietary
        // -----------
        boost::regex(
"\\s*Copyright \\(c\\) (\\d+(?:-\\d+)?), ([^#]*)"
" All rights reserved\\.\\s*"),
    };

    boost::smatch m;
    std::string text = licenseText.str();
    for( int license = MITLicense; license < 5; ++license ) {
        if( boost::regex_match(text, m, licenses_regex[license - 1]) ) {
            licenseType = (licenseCode)license;
            dates = m.str(1);
            grantor = m.str(2);
            break;
        }
    }
    cached = true;
}


void checker::normalize( const char *line, int first, int last )
{
    std::string s(&line[first], &line[last]);
    if( state == start ) {
        boost::smatch m;
        if( !boost::regex_search(s, m, boost::regex("Copyright")) ) return;
        state = readLicense;
    }
    while( first < last ) {
        switch( state ) {
        case normalizeLicense:
            while( first < last && isspace(line[first]) ) ++first;
            state = readLicense;
            break;
        case readLicense:
            while( first < last && !isspace(line[first]) ) {
                licenseText << line[first++];
            }
            licenseText << ' ';
            state = normalizeLicense;
            break;
        default:
            /* Nothing to do except prevent gcc from complaining. */
            return;
        }
    }
}


cppChecker::cppChecker()
    : comment(emptyLine),
      tokenizer(*this)

{
}


void cppChecker::newline(const char *line, int first, int last ) {
    switch( state ) {
    case readLicense:
        licenseText << ' ';
        state = normalizeLicense;
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
        normalize(line, first, last);
        if( (state == readLicense || state == normalizeLicense)
            && !fragment ) state = doneLicense;
        /* XXX we donot mark comment lines correctly but it does not
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
        licenseText << ' ';
        state = normalizeLicense;
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
        if( first < last && line[first] == '#' ) ++first;
        normalize(line, first, last);
        if( (state == readLicense || state == normalizeLicense)
            && !fragment ) state = doneLicense;
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
    dispatchDoc::instance()->fetch(s,"check",s.asUrl(pathname));
}

void checkstyle::flush( session& s ) const
{
    if( state != start ) {
        s.out() << html::table::end
                << html::p::end;
    }
}


void checkstyleFetch( session& s, const url& name )
{
    checkstyle p;
    p.fetch(s,s.abspath(name));
}


void lintAnnotate::init( session& s,
						 const boost::filesystem::path& key,
						 std::istream& info )
{
	errTokWriter w(super::annotations,key);
	errTokenizer tok(w);
	
	char buffer[4096];
	while( !info.eof() ) {
		info.read(buffer,4096);
		tok.tokenize(buffer,info.gcount());
	}
}


lintAnnotate::lintAnnotate( session& s,
							const boost::filesystem::path& key,
							std::istream& info ) 
	: super() { 
	init(s,key,info);
}

    
lintAnnotate::lintAnnotate( session& s,
							const boost::filesystem::path& key,
							std::istream& info,
							std::basic_ostream<char>& o )
	: super(o) {
	init(s,key,info);
}

}
