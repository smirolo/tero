/* Copyright (c) 2009-2012, Fortylines LLC
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
#include "markup.hh"
#include "feeds.hh"
#include "revsys.hh"

/** Pages related to todo items.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

static const char *todoExt = ".todo";

article
todoAdapter::fetch( session& s, const boost::filesystem::path& p ) {
    return article(boost::filesystem::basename(p.filename()),
        "vote for todo item",1);
}


const boost::regex todoFilter::viewPat(".*todos/.+");


std::string todoFilter::asPath( const std::string& tag ) {
    std::stringstream s;
    s << (modifs / tag) << todoExt;
    return s.str();
}


class byScore : public retainedFilter {
protected:
    typedef std::vector<post> indexSet;

    indexSet indexes;
    std::ostream *ostr;

public:
    byScore( std::ostream& o, postFilter &n ) : retainedFilter(&n), ostr(&o) {}

    virtual void filters( const post& );
    virtual void flush();
};


class todoCreateFeedback : public passThruFilter {
protected:
    std::ostream *ostr;

public:
    explicit todoCreateFeedback( std::ostream& o ) : ostr(&o) {}

    virtual void filters( const post& );
};


class todoCommentFeedback : public passThruFilter {
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


class todocommentor : public passThruFilter {
protected:
    boost::filesystem::path postname;

public:
    todocommentor( const boost::filesystem::path& p, postFilter* n  )
        : passThruFilter(n), postname(p) {}

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
#if 0
    /*\todo*/
    createfile(file,asPath(p.guid));
#endif
    mailwriter writer(file);
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
        using namespace boost::system::errc;
        f_lock.unlock();
        boost::throw_exception(boost::system::system_error(
                make_error_code(no_such_file_or_directory),
                postname.string()));
    }

    mailwriter writer(file);
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
    httpHeaders.refresh(0,url(posturl));
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
        std::sort(indexes.begin(),indexes.end(),orderByDateScore());
        for( indexSet::iterator idx = indexes.begin();
             idx != indexes.end(); ++idx ) {
            next->filters(*idx);
        }
        next->flush();
    }
}


void todoCreateFetch( session& s, const url& name )
{
    post p;
    std::stringstream str;
	
	boost::filesystem::path pathname = s.abspath(name);
    str << boost::uuids::random_generator()();
    p.guid = str.str();
    p.time = boost::posix_time::second_clock::local_time();
    p.score = 0;
    p.title = titleVar.value(s);

    p.authorEmail = authorVar.value(s);
    if( p.authorEmail.empty() ) {
        revisionsys* rev = revisionsys::findRev(s,pathname);
        if( rev ) {
            p.authorEmail = rev->configval("email");
        }
    }
    p.content = descrVar.value(s);

    boost::filesystem::ofstream file;
    boost::filesystem::path
        todoname(s.runAsCGI() ? (boost::filesystem::is_directory(pathname) ?
                pathname : pathname.parent_path())
            : boost::filesystem::current_path());
    todoname /= p.guid;
    todoname.replace_extension(todoExt);

    s.createfile(file,todoname);
    mailwriter writer(file);
    writer.filters(p);
    file.flush();
    file.close();

    if( s.runAsCGI() ) {
        httpHeaders.refresh(0,s.asUrl(todoname / "edit"));
    } else {
        s.out() << "Todo item " << p.guid << " has been created."
                << std::endl
                << "Edit " << todoname << " with your favorite editor "
                << "and commit it to the repository" << std::endl;
    }
}


void todoCommentFetch( session& s, const url& name )
{
	boost::filesystem::path pathname = s.abspath(name);
    boost::filesystem::path postname(pathname.parent_path());
    todoCommentFeedback fb(s.out(),s.asUrl(postname).string());
    todocommentor comment(pathname,&fb);

#if 0
    if( !istr->eof() ) {
        mailParser parser(comment);
        parser.walk(s,*istr);
    } else
#endif
    {
        post p;
        p.author = authorVar.value(s);
        p.authorEmail = authorEmail.value(s);
        p.content = descrVar.value(s);
        p.time = boost::posix_time::second_clock::local_time();
        p.guid = todoAdapter().fetch(s,postname).guid;
        p.score = 0;
        comment.filters(p);
   }
}

