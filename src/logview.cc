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

#include <boost/filesystem/fstream.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "logview.hh"
#include "markup.hh"
#include "project.hh"

/** Pages related to logs.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

void 
logview::addSessionVars( boost::program_options::options_description& opts )
{
    using namespace boost::program_options;
    
    options_description vars("logview");
    vars.add_options()
	("remoteIndexFile",value<std::string>(),"path to project interdependencies")
        ("indexFile",value<std::string>(),"path to project interdependencies");
    opts.add(vars);
}


void logview::fetch( session& s, const boost::filesystem::path& pathname ) const {

    using namespace rapidxml;
    using namespace boost::filesystem;

    /* Each log contains the results gathered on a build server. We prefer
       to present one build results per column but need to write s.out()
       one table row at a time so we create the table entirely in memory 
       before we display it. 
       If we were to present build results per row, we would still need
       to preprocess the log files once but only to determine the column
       headers. */
    static const boost::regex logPat("([^-]+)-.*\\.log");
    
    /* build as column headers */
    typedef std::set<path> colHeadersType;
    colHeadersType colHeaders;

    /* (project,(build,[status,exitCode]))  */
    typedef std::map<path,std::pair<std::string,int> > colType;
    typedef std::map<std::string,colType> tableType;
    tableType table;

    path srcTop(s.valueOf("srcTop"));
    boost::filesystem::path srcBase(boost::filesystem::path("/") 
				    / s.subdirpart(s.valueOf("siteTop"),
						   s.valueOf("srcTop")));

#if 0
    /* Populate the project names based on the directories in *srcTop*
       which hold a repository control subdirectory (.git, .svn, etc.). */
    for( directory_iterator entry = directory_iterator(srcTop); 
	 entry != directory_iterator(); ++entry ) {
	path repcontrol((srcTop / entry->filename()) / ".git");
	if( boost::filesystem::exists(repcontrol) ) {
	    table[entry->filename()] = colType();
	}
    }
#else
    /* Populate the project names based on the projects specified
       in the index file. */
    size_t fileSize = file_size(s.valueOf("indexFile"));
    char text[ fileSize + 1 ];
    ifstream file(s.valueOf("indexFile"));
    if( file.fail() ) {
	boost::throw_exception(basic_filesystem_error<path>(
		std::string("error opening file"),
		s.valueOf("indexFile"), 
		boost::system::error_code()));
    }
    file.read(text,fileSize);
    text[fileSize] = '\0';
    xml_document<> doc;    // character type defaults to char
    doc.parse<0>(text);     // 0 means default parse flags
    
    xml_node<> *root = doc.first_node();
    if( root != NULL ) {
	for( xml_node<> *project = root->first_node("project");
	     project != NULL; project = project->next_sibling("project") ) {
	    xml_attribute<> *name = project->first_attribute("name");
	    if( name != NULL ) {
		path repcontrol((srcTop / path(name->value())) / ".git");
		if( boost::filesystem::exists(repcontrol) ) {
		    table[name->value()] = colType();
		}
	    }
	}
    }
#endif
       

    path dirname(s.abspath(is_directory(pathname) ?
			   pathname : pathname.parent_path()));

    std::string logBase;
    for( directory_iterator entry = directory_iterator(dirname); 
	 entry != directory_iterator(); ++entry ) {
	boost::smatch m;
	path filename(entry->filename());	
	if( boost::regex_search(entry->filename(),m,logPat) ) {
	    logBase = m.str(1);
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
			    int exitCode = 0;
			    xml_attribute<> *exitAttr 
				= status->first_attribute("error");
			    if( exitAttr ) {
				exitCode = atoi(exitAttr->value());
			    }
			    table[name->value()][filename] 
				= std::make_pair(status->value(),exitCode);
			}
		    }
		}	
	    }
	}
    }

    s.out() << html::p() << "This build view page shows the stability "
	"of all projects at a glance. Each column represents a build log "
	"obtained by running the following command on a local machine:" 
	      << html::p::end;
    s.out() << html::pre() << html::a().href("/resources/dws") 
	      << "dws" << html::a::end << " build " 
	      << s.valueOf("remoteIndexFile")
	      << html::pre::end;
    s.out() << html::p() << "dws, the inter-project dependency tool "
	"generates a build log file with XML markups as part of building "
	"projects. That build log is then stamped with the local machine "
	"hostname and time before being uploaded onto the remote machine. "
	"This page presents all build logs currently available on the remote "
	"machine." << html::p::end;

    /* Display the table column headers. */
    if( colHeaders.empty() ) {
	s.out() << html::pre() << "There are no logs available"
	      << html::pre::end;
    } else {
	s.out() << html::table();
	s.out() << html::tr();
	s.out() << html::th() << html::th::end;	
	for( colHeadersType::const_iterator col = colHeaders.begin();
	     col != colHeaders.end(); ++col ) {
	    s.out() << html::th() 
		    << html::a().href(col->string())
		    << *col << html::a::end << html::th::end;
	}
	s.out() << html::tr::end;

	/* Display one project per row, one build result per column. */
	for( tableType::const_iterator row = table.begin();
	     row != table.end(); ++row ) {
	    s.out() << html::tr();
	    s.out() << html::th() << projhref(srcBase,row->first) 
		    << html::th::end;
	    for( colHeadersType::const_iterator col = colHeaders.begin();
		 col != colHeaders.end(); ++col ) {
		colType::const_iterator value = row->second.find(*col);
		if( value != row->second.end() ) {
		    if( value->second.second ) {
			s.out() << html::td().classref("positiveErrorCode");
		    } else {
			s.out() << html::td();
		    }
		    s.out() << value->second.first;	
		} else {
		    s.out() << html::td();
		}
		s.out() << html::td::end;
	    }
	    s.out() << html::tr::end;
	}
	s.out() << html::table::end;
    }

    /* footer */
    s.out() << html::p() << "Each cell contains the time it took to make "
	  "a specific project. If make exited with an error code before "
	"the completion of the last target, it is marked " 
	      << html::span().classref("positiveErrorCode") 
	      << "&nbsp;as such&nbsp;" << html::span::end << "." 
	      << html::p::end;
    s.out() << emptyParaHack;
}


