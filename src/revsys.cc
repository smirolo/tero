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

#include <cstdio>
#include <sys/stat.h>
#include "changelist.hh"
#include "markup.hh"
#include <boost/regex.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include "project.hh"
#include "revsys.hh"
#include "decorator.hh"
#include "popen_streambuf.h"

/** Execute git commands

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

extern pathVariable binDir;

/** interaction with a git repository.
 */
class gitcmd : public revisionsys {
protected:
    boost::filesystem::path executable;

    /** Execute a command line catching stdout such that apache does
        not end-up with malformed headers and throwing an exception
        when the command fails.
    */
    std::streambuf* shellcmd( const std::string& cmdline );

    static gitcmd _instance;

public:
    gitcmd() : revisionsys(".git") {}

    static gitcmd& instance( const boost::filesystem::path& binName ) {
        _instance.executable = binName;
        return _instance;
    }

    virtual void loadconfig( session& s );

    void checkins( const session& s,
        const boost::filesystem::path& pathname,
        postFilter& filter );

    void diff( std::ostream& ostr,
        const std::string& leftCommit,
        const std::string& rightCommit,
        const boost::filesystem::path& pathname );

    void history( std::ostream& ostr,
        const session& s,
        const boost::filesystem::path& pathname,
        historyref& r );

    void showDetails( std::ostream& ostr,
        const std::string& commit );

    void show( std::ostream& ostr,
        const boost::filesystem::path& pathname,
        const std::string& commit,
        decorator *dec = NULL );

    slice<char> loadtext( const boost::filesystem::path& pathname,
        const std::string& commit );

    std::streambuf* openfile( const boost::filesystem::path& pathname,
        const std::string& commit = "HEAD" );
};


class filesys : public revisionsys {
protected:
    static filesys _instance;

public:
    filesys() : revisionsys("fs") {}

    static filesys& instance() {
        return _instance;
    }

    boost::filesystem::path
    relative( const boost::filesystem::path& pathname ) const {
        return pathname;
    }

    virtual void loadconfig( session& s ) {}

    virtual void create( const boost::filesystem::path& pathname,
        bool group = false ) {}

    virtual void add( const boost::filesystem::path& pathname ) {}

    virtual void commit( const std::string& msg ) {}

    virtual void diff( std::ostream& ostr,
        const std::string& leftCommit,
        const std::string& rightCommit,
        const boost::filesystem::path& pathname ) {}

    virtual void history( std::ostream& ostr,
        const session& s,
        const boost::filesystem::path& pathname,
        historyref& r ) {}

    virtual void checkins( const session& s,
        const boost::filesystem::path& pathname,
        postFilter& filter ) {}

    virtual void showDetails( std::ostream& ostr,
        const std::string& commit ) {}

    /** Show a file in the repository at a specified *commit*,
        using a decorator to do syntax highlighting. */
    virtual void show( std::ostream& ostr,
        const boost::filesystem::path& pathname,
        const std::string& commit,
        decorator *dec = NULL ) {}

    slice<char> loadtext( const boost::filesystem::path& pathname,
        const std::string& commit );

    std::streambuf* openfile( const boost::filesystem::path& pathname,
        const std::string& commit = "HEAD" );

};


gitcmd gitcmd::_instance;
filesys filesys::_instance;


const std::string nullString;

revisionsys::revsSet revisionsys::revs;

const std::string& revisionsys::configval( const std::string& key ) {
    std::map<std::string,std::string>::const_iterator found = config.find(key);
    if( found != config.end() ) {
        return found->second;
    }
    return nullString;
}


boost::filesystem::path
revisionsys::absolute( const boost::filesystem::path& pathname ) const
{
    if( rootpath.leaf() == metadir ) {
        return rootpath.parent_path() / pathname;
    } else {
        size_t rootSize = rootpath.string().size();
        size_t metaSize = strlen(metadir);
        if( rootSize > metaSize
            && rootpath.string().compare(rootSize - metaSize,
                metaSize, metadir) == 0) {
            return rootpath.string().substr(0, rootSize - metaSize) / pathname;
        }
    }
    return rootpath / pathname;
}


