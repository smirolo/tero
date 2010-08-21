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

#include <cstdlib>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include "todo.hh"
#include "payment.hh"
#include "markup.hh"

/** Pages related to todo items.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


article 
todoAdapter::fetch( session& s, const boost::filesystem::path& p ) {
    return article(boost::filesystem::basename(p.filename()),
		   "vote for todo item",1);
}


const boost::regex todoFilter::viewPat(".*todos/.+");


std::string todoFilter::asPath( const std::string& tag ) {
    std::stringstream s;
    s << (modifs / tag) << ".todo";
    return s.str();
}


class todoliner : public postFilter {
protected:
    std::ostream *ostr;

public:
    explicit todoliner( std::ostream& o ) : ostr(&o) {}

    virtual void filters( const post& );
};

class byScore : public postFilter {
protected:
    typedef std::vector<post> indexSet;
    
    indexSet indexes;
    std::ostream *ostr;
    
public:
    byScore( std::ostream& o, postFilter &n ) : postFilter(&n), ostr(&o) {}
    
    virtual void filters( const post& );
    virtual void flush();
};


class todoCreateFeedback : public postFilter {
protected:
    std::ostream *ostr;

public:
    explicit todoCreateFeedback( std::ostream& o ) : ostr(&o) {}

    virtual void filters( const post& );
};


class todoCommentFeedback : public postFilter {
protected:
    std::ostream *ostr;
    std::string posturl;

public:
    todoCommentFeedback( std::ostream& o, const std::string& p ) 
	: ostr(&o), posturl(p) {}

    virtual void filters( const post& );
};


class todocreator : public todoFilter {
public:
    explicit todocreator( const boost::filesystem::path& m )
	: todoFilter(m) {}

    todocreator( const boost::filesystem::path& m, postFilter* n  ) 
	: todoFilter(m,n) {}

    virtual void filters( const post& );
};


class todocommentor : public postFilter {
protected:
    boost::filesystem::path postname;

public:
    todocommentor( const boost::filesystem::path& p, postFilter* n  ) 
	: postFilter(n), postname(p) {}

    virtual void filters( const post& );
};


void todoCreateFeedback::filters( const post& p ) {
    *ostr << html::p() << "item " << p.guid << " has been created." 
	      << html::p::end;    
}


void todocreator::filters( const post& v ) {
    post p = v;
    p.score = 0;
    
    std::stringstream s;
    s << boost::uuids::random_generator()();
    p.guid = s.str();
    p.time = boost::posix_time::second_clock::local_time();

#ifndef READONLY
    boost::filesystem::ofstream file;
    createfile(file,asPath(p.guid));
    blogwriter writer(file);
    writer.filters(p);	
    file.flush();    
    file.close();    
#else
    throw std::runtime_error("Todo item could not be created"
	" because you do not have permissions to do so on this server."
	" Sorry for the inconvienience.");
#endif

    if( next ) next->filters(p);
}


void todocommentor::filters( const post& p ) {
    using namespace boost::system;
    using namespace boost::filesystem;

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
#else
    throw std::runtime_error("comment could not be created"
	" because you do not have permissions to do so on this server."
			     " Sorry for the inconvienience.");
#endif
    if( next ) next->filters(p);
}


void todoCommentFeedback::filters( const post& p ) {
    /* \todo clean-up. We use this code such that the browser displays
       the correct url. If we use a redirect, it only works with static pages (index.html). */
    *ostr << httpHeaders << std::endl << "<html><head><META HTTP-EQUIV=\"Refresh\" CONTENT=\"0;URL=" << posturl << "\"></head><body></body></html>" << std::endl;
}


void todoliner::filters( const post& p ) {
    *ostr << html::tr() 
	      << "<!-- " << p.score << " -->" << std::endl
	      << html::td() << p.time.date() << html::td::end
	      << html::td() << p.authorEmail << html::td::end
	  << html::td() << html::a().href(p.guid + ".todo") 
	      << p.title 
	      << html::a::end << html::td::end;
    *ostr << html::td()
	      << p.score
	      << html::td::end;
    *ostr << html::tr::end;
}


void byScore::filters( const post& p )
{
    indexes.push_back(p);
}