void todoIndexWriteHtmlFetch( session& s, const url& name )
{
	boost::filesystem::path pathname = s.abspath(name);
    byTimeHtml shortline(s.out());
    byScore order(s.out(),shortline);
    mailParser parser(order,true);
    parser.fetch(s,pathname);
    order.flush();
}


void todoVoteAbandonFetch( session& s,
    const url& name ) {
    s.out() << html::p() << "You have abandon the transaction and thus"
            << " your vote has not been registered."
            << " Thank you for your interest."
            << html::p::end;
}


void todoVoteSuccessFetch( session& s, const url& name )
{
	boost::filesystem::path pathname = s.abspath(name);

#if 0
    payment::checkReturn(s,returnPath);
#endif

#if 0
    /* \todo keep old code around until we can verify pathname as return
       url works well. */
    std::stringstream str;
    boost::filesystem::path modifs(s.valueOf("todoDir"));
    str << (modifs / s.valueOf("referenceId")) << todoExt;
    boost::filesystem::path asPath(str.str());
#else
    boost::filesystem::path
        asPath(boost::filesystem::exists(pathname) ?
            pathname : pathname.parent_path());
#endif

    /* The pathname is set to the *todoVote* action name when we get here
       so we derive the document name from *href*. */
    boost::filesystem::path
        postname(boost::filesystem::exists(pathname) ?
            pathname : asPath);

    if( !boost::filesystem::is_regular_file(postname) ) {
        /* If *postname* does not point to a regular file,
           the inputs were incorrect somehome.
        */
        s.out() << html::p() << postname
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
        s.out() << html::p()
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
    ssize_t written;
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
            written = write(fildes,s.str().c_str(),s.str().size());
        } else {
            written = write(fildes,line.c_str(),line.size());
        }
    }
    infile.close();
    close(fildes);

    /* move temporary back to original */
    if( score > 0 ) {
        using namespace boost::filesystem;
        /* boost::filesystem::rename does not work accross filesystem. */
        copy_file(path(tmpname),postname,copy_option::overwrite_if_exists);
        remove(tmpname);
        s.out() << html::p() << "Your vote for item "
                << tag << " has been registered. Thank you."
                << html::p::end;
    } else {
        s.out() << html::p() << "There does not appear to have any"
            " score associated with item " << tag << " thus your"
            " vote cannot be registered. Sorry for the inconvienience."
                << html::p::end;
    }
    f_lock.unlock();
}

namespace {
char titleMeta[] = "title";
} // anonymous


void todoMeta( session& s, std::istream& in, const url& name )
{
    using namespace boost::filesystem;
    static const boost::regex valueEx("^(\\S+):\\s+(.*)");

    std::stringstream titles;
    titles << "(no title)";

    /* \todo should only load one but how does it sits with dispatchDoc
     that initializes s[varname] by default to "document"? */
    std::string line;
    std::getline(in,line);  // skip first line "From  ..." (see mbox)
    while( !in.eof() ) {
        boost::smatch m;
        std::getline(in,line);
        if( boost::regex_search(line,m,valueEx) ) {
            if( m.str(1) == std::string("Subject") ) {
                titles << " - " << m.str(2);
                s.insert("title",titles.str());
            } else {
                s.insert(m.str(1),m.str(2));
            }
        } else break;
    }

    /*
       std::time_t last_write_time( const path & ph );
       To convert the returned value to UTC or local time,
       use std::gmtime() or std::localtime() respectively. */
    metaFetch<titleMeta>(s,url());
}


void
todoWriteHtmlFetch( session& s, std::istream& in, const url& name )
{
	boost::filesystem::path pathname = s.abspath(name);

    htmlwriter writer(s.out());
    mailParser parser(writer);
    parser.fetch(s,s.abspath(name));
}