boost::filesystem::path
revisionsys::relative( const boost::filesystem::path& pathname ) const
{
    boost::filesystem::path result;
    for( boost::filesystem::path::const_iterator
             iter_path = pathname.begin(), iter_root = rootpath.begin();
         iter_path != pathname.end() && iter_root != rootpath.end();
         ++iter_path, ++iter_root ) {
        if( *iter_path != *iter_root ) {
            boost::filesystem::path base = *iter_root;
            size_t rootSize = base.string().size();
            size_t metaSize = strlen(metadir);
            if( rootSize > metaSize
                //&& rootSize != metaSize
                && base.string().compare(rootSize - metaSize,
                    metaSize, metadir) == 0) {
                ++iter_path;
                if( iter_path == pathname.end() ) break;
            }
            for( ; iter_path != pathname.end(); ++iter_path ) {
                if( *iter_path != "." ) {
                    result /= *iter_path;
                }
            }
            break;
        }
    }
    return result;
}


revisionsys*
revisionsys::exists( session& s, const boost::filesystem::path& dirname ) {

    using namespace boost::filesystem;

    if( revs.empty() ) {
        /* first time */
        revs.push_back(&gitcmd::instance(binDir.value(s) / "git"));
    }

    path repoRoot;
    for( revsSet::iterator r = revs.begin(); r != revs.end(); ++r ) {

        path metadir(dirname);
        metadir.replace_extension((*r)->metadir);
        if( is_directory(metadir) ) {
            /* directory finishing in .git, we have a candidate. */
            repoRoot = metadir;
        }
        metadir = dirname / (*r)->metadir;
        if( is_directory(metadir) ) {
            /* directory contains a .git subdirectory, we have a candidate. */
            repoRoot = metadir;
        }

        if( !repoRoot.empty() ) {
            (*r)->rootpath = repoRoot;
            (*r)->loadconfig(s);
            return *r;
        }
    }
    return NULL;
}


revisionsys*
revisionsys::findRev( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::filesystem;
    using namespace boost::system::errc;

    /* Find the root directory of the source repository where *pathname*
       resides. We first look directly if *pathname* can be interpreted
       as a repository root directory before walking up to *siteTop*.
       The advantage is both that we can deal with repositories outside
       siteTop if those are addressed absolutely and we don't look outside
       of siteTop otherwise. */
    path tmppath = pathname;
    revisionsys* rev = exists(s, tmppath);
    if( !rev ) {
        tmppath = tmppath.parent_path();
        while( s.prefix(siteTop.value(s), tmppath) ) {
            rev = exists(s, tmppath);
            if( rev ) break;
            tmppath = tmppath.parent_path();
        }
    }
    if( !rev ) {
        filesys::instance().rootpath = siteTop.value(s);
        return &filesys::instance();
    }
    return rev;
}


revisionsys*
revisionsys::findRevByMetadir( session& s, const std::string& metadir ) {
    if( revs.empty() ) {
        /* first time */
        revs.push_back(&gitcmd::instance(binDir.value(s) / "git"));
    }

    for( revsSet::iterator r = revs.begin(); r != revs.end(); ++r ) {
        if( (*r)->metadir == metadir ) {
            return *r;
        }
    }
    return NULL;
}


std::streambuf* revisionsys::findRevOpenfile(
    session& s,
    const boost::filesystem::path& pathname,
    const std::string& commit )
{
#if 1
    if( boost::filesystem::exists(pathname) ) {
        return filesys::instance().openfile(pathname, commit);
    } else {
#endif
        revisionsys* rev = findRev(s, pathname);
        if( rev ) {
            if( strncmp(rev->metadir, "fs", 2) != 0 ) {
                return rev->openfile(rev->relative(pathname), commit);
            } else {
                return rev->openfile(pathname, commit);
            }
        }
#if 1
    }
#endif
    return NULL;
}


