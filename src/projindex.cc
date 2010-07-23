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
#include "projindex.hh"
#include "slice.hh"
#include "markup.hh"
#include "decorator.hh"


void checkstyle::addDir( const session& s, 
			 const boost::filesystem::path& pathname ) {
}


void checkstyle::addFile( const session& s, 
			  const boost::filesystem::path& pathname ) {
    using namespace boost::filesystem; 

    if( state == start ) {
	std::cout << htmlContent;
	std::cout << html::p() 
		  << "<table>";
	std::cout << html::tr()
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
	std::cout << "</table>"
		  << html::p::end;
    }
}


void projindex::meta( session& s, const boost::filesystem::path& pathname ) {
    name = pathname.parent_path().filename();
    s.vars["title"] = name;
    document::meta(s,pathname);
}



void projindex::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace rapidxml;
    using namespace boost::filesystem;

    size_t fileSize = file_size(pathname);
    char text[ fileSize + 1 ];
    ifstream file;
    open(file,pathname);
    file.read(text,fileSize);
    text[fileSize] = '\0';
    xml_document<> doc;    // character type defaults to char
    doc.parse<0>(text);     // 0 means default parse flags

    xml_node<> *root = doc.first_node();
    if( root != NULL ) {
	xml_attribute<> *projname = NULL;
	for( xml_node<> *project = root->first_node("project");
	     project != NULL; project = project->next_sibling() ) {

	    projname = project->first_attribute("name");

	    /* Description of the project */
	    hrefLight dec(s);
	    dec.attach(std::cout);
	    for( xml_node<> *descr
		     = project->first_node("description"); descr != NULL; 
		 descr = descr->next_sibling() ) {
		std::cout << html::p()
#if 0
			  << descr->value();
#endif
		;
		for( xml_node<> *child
		     = descr->first_node(); child != NULL; 
		     child = child->next_sibling() ) {
		    std::cout << child->value();
		}
		std::cout << html::p::end;
	    }
	    dec.detach();

	    /* Information about the maintainer */
	    xml_node<> *maintainer = project->first_node("maintainer");
	    if( maintainer ) {
		xml_attribute<> *name = maintainer->first_attribute("name");
		xml_attribute<> *email = maintainer->first_attribute("email");
		if( name ) {
		    std::cout << html::p() << "maintainer: ";
		    if( email ) {
			std::cout << html::a().href(std::string("mailto:") 
						    + email->value());
		    }
		    std::cout << name->value();
		    if( email ) {
			std::cout << html::a::end;
		    }
		    std::cout << html::p::end;
		}
	    }

	    /* Shows the archives that can be downloaded. */
	    const char *dists[] = {
		"resources/Darwin",
		"resources/Fedora",
		"resources/Ubuntu",
		"resources/srcs" 
	    };
	    typedef std::vector<path> candidateSet;
	    candidateSet candidates;
	    for( const char **d = dists; 
		 d != &dists[sizeof(dists)/sizeof(char*)]; ++d ) {
		path dirname(s.valueOf("siteTop") + "/" + std::string(*d));
		path prefix(dirname / std::string(projname->value()));
		if( boost::filesystem::exists(dirname) ) {
		    for( directory_iterator entry = directory_iterator(dirname); 
			 entry != directory_iterator(); ++entry ) {
			if( entry->string().compare(0,prefix.string().size(),
						    prefix.string()) == 0 ) {
			    candidates.push_back(path("/") / path(*d) / entry->filename());
			}
		    }
		}
	    }
	    std::cout << html::p();
	    if( candidates.empty() ) {
		std::cout << "There are no prepackaged archive available for download.";
	    } else {
		for( candidateSet::const_iterator c = candidates.begin();
		     c != candidates.end(); ++c ) {
		    std::cout << html::a().href(c->string()) 
			      << c->filename() << html::a::end 
			      << "<br />" << std::endl;
		}
	    }
	    std::cout << html::p::end;
	   
	    /* Dependencies to install the project from a source compilation. */
	    xml_node<> *repository = project->first_node("repository");
	    if( repository == NULL ) repository = project->first_node("patch");
	    if( repository ) {
		const char *sep = "";
		std::cout << html::p() << "The repository is available at "
			  << html::p::end;
		std::cout << html::pre()
			  << s.valueOf("remoteSrcTop") 
			  << "/" << name << "/.git"
			  << html::pre::end;
		std::cout << html::p() << "The following prerequisites are "
		    "necessary to build the project from source: ";
		for( xml_node<> *dep = repository->first_node("dep");
		     dep != NULL; dep = dep->next_sibling() ) {
		    xml_attribute<> *name = dep->first_attribute("name");
		    if( name != NULL ) {
			if( boost::filesystem::exists(s.srcDir(name->value())) ) {			
			    std::cout << sep 
				      << html::a().href(std::string("/") 
							+ name->value()
							+ "/index.xml") 
				      << name->value()
				      << html::a::end;
			} else {
			    std::cout << sep << name->value();
			}
			sep = ", ";
		    }
		}	    
		std::cout << '.';
		std::cout << " The easiest way to install prerequisites,"
			  << " download the source tree locally,"
			  << " make and install the binaries is to issue"
			  << " a global " << html::a().href("/log")
			  << "build" << html::a::end << " command."; 
		std::cout << html::p::end;
		std::cout << html::p();
		std::cout << "You can then later update the local copy"
		    " of the source tree, re-build the prerequisites re-make"
		    " and re-install the binaries with the following commands:";
		std::cout << html::p::end;
		std::cout << html::pre();
		std::cout << "cd *buildTop*/" << name << std::endl;		
		std::cout << html::a().href("/resources/dws") 
			  << "dws" << html::a::end 
			  << " make recurse" << std::endl;
		std::cout << "dws make install" << std::endl;
		std::cout << html::pre::end;
	    }
	}
    }
}
