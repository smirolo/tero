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

    path dirname(is_directory(pathname) ? pathname : pathname.parent_path());
    for( directory_iterator entry = directory_iterator(dirname); 
	 entry != directory_iterator(); ++entry ) {
	path p(*entry);
	if( p.extension() == ".log" ) {
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

	    colHeaders.insert(*entry);
	    xml_node<> *root = doc.first_node();
	    if( root != NULL ) {
		for( xml_node<> *project = root->first_node("section");
		     project != NULL; project = project->next_sibling() ) {
		    xml_attribute<> *name = project->first_attribute("id");
		    if( name != NULL ) {
			xml_node<> *status = project->first_node("status");
			if( status != NULL ) {
			    table[name->value()][*entry] = status->value();
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
	std::cout << "<th>" << row->first << "</th>" << std::endl;
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
}