std::streambuf* gitcmd::shellcmd( const std::string& cmdline )
{
    using namespace boost::system;
    using namespace boost::filesystem;
    using namespace boost::iostreams;

    std::stringstream sstm;
    sstm << executable << cmdline; /* << " 2>&1";*/

    /* The git command needs to be issued from within a directory
       where the git config can be found by walking up the tree structure. */
    boost::filesystem::initial_path();
    boost::filesystem::current_path(rootpath);
#if 0
    std::cerr << "[gitshell] in " << boost::filesystem::current_path()
              << ": " << sstm.str() << std::endl;
#endif
    popen_streambuf *buf = new popen_streambuf();
    if( buf->open(sstm.str().c_str(), "r") == NULL ) {
        boost::throw_exception(system_error(1, system_category(), sstm.str()));
    }

    boost::filesystem::current_path(boost::filesystem::initial_path());
    return buf;
}


void gitcmd::diff( std::ostream& ostr,
    const std::string& leftCommit,
    const std::string& rightCommit,
    const boost::filesystem::path& pathname ) {

    std::stringstream cmd;
    cmd << " diff " << leftCommit << " " << rightCommit << " " << pathname;
    std::istream strm(shellcmd(cmd.str()));
    while( !strm.eof() ) {
        std::string line;
        std::getline(strm, line);
        ostr << line;
    }
}


void gitcmd::history( std::ostream& ostr,
    const session& s,
    const boost::filesystem::path& abspath,
    historyref& ref ) {

    /* The git command needs to be issued from within a directory
       where the git config can be found by walking up the tree structure. */
    boost::filesystem::initial_path();
    boost::filesystem::current_path(rootpath);

    std::stringstream sstm;
    boost::filesystem::path relpath = relative(abspath);
    sstm << " log --date=rfc --pretty=oneline -- " << relpath;
    std::istream strm(shellcmd(sstm.str()));

    while( !strm.eof() ) {
        std::string line;
        std::getline(strm, line);
        /* Parse the summary line in order to split the commit tag
           from the commit message. */

        size_t splitPos = line.find(' ');
        if( splitPos != std::string::npos ) {
            std::string rightRevision = line.substr(0,splitPos);
            std::string title = line.substr(splitPos);

            /* \todo '\n' at end of line? */
            ostr << html::a().href(ref.asUrl(s.asUrl(abspath).string(),
                    rightRevision).string()).title(title)
                 << rightRevision.substr(0,10) << "..."
                 << html::a::end << "<br />";
        }
    }
}


void gitcmd::loadconfig( session& s ) {
    using namespace boost::filesystem;
    static const boost::regex valueEx("(\\S+)\\s*=\\s*(.*)");
#if 0
    path configPath(rootpath / "config");
    std::streambuf *buf = filesys::openfile(configPath);
    std::istream configFile(buf);
    while( !configFile.eof() ) {
        boost::smatch m;
        std::string line;
        std::getline(configFile,line);
        if( boost::regex_search(line,m,valueEx) ) {
            config[m.str(1)] = m.str(2);
        }
    }
    delete buf;
#endif
}


