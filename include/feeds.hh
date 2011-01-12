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

#ifndef guardchangefs
#define guardchangefs

#include "mail.hh"
#include "post.hh"

/* Feeds can be generated out of repository commits or directories,
   aggregated together and presented as HTML or RSS.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


/** Aggregate feeds
 */
class feedAggregate : public document {
public:
    explicit feedAggregate( std::ostream& o )
	: document(o) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};


/** Feed of text file content from a directory
 */
template<typename postFilter>
class feedContent : public mailParser {
protected:
    postFilter writer;

public:
    feedContent( std::ostream& o, 
		 const boost::regex& filePat = boost::regex(".*") )
	: mailParser(o,filePat,writer,true), writer(o) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};

typedef feedContent<htmlwriter> htmlContent;
typedef feedContent<rsswriter> rssContent;


/** Feed of filenames from a directory 
 */
template<typename postFilter>
class feedNames : public document {
protected:
    postFilter writer;
    boost::regex filematch;

public:
    feedNames( std::ostream& o,
	       const boost::regex& filePat = boost::regex(".*") )
	: document(o), writer(o), filematch(filePat) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};

typedef feedNames<htmlwriter> htmlNames;
typedef feedNames<rsswriter> rssNames;


/** Feed from a repository commits
 */
template<typename postFilter>
class feedRepository : public document {
protected:
    postFilter writer;

public:
    explicit feedRepository( std::ostream& o ) 
	: document(o), writer(o) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname );	
};

typedef feedRepository<htmlwriter> htmlRepository;
typedef feedRepository<rsswriter> rssRepository;


#include "feeds.tcc"

#endif
