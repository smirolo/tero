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
#include "project.hh"
#include "revsys.hh"
#include "decorator.hh"

/** Pages related to projects

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


pathVariable srcTop("srcTop","path to document top");


void 
projectAddSessionVars( boost::program_options::options_description& all,
		       boost::program_options::options_description& visible )
{
    using namespace boost::program_options;
    
    options_description localOpts("project");
    localOpts.add(srcTop.option());
    all.add(localOpts);
    visible.add(localOpts);
}


projhref::projhref( const session& s, const std::string& n ) 
    : name(n), base(boost::filesystem::path("/") 
		    / s.subdirpart(siteTop.value(s),
				   srcTop.value(s)))
{
}


boost::filesystem::path 
projectName( const session& s, const boost::filesystem::path& p ) {
    boost::filesystem::path base(p);
    while( !base.string().empty() && !is_directory(base) ) {
		base.remove_leaf();
    }
    std::string projname = s.subdirpart(srcTop.value(s),base).string();
    if( projname[projname.size() - 1] == '/' ) {
		projname = projname.substr(0,projname.size() - 1);
    }
    return projname;
}

void projectTitle( session& s, const boost::filesystem::path& pathname )
{
    s.insert("title",projectName(s,pathname).string());
    session::variables::const_iterator look = s.find("title");
    if( s.found(look) ) {    
	s.out() << look->second.value;
    } else {
	s.out() << pathname;
    }
}


void projCreateFetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace boost::system;
    using namespace boost::filesystem;

    /** revision system to use to create the project. */
    revisionsys* rev = revisionsys::findRevByMetadir(s,".git");

    /* remove the "create" command, then derive the project name 
       as a relative path inside srcTop. */
    path projectname 
	= s.subdirpart(s.valueAsPath("srcTop"),pathname.parent_path());

    path projectDir(s.valueAsPath("srcTop") / projectname);

    if( exists(projectDir) ) {
	throw system_error(error_code(),std::string("already exists") +
			   projectDir.string());
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

    httpHeaders.refresh(0,url("/" + projectname.string() + "/dws.xml"));
}


void projindexFetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace RAPIDXML;
    using namespace boost::filesystem;

    s.check(pathname);

    path projdir = pathname.parent_path().filename();
    slice<char> text = s.loadtext(pathname);
    xml_document<> doc;    // character type defaults to char
    doc.parse<0>(text.begin());     // 0 means default parse flags

    xml_node<> *root = doc.first_node();
    if( root != NULL ) {
	xml_attribute<> *projname = NULL;
	for( xml_node<> *project = root->first_node("project");
	     project != NULL; project = project->next_sibling("project") ) {

	    projname = project->first_attribute("name");
	    
	    /* Description of the project */
	    hrefLight dec(s);
	    dec.attach(s.out());
	    for( xml_node<> *descr
		     = project->first_node("description"); descr != NULL; 
		 descr = descr->next_sibling("description") ) {
		s.out() << html::p()
#if 0
			  << descr->value();
#endif
		;
		for( xml_node<> *child
		     = descr->first_node(); child != NULL; 
		     child = child->next_sibling() ) {
		    s.out() << child->value();
		}
		s.out() << html::p::end;
	    }
	    dec.detach();

	    /* Information about the maintainer */
	    xml_node<> *maintainer = project->first_node("maintainer");
	    if( maintainer ) {
		xml_attribute<> *name = maintainer->first_attribute("name");
		xml_attribute<> *email = maintainer->first_attribute("email");
		if( name ) {
		    s.out() << html::p() << "maintainer: ";
		    if( email ) {
			s.out() << html::a().href(std::string("mailto:") 
						    + email->value());
		    }
		    s.out() << name->value();
		    if( email ) {
			s.out() << html::a::end;
		    }
		    s.out() << html::p::end;
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
		path dirname(siteTop.value(s) / std::string(*d));
		path prefix(dirname / std::string(projname->value()));
		if( boost::filesystem::exists(dirname) ) {
		    for( directory_iterator entry = directory_iterator(dirname); 
			 entry != directory_iterator(); ++entry ) {
			path p(*entry);
			if( p.string().compare(0,prefix.string().size(),
						    prefix.string()) == 0 ) {
			    candidates.push_back(path("/") / path(*d) / *entry);
			}
		    }
		}
	    }
	    s.out() << html::p();
	    if( candidates.empty() ) {
		s.out() << "There are no prepackaged archive available for download.";
	    } else {
		for( candidateSet::const_iterator c = candidates.begin();
		     c != candidates.end(); ++c ) {
		    s.out() << html::a().href(c->string()) 
			      << c->filename() << html::a::end 
			      << "<br />" << std::endl;
		}
	    }
	    s.out() << html::p::end;

	    /* Dependencies to install the project from a source compilation. */
	    xml_node<> *repository = project->first_node("repository");
	    if( repository == NULL ) repository = project->first_node("patch");
	    if( repository ) {
		const char *sep = "";
		s.out() << html::p() << "The repository is available at "
			  << html::p::end;
		s.out() << html::pre()
			<< "http://" << domainName.value(s) 
			<< "/reps/" << projhref(s,projdir.string()) << "/.git"
			<< html::pre::end;
		s.out() << html::p() << "The following prerequisites are "
		    "necessary to build the project from source: ";
		for( xml_node<> *dep = repository->first_node("dep");
		     dep != NULL; dep = dep->next_sibling() ) {
		    xml_attribute<> *name = dep->first_attribute("name");
		    if( name != NULL ) {
			if( boost::filesystem::exists(srcTop.value(s) 
						      / name->value()) ) {
			    s.out() << sep 
				    << projhref(s,name->value());
			} else {
			    s.out() << sep << name->value();
			}
			sep = ", ";
		    }
		}	    
		s.out() << '.';
		s.out() << " All the necessary steps to build and install from source"
				<< " can also be executed in a single "
				<< html::a().href("/log/") << "build" << html::a::end 
				<< " command."; 
		s.out() << html::p::end;
		s.out() << html::p();
		s.out() << "You can then later update the local copy"
		    " of the source tree, re-build the prerequisites re-make"
		    " and re-install the binaries with the following commands:";
		s.out() << html::p::end;
		s.out() << html::pre();
		s.out() << "cd *buildTop*/" << projname->value() << std::endl;
		s.out() << "dws make recurse" << std::endl;
		s.out() << "dws make install" << std::endl;
		s.out() << html::pre::end;
	    }
	}
    }
}