void regressions::fetch( session& s, const boost::filesystem::path& pathname ) const {

    using namespace rapidxml;
    using namespace boost::filesystem;

    if( !boost::filesystem::exists(pathname) ) {
	s.out() << html::p()
		  << "There are no regression logs available for the unit tests."
		  << html::p::end;
	return;
    }

    size_t fileSize = file_size(pathname);
    char text[ fileSize + 1 ];
    ifstream file;
    open(file,pathname);
    file.read(text,fileSize);
    text[fileSize] = '\0';
    file.close();
    xml_document<> doc;    // character type defaults to char
    doc.parse<0>(text);     // 0 means default parse flags


    xml_node<> *root = doc.first_node();
    if( root != NULL ) {
	size_t col = 1;
	typedef std::map<std::string,size_t> colMap;
	colMap colmap;	
	s.out() << html::p();
	s.out() << html::table() << std::endl;
	s.out() << html::tr();

	s.out() << html::th();
	xml_node<> *config = root->first_node("config");
	if( config ) {
	    xml_attribute<> *configName = config->first_attribute("name");
	    if( configName ) {
		s.out() << configName->value();
	    }
	}       
	s.out() << " vs." << html::th::end;
	
	xml_node<> *ref = root->first_node("reference");
	if( ref ) {
	    for( ; ref != NULL; ref = ref->next_sibling() ) {
		xml_attribute<> *id = ref->first_attribute("id");
		if( id != NULL ) {		
		    s.out() << html::th() << id->value() << html::th::end;
		    xml_attribute<> *name = ref->first_attribute("name");
		    assert( name != NULL );
		    colmap.insert(std::make_pair(name->value(),col++));
		}
	    }
	} else {
	    s.out() << html::th() << "No results to compare against."
		      << html::th::end;
	}
	s.out() << html::tr::end;

	xml_node<>* cols[colmap.size() + 1];
	for( xml_node<> *test = root->first_node("test");
	     test != NULL; test = test->next_sibling() ) {
	    s.out() << html::tr();
	    memset(cols,0,sizeof(cols));
	    xml_attribute<> *name = test->first_attribute("name");
	    if( name != NULL ) {
		s.out() << html::td() << name->value() << html::td::end;
	    }
	    for( xml_node<> *compare = test->first_node("compare");
		 compare != NULL; compare = compare->next_sibling("compare") ) {
		xml_attribute<> *name = compare->first_attribute("name");
		if( name != NULL ) {
		    colMap::const_iterator found = colmap.find(name->value());
		    if( found != colmap.end() ) {
			cols[found->second] = compare;
		    } else {
			cols[0] = compare;
		    }
		}
	    }
	    for( xml_node<> **c = &cols[1]; 
		 c != &cols[colmap.size() + 1]; ++c ) {
		s.out() << html::td();
		if( *c != NULL )
		    s.out() << (*c)->value();
		s.out() << html::td::end;
      	    }
	    s.out() << html::tr::end;
#if 0
	    /* display the actual ouput as an expandable row. */
	    s.out() << html::tr();
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
	    for( xml_node<> **c = cols; c != &cols[colmap.size() + 1]; ++c ) {
		s.out() << html::td() << html::pre();
		if( *c != NULL )
		    s.out() << (*c)->value();
		s.out() << html::pre::end << html::td::end;
	    }
	    s.out() << html::tr::end;
#endif
	}	

	s.out() << html::table::end << html::p::end;
    }   
}