void byScore::flush()
{
    if( indexes.empty() ) {
	*ostr << html::tr()
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


boost::filesystem::path todoModifPost::asPath( const std::string& tag ) const
{
    std::stringstream s;
    s << (modifs / tag) << ".todo";
    return boost::filesystem::path(s.str());
}


void todoCreate::fetch( session& s, const boost::filesystem::path& pathname )
{    
    boost::filesystem::path 
	dirname(boost::filesystem::exists(pathname) ? 
		pathname : s.abspath(modifs));
    

    todoCreateFeedback fb(*ostr);
    todocreator create(dirname,&fb);
#if 0
    if( !istr->eof() ) {
	mailParser parser(*ostr,create);
	parser.walk(s,*istr);
    } else 
#endif
	{
	post p;
	p.score = 0;
	p.title = s.valueOf("title");
	p.authorEmail = s.valueOf("author");
	p.descr = s.valueOf("descr");	
	create.filters(p);
	}

}


void todoComment::fetch( session& s, const boost::filesystem::path& pathname )
{
    boost::filesystem::path 
	postname(boost::filesystem::exists(pathname) ? 
		 pathname : s.abspath(s.valueOf("href")));

    todoCommentFeedback fb(*ostr,s.asUrl(postname).string());
    todocommentor comment(postname,&fb);

#if 0
    if( !istr->eof() ) {
	mailParser parser(*ostr,comment);
	parser.walk(s,*istr); 	
    } else 
#endif
	{
	post p;
	p.authorEmail = s.valueOf("author");
	p.descr = s.valueOf("descr");
	p.time = boost::posix_time::second_clock::local_time();
	p.guid = todoAdapter().fetch(s,postname).guid;
	p.score = 0;
	comment.filters(p);
   }
}

void todoIndexWriteHtml::fetch( session& s, 
				const boost::filesystem::path& pathname ) 
{
    todoliner shortline(*ostr);
    byScore order(*ostr,shortline);
    mailParser parser(*ostr,order,true);
    parser.fetch(s,pathname);
}


void todoVoteAbandon::fetch( session& s, 
			     const boost::filesystem::path& pathname ) {
	*ostr << html::p() << "You have abandon the transaction and thus"
		  << " your vote has not been registered."
		  << " Thank you for your interest." 
		  << html::p::end;
}


void todoVoteSuccess::fetch( session& s, 
			     const boost::filesystem::path& pathname )
{
    payment::checkReturn(s,returnPath);

    /* The pathname is set to the *todoVote* action name when we get here
       so we derive the document name from *href*. */
    boost::filesystem::path 
	postname(boost::filesystem::exists(pathname) ? 
		 pathname : asPath(s.valueOf("referenceId")));

    if( !boost::filesystem::is_regular_file(postname) ) {
	/* If *postname* does not point to a regular file,
	   the inputs were incorrect somehome. 
	 */
	*ostr << html::p() << postname 
		  << " does not appear to be a regular file on the server"
		  << " and thus your vote for it cannot be registered."
	    " Please " << html::a().href("info@fortylines.com") << "contact us"
		  << html::a::end << " about the issue."
		  << " Sorry for the inconvienience."
		  << html::p::end;
	return;
    } 


    std::string tag = todoAdapter().fetch(s,postname).guid;

    /* make temporary file */
    char tmpname[FILENAME_MAX] = "/tmp/vote-XXXXXX";
    int fildes = mkstemp(tmpname);
    if( fildes == -1 ) {
	*ostr << html::p() 
		  << " Unable to create temporary file on the server"
		  << " and thus your vote for it cannot be registered."
	    " Please " << html::a().href("info@fortylines.com") << "contact us"
		  << html::a::end << " about the issue."
		  << " Sorry for the inconvienience."
		  << html::p::end;
	return;
    }

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
	*ostr << html::p() << "Your vote for item " 
		  << tag << " has been registered. Thank you." 
		  << html::p::end;
    } else {
	*ostr << html::p() << "There does not appear to have any"
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
    title << '[' << todoAdapter().fetch(s,pathname).guid << ']';
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
    htmlwriter writer(*ostr); 
    mailParser parser(*ostr,writer);
    parser.fetch(s,pathname);
}