bool projfiles::selects( const boost::filesystem::path& pathname ) const {
    return dispatchDoc::instance()->select("check",pathname.string()) != NULL;
}


void 
projfiles::addDir( session& s, const boost::filesystem::path& dir ) const {
    switch( state ) {
    case start:
	s.out() << html::div().classref("MenuWidget");
	break;
    case toplevelFiles:
    case direntryFiles:
	s.out() << html::p::end;
	break;	
    }
    state = direntryFiles;

    std::string href = dir.string();
    if( href.compare(0,srcTop.value(s).string().size(),
		     srcTop.value(s).string()) == 0 ) {
	href = (s.subdirpart(srcTop.value(s),dir) / std::string("dws.xml")).string();
    }
    if( boost::filesystem::exists(dir / std::string("dws.xml")) ) {
	s.out() << html::a().href(href) 
			<< html::h(2)
			<< dir.leaf().string()
			<< html::h(2).end()
			<< html::a::end;
    } else {
		s.out() << html::h(2) << dir.leaf() << html::h(2).end();
    }
    s.out() << html::p();
}


void 
projfiles::addFile( session& s, const boost::filesystem::path& file ) const {
    if( state == start ) {
	s.out() << html::div().classref("MenuWidget");
	s.out() << html::p();
	state = toplevelFiles;
    }
    s.out() << html::a().href(s.asUrl(file).string()) 
			<< file.leaf().string()
			<< html::a::end << "<br />" << std::endl;
}


void projfiles::flush( session& s ) const 
{
    switch( state ) {
    case toplevelFiles:
    case direntryFiles:
	s.out() << html::p::end;
	s.out() << html::div::end;
	break;	
    default:
	/* Nothing to do excepts shutup gcc warnings. */
	break;
    }	
}


void projfiles::fetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace std;
    using namespace boost::system;
    using namespace boost::filesystem;

    state = projfiles::start;
    projdir = s.root(pathname,"dws.xml");
    
    if( !projdir.empty() ) {
	/* We insert pathnames into a set<> such that they later can be 
	   iterated in alphabetically sorted order. */
	std::set<path> topdirs;
	std::set<path> topfiles;
	for( directory_iterator entry = directory_iterator(projdir); 
		 entry != directory_iterator(); ++entry ) {
	    if( is_directory(*entry) ) {
		topdirs.insert(*entry);
	    } else {
		if( selects(*entry) ) topfiles.insert(*entry);
	    }
	}
	
	for( std::set<path>::const_iterator entry = topfiles.begin(); 
		 entry != topfiles.end(); ++entry ) {
	    addFile(s,*entry);
	}
	
	for( std::set<path>::const_iterator entry = topdirs.begin(); 
		 entry != topdirs.end(); ++entry ) {
	    std::set<path> files;
	    /* Insert all filename which match a filter */
	    for( directory_iterator file = directory_iterator(*entry); 
		 file != directory_iterator(); ++file ) {
		if( !is_directory(*file) ) {
		    if( selects(*file) ) files.insert(*file);
		}
	    }
	    if( !files.empty() ) {
		addDir(s,*entry);
		for( std::set<path>::const_iterator file = files.begin(); 
		     file != files.end(); ++file ) {
		    addFile(s,*file);
		}
	    }
	}
	flush(s);
    }
}

void projfilesFetch( session& s, const boost::filesystem::path& pathname )
{
    projfiles p;
    p.fetch(s,pathname);
}
