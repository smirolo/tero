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

#include <boost/filesystem/fstream.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "logview.hh"
#include "markup.hh"

void logview::fetch( session& s, const boost::filesystem::path& pathname ) {

    using namespace rapidxml;
    using namespace boost::filesystem;

    /* Each log contains the results gathered on a build server. We prefer
       to present one build results per column but need to write std::cout
       one table row at a time so we create the table entirely in memory 
       before we display it. 
       If we were to present build results per row, we would still need
       to preprocess the log files once but only to determine the column
       headers. */


    /* build as column headers */
    typedef std::set<path> colHeadersType;
    colHeadersType colHeaders;

    /* (project,(build,status))  */
    typedef std::map<path,std::string> colType;
    typedef std::map<std::string,colType> tableType;
    tableType table;

    path dirname(s.abspath(is_directory(pathname) ?
			   pathname : pathname.parent_path()));
    for( directory_iterator entry = directory_iterator(dirname); 
	 entry != directory_iterator(); ++entry ) {
	path filename(entry->filename());	
	if( filename.extension() == ".log" ) {
	    /* \todo because we check the extension ".log", we will
	       also pick-up the empty file that is used to dispatch
	       into this method. Needs fixing of the design? */
	    size_t fileSize = file_size(*entry);
	    char text[ fileSize + 1 ];
	    ifstream file(*entry);
	    if( file.fail() ) {
		boost::throw_exception(basic_filesystem_error<path>(
				       std::string("error opening file"),
				       *entry, 
				       boost::system::error_code()));
	    }
	    file.read(text,fileSize);
	    text[fileSize] = '\0';
	    xml_document<> doc;    // character type defaults to char
	    doc.parse<0>(text);     // 0 means default parse flags

	    colHeaders.insert(filename);
	    xml_node<> *root = doc.first_node();
	    if( root != NULL ) {
		for( xml_node<> *project = root->first_node("section");
		     project != NULL; project = project->next_sibling() ) {
		    xml_attribute<> *name = project->first_attribute("id");
		    if( name != NULL ) {
			xml_node<> *status = project->first_node("status");
			if( status != NULL ) {
			    table[name->value()][filename] = status->value();
			}
		    }
		}	
	    }
	}
    }


    /* Let's write the table, one row at a time. */
    std::cout << "<table>" << std::endl;
    std::cout << "<tr>" << std::endl;
    std::cout << "<th></th>";
    for( colHeadersType::const_iterator col = colHeaders.begin();
	 col != colHeaders.end(); ++col ) {
	std::cout << "<th>" << *col << "</th>";
    }
    std::cout << "</tr>" << std::endl;
    for( tableType::const_iterator row = table.begin();
	 row != table.end(); ++row ) {
	std::cout << "<tr>" << std::endl;
	std::cout << "<th>" << projhref(row->first) << "</th>" << std::endl;
	for( colHeadersType::const_iterator col = colHeaders.begin();
	     col != colHeaders.end(); ++col ) {
	    colType::const_iterator value = row->second.find(*col);
	    if( value != row->second.end() ) {
		std::cout << "<td>" << value->second << "</td>" << std::endl;	
	    } else {
		std::cout << "<td>" << "</td>" << std::endl;
	    }
	}
	std::cout << "</tr>" << std::endl;
    }
    std::cout << "</table>" << std::endl;
    std::cout << emptyParaHack;
}


void regressions::fetch( session& s, const boost::filesystem::path& pathname ) {

    using namespace rapidxml;
    using namespace boost::filesystem;

    size_t fileSize = file_size(pathname);
    char text[ fileSize + 1 ];
    ifstream file;
    std::cerr << " regressions::fetch(pathname=" << pathname 
	      << ")" << std::endl;
    open(file,pathname);
    file.read(text,fileSize);
    text[fileSize] = '\0';
    file.close();
    std::cerr << "file has been read" << std::endl;

    xml_document<> doc;    // character type defaults to char
    doc.parse<0>(text);     // 0 means default parse flags


    xml_node<> *root = doc.first_node();
    if( root != NULL ) {
	size_t col = 1;
	typedef std::map<std::string,size_t> colMap;
	colMap colmap;	
	std::cout << "<table border=\"2\">" << std::endl;
	std::cout << html::tr();
	std::cout << html::th() << html::th::end;
	for( xml_node<> *ref = root->first_node("reference");
	     ref != NULL; ref = ref->next_sibling() ) {
	    xml_attribute<> *id = ref->first_attribute("id");
	    if( id != NULL ) {		
		std::cout << html::th() << id->value() << html::th::end;
		xml_attribute<> *name = ref->first_attribute("name");
		assert( name != NULL );
		colmap.insert(std::make_pair(name->value(),col++));
	    }
	}
	std::cout << html::tr::end;

	xml_node<>* cols[colmap.size() + 1];
	for( xml_node<> *test = root->first_node("test");
	     test != NULL; test = test->next_sibling() ) {
	    std::cout << html::tr();
	    memset(cols,0,sizeof(cols));
	    xml_attribute<> *name = test->first_attribute("name");
	    if( name != NULL ) {
		std::cerr << "!!! " << name->value() << std::endl;
		std::cout << html::td() << name->value() << html::td::end;
	    }
	    for( xml_node<> *compare = test->first_node("compare");
		 compare != NULL; compare = compare->next_sibling("compare") ) {
		xml_attribute<> *name = compare->first_attribute("name");
		if( name != NULL ) {
		    std::cerr << "compare " << name->value() << std::endl;
		    colMap::const_iterator found = colmap.find(name->value());
		    if( found != colmap.end() ) {
			std::cerr << "compare cols[" << found->second << "] = "
				  << compare->value() << std::endl;
			cols[found->second] = compare;
		    } else {
			cols[0] = compare;
			std::cerr << "compare cols[0] = "
				  << compare->value() << std::endl;
		    }
		}
	    }
	    for( xml_node<> **c = &cols[1]; 
		 c != &cols[colmap.size() + 1]; ++c ) {
		std::cout << html::td();
		if( *c != NULL )
		    std::cout << (*c)->value();
		std::cout << html::td::end;
      	    }
	    std::cout << html::tr::end;
#if 0
	    std::cout << html::tr();
	    memset(cols,0,sizeof(cols));
	    for( xml_node<> **c = cols; c != &cols[colmap.size() + 1]; ++c ) {
		assert( *c == NULL );
	    }
	    for( xml_node<> *output = test->first_node("output");
		 output != NULL; output = output->next_sibling("output") ) {
		xml_attribute<> *name = output->first_attribute("name");
		if( name != NULL ) {
		    colMap::const_iterator found = colmap.find(name->value());
		    xml_node<> *data = output;
		    if( output->first_node() != NULL ) {
			data = output->first_node();
		    }
		    if( found != colmap.end() ) {
			cols[found->second] = data;
		    } else {
			cols[0] = data;
		    }
		}
	    }
	    std::cerr << std::endl;
	    for( xml_node<> **c = cols; c != &cols[colmap.size() + 1]; ++c ) {
		std::cout << html::td() << html::pre();
		if( *c != NULL )
		    std::cout << (*c)->value();
		std::cout << html::pre::end << html::td::end;
	    }
	    std::cout << html::tr::end;
#endif
	}	

	std::cout << "</table>" << std::endl;	
    }   
}
