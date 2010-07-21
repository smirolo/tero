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
#include "aws.hh"

namespace {

    std::string todoAbsPath( const std::string& tag ) {
	std::stringstream s;
        s << "/contrib/todos/" << tag << ".todo";
	return s.str();
    }

    std::string todoAbsPath( boost::uuids::uuid tag ) {
	std::stringstream s;
        s << "/contrib/todos/" << tag << ".todo";
	return s.str();
    }
    
}  // anonymous namespace

boost::uuids::uuid todouuid( const boost::filesystem::path& p ) {
    return asuuid(boost::filesystem::basename(p.filename()));
}

class todoliner : public postFilter {
public:

    virtual void filters( const post& );
};

class byScore : public postFilter {
protected:
    typedef std::vector<post> indexSet;
    
    indexSet indexes;
    
public:
    explicit byScore( postFilter &n ) : postFilter(&n) {}
    
    virtual void filters( const post& );
    virtual void flush();
};


class todoCreateFeedback : public postFilter {
public:
    virtual void filters( const post& );
};


class todocreator : public postFilter {
protected:
    const session *s;

public:
    todocreator( const session& v, postFilter* n  ) 
	: postFilter(n), s(&v) {}

    virtual void filters( const post& );
};


class todocommentor : public postFilter {
protected:
    const session *s;

public:
    todocommentor( const session& v, postFilter* n  ) 
	: postFilter(n), s(&v) {}

    virtual void filters( const post& );
};


void todoCreateFeedback::filters( const post& p ) {
    std::cout << htmlContent;
    std::cout << html::p() << "item " << p.tag << " has been created." 
	      << html::p::end;    
}


void todocreator::filters( const post& v ) {
    post p = v;
    p.tag = boost::uuids::random_generator()();   
    p.time = boost::posix_time::second_clock::local_time();

#ifndef READONLY
    boost::filesystem::ofstream file;
    createfile(file,s->srcDir(todoAbsPath(p.tag)));

    blogwriter writer(file);
    writer.filters(p);	
    file.flush();    
    file.close();
    next->filters(p);
#else
    throw std::runtime_error("Todo item could not be created"
	" because you do not have permissions to do so on this server."
	" Sorry for the inconvienience.");
#endif

}


void todocommentor::filters( const post& v ) {

}


void todoliner::filters( const post& p ) {
    std::cout << html::tr() 
	      << "<!-- " << p.score << " -->" << std::endl
	      << html::td() << p.time.date() << html::td::end
	      << html::td() << p.authorEmail << html::td::end
	      << html::td() << html::a().href(todoAbsPath(p.tag)) 
	      << p.title 
	      << html::a::end << html::td::end;
    std::cout << html::td()
	      << p.score
	      << html::td::end;
    std::cout << html::tr::end;
}

void byScore::filters( const post& p )
{
    indexes.push_back(p);
}


void byScore::flush()
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


void todoAppendPost::fetch( session& s, 
			    const boost::filesystem::path& pathname )
{
}


void todoCreate::fetch( session& s, const boost::filesystem::path& pathname )
{    
    post p;
    p.title = s.valueOf("title");
    p.authorEmail = s.valueOf("author");
    p.descr = s.valueOf("descr");

    todoCreateFeedback fb;
    todocreator create(s,&fb);
    if( p.valid() ) {
	create.filters(p);

    } else {
	mailParser parser(create);
	parser.walk(s,*istr);
    }
}


void todoComment::fetch( session& s, const boost::filesystem::path& pathname )
{
    using namespace boost::system;
    using namespace boost::filesystem;

    boost::filesystem::path postname(boost::filesystem::exists(pathname) ? 
				     pathname 
				     : s.abspath(s.valueOf("href")));

    post p;
    p.authorEmail = s.valueOf("author");
    p.descr = s.valueOf("descr");
    p.time = boost::posix_time::second_clock::local_time();
    p.tag = todouuid(postname);

#if 0
    if( p.valid() ) {
	comment.filters(p);
    } else {
	mailParser parser(comment);
	parser.walk(s,*istr);
    }
#endif

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

void todoIndexWriteHtml::fetch( session& s, 
				const boost::filesystem::path& pathname ) 
{
    todoliner shortline;
    byScore order(shortline);
    mailParser parser(order);
    parser.fetch(s,pathname);
}


void todoVoteAbandon::fetch( session& s, 
			     const boost::filesystem::path& pathname ) {
	std::cout << htmlContent;
	std::cout << html::p() << "You have abandon the transaction and thus"
		  << " your vote has not been registered."
		  << " Thank you for your interest." 
		  << html::p::end;
}


void todoVoteSuccess::fetch( session& s, 
			     const boost::filesystem::path& pathname )
{
    awsStandardButton button(s.valueOf("awsAccessKey"),
			     s.valueOf("awsSecretKey"),
			     s.valueOf("awsCertificate"));
    if( !button.checkReturn(s,"/todoVoteSuccess") ) {
	std::cout << "error wrong request signature" << std::endl;
	return;
    }

    /* The pathname is set to the *todoVote* action name when we get here
       so we derive the document name from *href*. */
    boost::filesystem::path postname(boost::filesystem::exists(pathname) ? 
				     pathname 
				     : todoAbsPath(s.valueOf("referenceId")));

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


void todoWriteHtml::meta( session& s, 
			  const boost::filesystem::path& pathname ) {
    using namespace boost::filesystem; 
    static const boost::regex valueEx("^(Subject):\\s+(.*)");

    std::stringstream title;
    title << '[' << todouuid(pathname) << ']';
    ifstream strm;
    open(strm,pathname);
    while( !strm.eof() ) {
	boost::smatch m;
	std::string line;
	std::getline(strm,line);
	if( boost::regex_search(line,m,valueEx) ) {
	    title << ' ' << m.str(2);
	    break;
	}
    }
    strm.close();
    s.vars["Subject"] = title.str();
    document::meta(s,pathname);
}

void 
todoWriteHtml::fetch( session& s, const boost::filesystem::path& pathname ) 
{
    std::cerr << "write todo html " << pathname << " ..." << std::endl;
    htmlwriter writer(std::cout); 
    mailParser parser(writer);
    parser.fetch(s,pathname);
}
