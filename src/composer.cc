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

#include <iostream>
#include <boost/regex.hpp>
#include <boost/filesystem/fstream.hpp>
#include "composer.hh"

/** Document composition

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

class composerListener : public xmlTokListener {
protected:
    session& s;
    size_t elementDepth;
    size_t replaceDepth;
    enum {
        composeInitial = 0,
        composeElemStart,
        composeClassStart,
        composeInComposition,
    } state;

    std::stringstream buffer;

#if 0
    void fetchAndReplace( session& s ) {
        std::ostream& prevDisp = s.out();
        try {
            path prev = current_path();
            current_path(s.prefixdir(fixed));
            dispatchDoc::instance()->fetch(s,widget,url(value));
            current_path(prev);
        } catch( const std::runtime_error& e ) {
            s.out(prevDisp);
            s.feeds = NULL; /* ok here since those objects were
                               allocated on the stack. */
            ++s.nErrs;
            std::cerr << "[embed of '" << value << "'] "
                      << e.what() << std::endl;
            s.out() << "<p>" << e.what() << "</p>" << std::endl;
        }
    }
#endif

public:
    explicit composerListener( session& ps )
        : s(ps), elementDepth(0), replaceDepth(0), state(composeInitial) {}

    virtual void newline( const char *line, int first, int last ) {
        static const char *endofline = "\n";
        if( state == composeInitial ) {
            // flush buffer
            buffer.write(endofline, 1);
            s.out() << buffer.str();
            buffer.str("");
        } else if( state != composeInComposition ) {
            buffer.write(endofline, 1);
        }
    }

    virtual void token( xmlToken token, const char *line,
        int first, int last, bool fragment ) {
        switch( token ) {
        case xmlErr:
        case xmlAssign:
            break;
        case xmlAttValue:
            if( state == composeClassStart ) {
                std::string att(&line[first + 1], last - first - 2);
                const fetchEntry* entry = dispatchDoc::instance()->select(
                    att, document.value(s).string());
                if( entry ) {
                    // XXX && with found a match
                    replaceDepth = elementDepth - 1;
                    state = composeInComposition;
                    std::cerr << "[found entry for " << att << " at depth "
                              << replaceDepth << "]" << std::endl;
                    buffer.str("");
                    dispatchDoc::instance()->fetch(s, att, document.value(s));
                }
            }
            break;
        case xmlCloseTag:
            if( state == composeElemStart || state == composeClassStart ) {
                // no appropriate class, let's flush
                state = composeInitial;
            }
            break;
        case xmlComment:
        case xmlContent:
        case xmlDeclEnd:
        case xmlDeclStart:
            break;
        case xmlElementEnd:
        case xmlEmptyElementEnd:
            if( elementDepth == replaceDepth ) {
                // flushing
                state = composeInitial;
                std::cerr << "[restore after "
                          << std::string(&line[first], last - first)
                          << "at depth " << replaceDepth << "]"
                          << std::endl;
            }
            --elementDepth;
            break;
        case xmlElementStart:
            // start buffering
            ++elementDepth;
            state = composeElemStart;
            break;
        case xmlName:
            if( strncmp(&line[first], "class", last - first) == 0 ) {
                state = composeClassStart;
            }
            break;
        case xmlSpace:
            break;
        }
        if( state == composeInitial ) {
            // flush buffer
            buffer.write(&line[first], last - first);
            s.out() << buffer.str();
            buffer.str("");
        } else if( state != composeInComposition ) {
            // buffer
            buffer.write(&line[first], last - first);
        }
    }
};


pathVariable themeDir("themeDir",
    "directory that contains the user interface templates");

void
composerAddSessionVars( boost::program_options::options_description& opts,
    boost::program_options::options_description& visible )
{
    using namespace boost::program_options;

    options_description localOpts("composer");
    localOpts.add(themeDir.option());
    opts.add(localOpts);
    visible.add(localOpts);

    options_description hiddenOpts("hidden");
    hiddenOpts.add_options()
        ("document",value<std::string>(),"document");
    opts.add(hiddenOpts);
}


void
compose( session& s, std::istream& strm, const boost::filesystem::path& fixed )
{
    using namespace boost;
    using namespace boost::system;
    using namespace boost::filesystem;

    composerListener cmpl(s);
    xmlTokenizer tokenizer(cmpl);

    std::string line;
    std::getline(strm, line);
    while( !strm.eof() ) {
        tokenizer.tokenize(line.c_str(), line.size());
        cmpl.newline(NULL, 0, 0);
        std::getline(strm, line);
    }
}
