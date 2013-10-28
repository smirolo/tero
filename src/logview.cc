/* Copyright (c) 2009-2013, Fortylines LLC
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

pathVariable indexFile("indexFile",
    "index file with projects dependencies information");

void logAddSessionVars( boost::program_options::options_description& all,
    boost::program_options::options_description& visible )
{
    using namespace boost::program_options;

    options_description localOptions("log");
    localOptions.add(indexFile.option());
    all.add(localOptions);
    visible.add(localOptions);
}


int attrAsInt( RAPIDXML::xml_node<> *node, const char *name ) {
    RAPIDXML::xml_attribute<> *attr = node->first_attribute(name);
    return attr ? atoi(attr->value()) : 0;
}


boost::posix_time::ptime
attrAsDate( RAPIDXML::xml_node<> *node, const char *name ) {
    RAPIDXML::xml_attribute<> *attr = node->first_attribute(name);
    return attr ? from_mbox_string(attr->value()) : boost::posix_time::ptime();
}

void junitDate( session& s, const url& name )
{
    RAPIDXML::xml_document<> *doc = s.loadxml(s.abspath(name));
    RAPIDXML::xml_node<> *testsuite = doc->first_node();
    if( testsuite != NULL ) {
        s.out().imbue(std::locale(s.out().getloc(), pubDate::shortFormat()));
        s.out() <<  attrAsDate(testsuite, "timestamp");
    }
#if 0
    /* XXX generates a pointer being freed was not allocated */
    if( doc ) delete doc;
#endif
}


void junitContent( session& s, const slice<char>& text, const url& name )
{
    using namespace RAPIDXML;

    RAPIDXML::xml_document<> *doc = s.loadxml(s.abspath(name));

    xml_node<> *testsuite = doc->first_node();
    if( testsuite != NULL ) {
        int nbFailures = attrAsInt(testsuite, "failures");
        int nbErrors = attrAsInt(testsuite, "errors");
        int nbTests = attrAsInt(testsuite, "tests");

        if( (nbErrors + nbFailures) > 0  ) {
            const char *sep = "";
            if( nbFailures > 0 ) {
                s.out() << nbFailures << " failures" << std::endl;
                sep = ",";
            }
            if( nbErrors > 0 ) {
                s.out() << nbErrors << " errors" << std::endl;
            }
            s.out() << " out of " << nbTests << " tests." << std::endl;
        } else {
            s.out() << nbTests << " tests passed." << std::endl;
        }

        for( xml_node<> *testcase = testsuite->first_node("testcase");
             testcase != NULL; testcase = testcase->next_sibling("testcase") ) {
            xml_attribute<> *name = testcase->first_attribute("name");
            if( name ) {
                // XXX implement: output testcase if failed.
                // std::cerr << "testcase: " << name->value() << std::endl;
            }
        }
    }

    if( doc ) delete doc;
}


