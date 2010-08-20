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

#include <iostream>
#include <boost/regex.hpp>
#include <boost/filesystem/fstream.hpp>
#include "composer.hh"


void composer::embed( session& s, const std::string& value ) {
    using namespace boost::filesystem;
    try {
	dispatchDoc::instance->fetch(s,value);
    } catch( const basic_filesystem_error<path>& e ) {
	*ostr << "<p>" << e.what() << "</p>" << std::endl;
    }
}


void composer::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem;

    static const boost::regex tmplname("<!-- tmpl_name '(\\S+)' -->");
    static const boost::regex tmplvar("<!-- tmpl_var name='(\\S+)' -->");
    static const boost::regex tmplinc("<!-- tmpl_include name='(\\S+)' -->");
    
    ifstream strm;
    open(strm,fixed.empty() ? pathname : fixed);

    document* doc = dispatchDoc::instance->select("document",
						  s.valueOf("document"));
    if( doc ) {
	doc->meta(s,s.valueAsPath("document"));
    }
    *ostr << httpHeaders.contentType();

    skipOverTags(strm);
    while( !strm.eof() ) {
	smatch m;
	std::string line;
	bool found = false;
	std::getline(strm,line);
	if( regex_search(line,m,tmplname) ) {
	    std::string varname = m.str(1);
	    session::variables::const_iterator v = s.vars.find(varname);
	    if( v != s.vars.end() ) {
		*ostr << m.prefix();
		*ostr << v->second;
		*ostr << m.suffix() << std::endl;
	    }
	    found = true;
	}

	if( regex_search(line,m,tmplvar) ) {
	
	    std::string varname = m.str(1);
	    session::variables::const_iterator v = s.vars.find(varname);
	    if( v == s.vars.end() ) {
		/* hmmm ... variable wasn't set in meta? */
	    }
	    document* doc 
		= dispatchDoc::instance->select(varname,s.valueOf(varname));
	    if( doc != NULL ) {
		boost::filesystem::path docname = s.valueAsPath(varname);
		*ostr << m.prefix();
		/* \todo code could be:
		           doc->fetch(s,docname);  
			 but that would skip over the override 
			 of changediff::embed(). */
		embed(s,varname);
		*ostr << m.suffix() << std::endl;
		found = true;
	    
	    } else {
		v = s.vars.find(varname);
		if( v != s.vars.end() ) {
		    *ostr << m.prefix();
		    *ostr << s.valueOf(varname);
		    *ostr << m.suffix() << std::endl;
		    found = true;
		}
	    }
	} else if( regex_search(line,m,tmplinc) ) {
	    /* \todo fetch another template. This code should
	     really call to the dispatcher once we can sort
	     out varnames and pathnames... */
	    path incpath((fixed.empty() ? pathname.parent_path() 
			  : fixed.parent_path()) / m.str(1));
	    document* doc = dispatchDoc::instance->select("document",incpath.string());
	    if( doc != NULL ) {
		doc->fetch(s,incpath);
	    }
	    found = true;
	}
	if( !found ) {
	    *ostr << line << std::endl;
	}
    }
    strm.close();
}
