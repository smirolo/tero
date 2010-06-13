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

#include <cstdlib>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include "todo.hh"
#include "markup.hh"

namespace {

    std::string todoAbsPath( boost::uuids::uuid tag ) {
	std::stringstream s;
        s << "/contrib/todos/" << tag << ".todo";
	return s.str();
    }

}  // anonymous namespace


void todoliner::filters( const post& p ) {
    std::cout << html::tr() 
	      << "<!-- " << p.score << " -->" << std::endl
	      << html::td() << p.time.date() << html::td::end
	      << html::td() << p.author << html::td::end
	      << html::td() << html::a().href(todoAbsPath(p.tag)) 
	      << p.title 
	      << html::a::end << html::td::end;
    
#if 0
    std::stringstream s;
    s << "/todoVote?href=" << todoAbsPath(p.tag);
    std::cout << html::td() 
	      << html::a().href(s.str())
	      << "<img src=\"/resources/donate.png\">"
	      << html::a::end
	      <<  html::td::end;
#endif
    std::cout << html::tr::end;
}


void todoIdx::byScore::filters( const post& p )
{
    indexes.push_back(p);
}


void todoIdx::byScore::flush()
{
    if( indexes.empty() ) {
	std::cout << html::tr()
		  << html::td().colspan("4")
		  << "no todo items present"
		  << html::td::end
		  << html::tr::end; 
    } else {
	std::sort(indexes.begin(),indexes.end(),orderByScore());
	for( indexSet::iterator idx = indexes.begin(); 
	     idx != indexes.end(); ++idx ) {
	    next->filters(*idx);
	}
	next->flush();
    }
}


void todoCreate::fetch( session& s, const boost::filesystem::path& pathname )
{
    boost::uuids::uuid tag = boost::uuids::random_generator()();   
    
    post p;
    p.title = s.valueOf("title");
    p.author = s.valueOf("author");
    p.time = boost::posix_time::second_clock::local_time();
    p.descr = s.valueOf("descr");

    std::string postname = todoAbsPath(tag);

    std::cout << htmlContent;

#ifndef READONLY
    boost::filesystem::ofstream file;
    create(file,s.srcDir(postname));

    blogwriter writer(file);
    writer.filters(p);	
    file.flush();    
    file.close();
   
    std::cout << html::p() << "item " << tag << " has been created." 
	      << html::p::end;
#else
    std::cout << html::p() << "item " << tag << " could not be created"
	" because you do not have permissions to do so on this server."
	" Sorry for the inconvienience."
	      << html::p::end;
#endif

}


void todoComment::fetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace boost::system;
    using namespace boost::filesystem;

    post p;
    p.author = s.valueOf("author");
    p.time = boost::posix_time::second_clock::local_time();
    p.descr = s.valueOf("descr");

    boost::filesystem::path postname(boost::filesystem::exists(pathname) ? 
				     pathname 
				     : s.abspath(s.valueOf("href")));


#ifndef READONLY
    boost::interprocess::file_lock f_lock(postname.string().c_str());
    boost::filesystem::ofstream file;

    f_lock.lock();
    file.open(postname,std::ios_base::out | std::ios_base::app);
    if( file.fail() ) {
	f_lock.unlock();
	boost::throw_exception(basic_filesystem_error<path>(
	   std::string("unable to open file"),
	   postname, 
	   error_code()));
    }

    blogwriter writer(file);
    writer.filters(p);	
    file.flush();
    
    file.close();
    f_lock.unlock();

    std::cout << redirect(s.asUrl(postname).string()) 
	      << htmlContent << std::endl;

#else
    std::cout << htmlContent << std::endl;
    std::cout << html::p() << "comment could not be created"
	" because you do not have permissions to do so on this server."
	" Sorry for the inconvienience."
	      << html::p::end;
#endif

}


void todoVote::fetch( session& s, const boost::filesystem::path& pathname )
{
    /* The pathname is set to the *todoVote* action name when we get here
       so we derive the document name from *href*. */
    
    boost::filesystem::path postname(boost::filesystem::exists(pathname) ? 
				     pathname 
				     : s.abspath(s.valueOf("href")));

    if( !boost::filesystem::is_regular_file(postname) ) {
	/* If *postname* does not point to a regular file,
	   the inputs were incorrect somehome. 
	 */
	std::cout << htmlContent;
	std::cout << html::p() << postname 
		  << " does not appear to be a regular file on the server"
		  << " and thus your vote for it cannot be registered."
		  << " Sorry for the inconvienience."
		  << html::p::end;
	return;
    } 


    boost::uuids::uuid tag = asuuid(boost::filesystem::basename(postname));

    /* make temporary file */
    char tmpname[FILENAME_MAX] = "vote-XXXXXX";
    int fildes = mkstemp(tmpname);

    std::cerr << "!!! tmpname = " << tmpname << std::endl;

    /* read/copy with score update */
    uint32_t score = 0;
    boost::interprocess::file_lock f_lock(postname.string().c_str());
    f_lock.lock();
    boost::filesystem::ifstream infile(postname);
    while( !infile.eof() ) {
	std::string line;
	std::getline(infile,line);
	line += '\n';
	if( line.compare(0,7,"Score: ") == 0 ) {
	    std::stringstream s;
	    score = atoi(line.substr(7).c_str()) + 1;
	    s << "Score: " << score << std::endl;	    
	    write(fildes,s.str().c_str(),s.str().size());
	} else {
	    write(fildes,line.c_str(),line.size());
	}
    }
    infile.close();
    close(fildes);

    /* move temporary back to original */
    if( score > 0 ) {
	boost::filesystem::remove(postname);
	boost::filesystem::rename(tmpname,postname);

	std::cout << htmlContent;
	std::cout << html::p() << "Your vote for item " 
		  << tag << " has been registered. Thank you." 
		  << html::p::end;
    } else {
	std::cout << htmlContent;
	std::cout << html::p() << "There does not appear to have any"
	    " score associated with item " << tag << " thus your"
	    " vote cannot be registered. Sorry for the inconvienience."
		  << html::p::end;
    }
    f_lock.unlock();
}