void logviewFetch( session& s, const url& name )
{

    using namespace RAPIDXML;
    using namespace boost::filesystem;

	path pathname = s.abspath(name);

    /* remove the '/log' command tag and add the project dependency info
       (dws.xml). */
    boost::filesystem::path indexFileName = indexFile.value(s);

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

    /* Populate the project names based on the projects specified
       in the index file. */
    xml_document<> *doc = s.loadxml(indexFileName);
    if ( doc ) {
        /* Error loading the document, don't bother */
        xml_node<> *root = doc->first_node();
        if( root != NULL ) {
            for( xml_node<> *project = root->first_node("project");
                 project != NULL; project = project->next_sibling("project") ) {
                xml_attribute<> *name = project->first_attribute("name");
                if( name != NULL ) {
                     path repcontrol((srcTop.value(s)
                             / path(name->value())));
                    if( boost::filesystem::exists(repcontrol) ) {
                        table[name->value()] = colType();
                    }
                }
            }
        }
        path dirname(is_directory(pathname) ?
            pathname : siteTop.value(s) / "log");

        std::string logBase;
        for( directory_iterator entry = directory_iterator(dirname);
             entry != directory_iterator(); ++entry ) {
            boost::smatch m;
            path fullpath = entry->path();
            path filename = fullpath.filename();
            if( boost::regex_search(filename.string(),m,logPat) ) {
                std::streambuf *buf = revisionsys::findRevOpenfile(s, *entry);
                std::istream input(buf);
                colHeaders.insert(filename);
                std::string name;
                while( !input.eof() ) {
                    std::string line;
                    getline(input, line);
                    if( boost::regex_match(line, m,
                            boost::regex("######## (\\S+)")) ) {
                        name = m.str(1);
                    } else if( boost::regex_match(line, m,
                            boost::regex("\\S+: completed in (.*)")) ) {
                        std::string status = m.str(1);
                        if( table.find(name) != table.end() ) {
                            table[name][filename]
                                = std::make_pair(status, 0);
                        }
                    } else if( boost::regex_match(line, m,
                            boost::regex("\\S+: error \\((\\d+)\\) after (.*)")) ) {
                        std::string status = m.str(2);
                        int exitCode = atoi(m.str(1).c_str());
                        if( table.find(name) != table.end() ) {
                            table[name][filename]
                                = std::make_pair(status, exitCode);
                        }
                    }
                }
                delete buf;
            }
        }
    }
    s.out() << html::p() << "This build view page shows the stability "
        "of all projects at a glance. Each column represents a build log "
        "obtained by running the following command on a local machine:"
            << html::p::end;
    s.out() << html::pre() << html::a().href("/resources/dws")
            << "dws" << html::a::end << " build "
            << s.asAbsUrl(s.asUrl(indexFileName),"")
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
		    << html::a().href((boost::filesystem::path("/log") / *col).string())
		    << *col << html::a::end << html::th::end;
	}
	s.out() << html::tr::end;

	/* Display one project per row, one build result per column. */
	for( tableType::const_iterator row = table.begin();
	     row != table.end(); ++row ) {
	    s.out() << html::tr();
	    s.out() << html::th() << projhref(s,row->first) 
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


void regressionsFetch( session& s, const url& name )
{
    using namespace RAPIDXML;
    using namespace boost::filesystem;

    boost::filesystem::path regressname
	= siteTop.value(s) 
	/ boost::filesystem::path("log/tests")
	/ (document.value(s).pathname.parent_path().filename().string()
	   + std::string("-test/regression.log"));

    if( !boost::filesystem::exists(regressname) ) {
		s.out() << html::p()
				<< "There are no regression logs available for the unit tests."
				<< html::p::end;
		return;
    }

    xml_document<> *doc = s.loadxml(regressname);
    xml_node<> *root = doc->first_node();
    if( root != NULL ) {

	std::set<std::string> headers;
	for( xml_node<> *testcase = root->first_node("testcase");
		 testcase != NULL; testcase = testcase->next_sibling("testcase") ) {
	    xml_attribute<> *name = testcase->first_attribute("name");
		assert( name != NULL );
		headers.insert(name->value());
	}

	typedef std::map<std::string,size_t> colMap;
	colMap colmap;	
	size_t col = 0;

	s.out() << html::p();
	s.out() << html::table() << std::endl;
	s.out() << html::tr();	
	s.out() << html::th() << html::th::end;
	for( std::set<std::string>::const_iterator header = headers.begin();
		 header != headers.end(); ++header ) {
		s.out() << html::th() << *header << html::th::end;
		colmap.insert(std::make_pair(*header,col++));		
	}
	s.out() << html::tr::end;

	std::string current;
	xml_node<>* cols[colmap.size()];
	for( xml_node<> *test = root->first_node("testcase");
	     test != NULL; test = test->next_sibling("testcase") ) {
	    xml_attribute<> *name = test->first_attribute("classname");
		if( name->value() != current ) {
			if( !current.empty() ) {
				s.out() << html::tr() << html::td() << current << html::td::end;
				for( xml_node<> **c = cols; 
					 c != &cols[colmap.size()]; ++c ) {
					s.out() << html::td();
					if( *c != NULL ) {
						xml_node<> *error = (*c)->first_node("error");
						if( error ) {
							xml_attribute<> *type 
								= error->first_attribute("type");
							s.out() << type->value();
						} else {
							s.out() << "pass";
						}
					}
					s.out() << html::td::end;
				}
				s.out() << html::tr::end;
			}
			current =  name->value();
			memset(cols,0,sizeof(cols));
		}
		name = test->first_attribute("name");
		if( name != NULL ) {
		    colMap::const_iterator found = colmap.find(name->value());
		    if( found != colmap.end() ) {
				cols[found->second] = test;
		    }
		}
	}
	if( !current.empty() ) {
		s.out() << html::tr() << html::td() << current << html::td::end;
		for( xml_node<> **c = cols; 
			 c != &cols[colmap.size()]; ++c ) {
			s.out() << html::td();
			if( *c != NULL ) {
				xml_node<> *error = (*c)->first_node("error");
				if( error ) {
					xml_attribute<> *type = error->first_attribute("type");
					s.out() << type->value();
				} else {
					s.out() << "pass";
				}
			}
			s.out() << html::td::end;
		}
		s.out() << html::tr::end;
	}

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

	s.out() << html::table::end << html::p::end;
    }   
}