void gitcmd::checkins( const session& s,
    const boost::filesystem::path& pathname,
    postFilter& filter ) {
    using namespace boost::filesystem;
    boost::filesystem::path project = projectName(s,rootpath);
    url base = s.asUrl(absolute(""));

    /* shows only the last 2 commits */
    std::stringstream sstm;
    sstm << " log --date=rfc --name-only -2 ";

    bool firstFile = true;
    bool itemStarted = false;
    bool descrStarted = false;
    post ci;
    std::string authorName;
    std::stringstream descr, link, title;
    std::istream strm(shellcmd(sstm.str()));

    while( !strm.eof() ) {
        std::string line;
        std::getline(strm, line);
        if( line.empty() ) continue;
        if( line.compare(0, 6, "commit") == 0 ) {
            if( descrStarted ) {
                if( !firstFile ) descr << html::pre::end;
                ci.link = link.str();
                ci.title = title.str();
                ci.content = descr.str();
                ci.normalize();
                filter.filters(ci);
                descrStarted = false;
            }
            firstFile = true;
            itemStarted = true;
            ci = post();
            // XXX lcstr[strlen(lcstr) - 1] = '\0'; // remove trailing '\n'
            title.str("");
            std::stringstream guid;
            guid << base << "commit/" << strip(line.substr(7));
            ci.guid = guid.str();

        } else if ( line.compare(0,7,"Author:") == 0 ) {
            /* The author field is formatted as "First Last <emailAddress>". */
            size_t first = 7;
            size_t last = first;
            while( last < line.size() ) {
                switch( line[last] ) {
                case '<':
                    authorName = line.substr(first,last - first);
                    first = last + 1;
                    break;
                case '>':
                    ci.author = contrib::find(line.substr(first,last - first),
                        authorName);
                    first = last + 1;
                    break;
                }
                ++last;
            }
        } else if ( line.compare(0,5,"Date:") == 0 ) {
            try {
                ci.time = from_mbox_string(line.substr(5));
            } catch( std::exception& e ) {
                std::cerr << "!!! exception " << e.what() << std::endl;
            }
        } else {
            /* more description */
            if( !descrStarted ) {
                link.str("");
                link << project << " &nbsp;&mdash;&nbsp; ";
                link << html::a().href(ci.guid) << basename(path(ci.guid)) << html::a::end << "<br />";
                descr.str("");
                descrStarted = true;
            }
            if( !isspace(line[0]) ) {
                /* We are dealing with a file that was part of this commit. */
                if( firstFile ) {
                    descr << html::pre();
                    firstFile = false;
                }
                writelink(descr,base.pathname,boost::filesystem::path(strip(line)));
                descr << std::endl;
            } else {
                size_t maxTitleLength = 80;
                if( title.str().size() < maxTitleLength ) {
                    size_t remain = (maxTitleLength - title.str().size());
                    if( line.size() > remain ) {
                        title << strip(line.substr(0,remain)) << "...";
                    } else {
                        title << strip(line);
                    }
                }
                descr << line << std::endl;
            }
        }
    }
    if( descrStarted ) {
        if( !firstFile ) descr << html::pre::end;
        ci.link = link.str();
        ci.title = title.str();
        ci.content = descr.str();
        ci.normalize();
        filter.filters(ci);
        descrStarted = false;
    }
}


void gitcmd::showDetails( std::ostream& ostr,
    const std::string& commit ) {

    std::stringstream sstm;
    sstm << " show " << commit;

    std::istream strm(shellcmd(sstm.str()));

    while( !strm.eof() ) {
        std::string line;
        std::getline(strm, line);
        ostr << line << std::endl;
    }
}


void gitcmd::show( std::ostream& ostr,
    const boost::filesystem::path& pathname,
    const std::string& commit,
    decorator *dec )
{
    using namespace boost;
    using namespace boost::system;

    if( dec ) {
        if( dec->formated() ) ostr << code();
        dec->attach(ostr);
    }

    std::stringstream sstm;
    sstm << " show " << commit << ":" << pathname;
    std::istream strm(shellcmd(sstm.str()));

    while( !strm.eof() ) {
        std::string line;
        std::getline(strm, line);
        ostr << line << '\n';
    }

    if( dec ) {
        dec->detach();
        if( dec->formated() ) ostr << html::pre::end;
    }
}


slice<char> gitcmd::loadtext( const boost::filesystem::path& pathname,
    const std::string& commit )
{
    std::vector<char> dyn_buff;
    std::stringstream sstm;
    sstm  << " show " << commit << ":" << pathname;

    std::istream strm(shellcmd(sstm.str()));
    while( !strm.eof() ) {
        std::string line;
        std::getline(strm, line);
        /* XXX Not really the most efficient to allocate and fill buffer
           while we do not know the size of the input file ... yet will
           do for now. */
        std::copy(line.begin(), line.end(), std::back_inserter(dyn_buff));
        dyn_buff.push_back('\n');
    }

    char *text = new char[ dyn_buff.size() + 1 ];
    std::copy(dyn_buff.begin(), dyn_buff.end(), text);
    text[dyn_buff.size()] = '\0';
    return slice<char>(text, &text[dyn_buff.size()]);
}


