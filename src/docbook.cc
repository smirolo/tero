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

#include "docbook.hh"
#include "bookParser.hh"


namespace {

    void parseInfo( session& s, const rapidxml::xml_node<>& r ) {
	using namespace rapidxml;

	/* We found an <info> tag, let's parse the meta information 
	   about the article such as the author, the date, etc. */
	for( xml_node<> *n = r.first_node();
	     n != NULL; n = r.next_sibling() ) {
	    switch( n->type() ) {
	    case node_element:
		/* author, title, date. */
		s.vars[n->name()] = n->value();
	    }
	}
	if( s.vars.find("title") == s.vars.end() ) {
	    s.vars["title"] = s.valueOf("document");
	}
    }
#if 0
    void parseSection( const rapid_xml::xml_node<>& r, int level = 1 ) {
	/* We found a <section> tag, let's simply rewrite the title
	   s <h?> tags and the <para> as <p> tags. */
	for( xml_node<> *n = r.first_node();
	     n != NULL; n = r.next_sibling() ) {
	    switch( n->type() ) {
	    case node_element:
		if( n->name() == "title" ) {
		    std::cout << html::h(level) 
			      << n->value() 
			      << html::h(level).end();
		} else if( n->name() == "para" ) {
		    std::cout << html::p << n->value() << html::p::end;
		} else if( n->name() == "section" ) {
		    parseSection(*n,level + 1);
		} else if( n->name() == "simplelist" ) {
		    parseSimpleList(*n);
		}

#if 0
	    case node_data:
		/* A data node already contains escape codes 
		   for special characters. */
		std::cout << n.value();
		break;
	    case node_cdata:        
		/* CDATA nodes might contain characters 
		   that should be escaped. */
		std::cout << n.value();
		break;
#endif
	    }
	}
    }
#endif

}  // anonymous namespace


docbook::docbook( decorator& l,  decorator& r ) 
    : text(l,r), buffer(NULL) {}

docbook::~docbook() {
    if( buffer != NULL ) delete [] buffer;
}


void docbook::meta( session& s, const boost::filesystem::path& pathname ) {
    using namespace rapidxml;

    /* We need the meta information at this point but will only print 
       the formatted text when fetch() is called later on.
       We load the text *buffer* and keep it around in order to parse 
       the XML only once. */
    size_t fileSize = file_size(pathname);
    buffer = new char [ fileSize + 1 ];
    boost::filesystem::ifstream file;

    open(file,pathname);
    file.read(buffer,fileSize);
    buffer[fileSize] = '\0';
    file.close();

    doc.parse<0>(buffer);

    xml_node<> *root = doc.first_node();
    if( root != NULL ) {
	xml_node<> *info = root->first_node("info");
	if( info != NULL ) {
	    parseInfo(s,*info);
	}
    }
}


void docbook::fetch( session& s, const boost::filesystem::path& pathname )
{
    boost::filesystem::ifstream file;

    open(file,pathname);
    docbookScanner tok(file);
    docbookParser parser(tok,*this);    
    parser.llsection();
}
