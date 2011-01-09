/* Copyright (c) 2009, Fortylines LLC
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
#include "project.hh"

/** Pages related to projects

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


void projCreate::fetch(	session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::filesystem;

    /* remove the "create" command, then derive the project name 
       as a relative path inside srcTop. */
    path projectname 
	= s.subdirpart(s.valueAsPath("srcTop"),pathname.parent_path());

    path projectDir(s.valueAsPath("srcTop") / projectname);

    if( exists(projectDir) ) {
	throw basic_filesystem_error<path>(std::string("already exists"),
					   projectDir, 
					   error_code());
    }

    /* \todo HACK: force group to true to run projCreateTest 
       while the login feature is not fully working. */
    rev->create(projectDir,true);

    ofstream index(projectDir / path("dws.xml"));
    index << "<?xml version=\"1.0\" ?>\n"
	  << "<projects>\n"
	  << "  <project name=\"" << projectname << "\">\n"
	  << "    <title></title>\n"
	  << "    <description></description>\n"
	  << "    <maintainer name=\"\" email=\"\" />\n"
	  << "    <repository>\n"
	  << "    </repository>\n"
	  << "  </project>\n"
	  << "</projects>\n";
    index.close();

    rev->add(projectDir);
    rev->commit("initial index (template)");

    *ostr << httpHeaders.refresh(0,url("/" + projectname.string() + "/dws.xml")) 
	  << std::endl;   
}


void 
projindex::addSessionVars( boost::program_options::options_description& opts )
{
    using namespace boost::program_options;
    
    options_description vars("project");
    vars.add_options()
	("remoteSrcTop",value<std::string>(),"path to root of the project repositories");
    opts.add(vars);
}


void projindex::meta( session& s, const boost::filesystem::path& pathname ) {
    name = pathname.parent_path().filename();
    s.insert("title",name);
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
	    dec.attach(*ostr);
	    for( xml_node<> *descr
		     = project->first_node("description"); descr != NULL; 
		 descr = descr->next_sibling() ) {
		*ostr << html::p()
#if 0
			  << descr->value();
#endif
		;
		for( xml_node<> *child
		     = descr->first_node(); child != NULL; 
		     child = child->next_sibling() ) {
		    *ostr << child->value();
		}
		*ostr << html::p::end;
	    }
	    dec.detach();

	    /* Information about the maintainer */
	    xml_node<> *maintainer = project->first_node("maintainer");
	    if( maintainer ) {
		xml_attribute<> *name = maintainer->first_attribute("name");
		xml_attribute<> *email = maintainer->first_attribute("email");
		if( name ) {
		    *ostr << html::p() << "maintainer: ";
		    if( email ) {
			*ostr << html::a().href(std::string("mailto:") 
						    + email->value());
		    }
		    *ostr << name->value();
		    if( email ) {
			*ostr << html::a::end;
		    }
		    *ostr << html::p::end;
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
	    *ostr << html::p();
	    if( candidates.empty() ) {
		*ostr << "There are no prepackaged archive available for download.";
	    } else {
		for( candidateSet::const_iterator c = candidates.begin();
		     c != candidates.end(); ++c ) {
		    *ostr << html::a().href(c->string()) 
			      << c->filename() << html::a::end 
			      << "<br />" << std::endl;
		}
	    }
	    *ostr << html::p::end;
	   
	    /* Dependencies to install the project from a source compilation. */
	    xml_node<> *repository = project->first_node("repository");
	    if( repository == NULL ) repository = project->first_node("patch");
	    if( repository ) {
		const char *sep = "";
		*ostr << html::p() << "The repository is available at "
			  << html::p::end;
		*ostr << html::pre()
			  << s.valueOf("remoteSrcTop") 
			  << "/" << name << "/.git"
			  << html::pre::end;
		*ostr << html::p() << "The following prerequisites are "
		    "necessary to build the project from source: ";
		for( xml_node<> *dep = repository->first_node("dep");
		     dep != NULL; dep = dep->next_sibling() ) {
		    xml_attribute<> *name = dep->first_attribute("name");
		    if( name != NULL ) {
			if( boost::filesystem::exists(s.srcDir(name->value())) ) {			
			    *ostr << sep 
				  << projhref(name->value());
			} else {
			    *ostr << sep << name->value();
			}
			sep = ", ";
		    }
		}	    
		*ostr << '.';
		*ostr << " The easiest way to install prerequisites,"
			  << " download the source tree locally,"
			  << " make and install the binaries is to issue"
			  << " a global " << html::a().href("/log")
			  << "build" << html::a::end << " command."; 
		*ostr << html::p::end;
		*ostr << html::p();
		*ostr << "You can then later update the local copy"
		    " of the source tree, re-build the prerequisites re-make"
		    " and re-install the binaries with the following commands:";
		*ostr << html::p::end;
		*ostr << html::pre();
		*ostr << "cd *buildTop*/" << name << std::endl;		
		*ostr << html::a().href("/resources/dws") 
			  << "dws" << html::a::end 
			  << " make recurse" << std::endl;
		*ostr << "dws make install" << std::endl;
		*ostr << html::pre::end;
	    }
	}
    }
}