std::streambuf*
gitcmd::openfile( const boost::filesystem::path& pathname,
    const std::string& commit )
{
    std::stringstream sstm;
    sstm << " show " << commit << ":" << pathname;
    return shellcmd(sstm.str());
}


slice<char> filesys::loadtext( const boost::filesystem::path& pathname,
    const std::string& commit )
{
#if 0
    check(pathname);
#endif
    if( is_regular_file(pathname) ) {
        size_t fileSize = file_size(pathname);
        char *text = new char[ fileSize + 1 ];
        std::streambuf *buf = openfile(pathname);
        if( buf ) {
            std::istream file(buf);
            file.read(text, fileSize);
            delete buf;
        }
        text[fileSize] = '\0';
        /* +1 for zero but it would arbitrarly augment text -
           does not work for tokenizers. */
        return slice<char>(text, &text[fileSize]);
    }
    return slice<char>();
}


/** Open a file for reading.

    This function throws an exception if there is any error otherwise
    it returns ios_base::binary if the file seems to be a binary file
    and zero if it looks like a text file (analyzing first 16 bytes).
*/

std::streambuf* filesys::openfile( const boost::filesystem::path& pathname,
    const std::string& commit ) {
    using namespace boost::system::errc;
    using namespace boost::filesystem;
    using namespace boost::iostreams;

#if 0
    check(pathname);
#endif
    ifstream strm;
    strm.open(pathname);
    if( strm.fail() ) {
        /* \todo figure out how to pass iostream error code in exception. */
        boost::throw_exception(boost::system::system_error(
                make_error_code(no_such_file_or_directory),pathname.string()));
    }
    char firstBytes[16+1];
    int bytesRead = strm.readsome(firstBytes,16);
    strm.seekg(0,std::ios_base::beg);
    firstBytes[16] = '\0';
    for( int i = 0; i < bytesRead; ++i ) {
        if( !(isprint(firstBytes[i]) | isspace(firstBytes[i])) ) {
            /* '\n' is not a printable character according to the C standard. */
            return NULL;
        }
    }

    std::streambuf *buf
        = new stream_buffer<file_descriptor_source>(pathname,
            std::ios_base::in|std::ios_base::binary);
    return buf;
}


namespace detail {

void rev_directory_iterator_construct(
    rev_directory_iterator& iter,
    const boost::filesystem::path& abspath,
    boost::system::error_code* errcode,
    session& ses )
{
    revisionsys *rev = revisionsys::findRev(ses, abspath);
    if( !rev ) {
        using namespace boost::system::errc;
        boost::throw_exception(boost::system::system_error(
                make_error_code(no_such_file_or_directory), abspath.string()));
    }
    boost::filesystem::path relpath = rev->relative(abspath);
    std::stringstream strm;
    rev->show(strm, relpath, "HEAD");
    std::string buffer = strm.str();

    size_t prev = 0;
    bool started = false;
    size_t curr = buffer.find("\n", prev);
    while( curr != std::string::npos ) {
        if( curr != prev ) {
            if( started ) {
                /* The first line is a description of the .git blob
                   (i.e. something like tree HEAD:) */
                iter.m_imp->entries.push_back(buffer.substr(prev, curr - prev));
            } else {
                started = true;
            }
        }
        prev = curr + 1;
        curr = buffer.find("\n", prev);
    }

    boost::filesystem::file_status file_stat, symlink_file_stat;
    iter.m_imp->next = iter.m_imp->entries.begin();
    if( iter.m_imp->next != iter.m_imp->entries.end() ) {
        iter.m_imp->dir_entry.assign(*(iter.m_imp->next),
            file_stat, symlink_file_stat);
        ++(iter.m_imp->next);
    }
}


void rev_directory_iterator_increment(
    rev_directory_iterator& iter,
    boost::system::error_code* errcode )
{
    if( iter.m_imp->next == iter.m_imp->entries.end() ) {
        iter.m_imp.reset();
        return;
    }

    boost::filesystem::file_status file_stat, symlink_file_stat;
    iter.m_imp->dir_entry.assign(*(iter.m_imp->next),
        file_stat, symlink_file_stat);
    ++(iter.m_imp->next);
}

}
