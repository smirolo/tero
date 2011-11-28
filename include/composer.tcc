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


/** Replace all <!-- widget ... --> statements in a template file
    by the appropriate html.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

template<const char *layoutPtr>
void compose( session& s, const boost::filesystem::path& pathname )
{
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem;

    static const boost::regex tmpl("<!-- widget '(\\S+)'(\\s+name='(\\S+)')?(\\s+value='(\\S+)')? -->");

    ifstream strm;
    std::string layout(layoutPtr);
    path fixed(themeDir.value(s) / (!layout.empty() ? (layout + ".template")
				    : pathname.filename()));

    s.openfile(strm,fixed);
    skipOverTags(s,strm);
    while( !strm.eof() ) {
	smatch m;
	std::string line;
	bool found = false;
	std::getline(strm,line);
	if( regex_search(line,m,tmpl) ) {
	    std::string widget = m.str(1);
	    std::string name = m.str(3);
	    if( name.empty() ) name = widget;	    
	    std::string value = m.str(5);
	    if( value.empty() ) {
			/* By default if a variable does not have a value, use the value
			   of "document". */
			session::variables::const_iterator look = s.find(name);
			if( !s.found(look) ) {    
				s.insert(name,document.value(s).string());
				look = s.find(name);
			}
			value = look->second.value;
	    }
		
	    found = true;
	    s.out() << m.prefix();
	    std::ostream& prevDisp = s.out();
	    try {
			path prev = current_path();
			current_path(s.prefixdir(fixed));
			dispatchDoc::instance()->fetch(s,widget,url(value));
			current_path(prev);
	    } catch( const std::runtime_error& e ) {
			s.out(prevDisp);
			s.feeds = NULL; /* ok here since those objects were
							   allocated on the stack. */
			++s.nErrs;
			std::cerr << "[embed of '" << value << "'] " 
					  << e.what() << std::endl;	
			s.out() << "<p>" << e.what() << "</p>" << std::endl;
	    }
	    s.out() << m.suffix() << std::endl;
	}
	if( !found ) {
	    s.out() << line << std::endl;
	}
    }
    strm.close();
}
