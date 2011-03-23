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

/** Embed content associated to *variable* in the output stream.

    Implementation note:

    *embed* is referenced from the template function *composeh* so a prototype 
    has to be available before its implementation.
 */
void embed( session& s, const std::string& variable );


template<const char *layout>
void compose( session& s, const boost::filesystem::path& pathname )
{
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem;

    static const boost::regex tmplname("<!-- tmpl_name '(\\S+)' -->");
    static const boost::regex tmplvar("<!-- tmpl_var name='(\\S+)' -->");
    static const boost::regex tmplinc("<!-- tmpl_include name='(\\S+)' -->");

    ifstream strm;
    path fixed(themeDir.value(s)
	       / (std::string(layout) + ".template"));
    s.openfile(strm,fixed.empty() ? pathname : fixed);

    skipOverTags(s,strm);
    while( !strm.eof() ) {
	smatch m;
	std::string line;
	bool found = false;
	std::getline(strm,line);
	if( regex_search(line,m,tmplname) ) {
	    std::string varname = m.str(1);
	    session::variables::const_iterator v = s.find(varname);
	    s.out() << m.prefix();
	    if( s.found(v) ) {		
		s.out() << v->second.value;		
	    } else {
		s.out() << varname << " not found!";
	    }
	    s.out() << m.suffix() << std::endl;
	    found = true;
	}

	if( regex_search(line,m,tmplvar) ) {
	    found = true;
	    s.out() << m.prefix();
	    embed(s,m.str(1));					    
	    s.out() << m.suffix() << std::endl;
	} else if( regex_search(line,m,tmplinc) ) {
	    path incpath((fixed.empty() ? pathname.parent_path() 
			  : fixed.parent_path()) / m.str(1));
	    dispatchDoc::instance()->fetch(s,"document",incpath.string());
	    found = true;
	}
	if( !found ) {
	    s.out() << line << std::endl;
	}
    }
    strm.close();
}
