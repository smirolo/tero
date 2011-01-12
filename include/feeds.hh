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

/* Different displays of changes to an underlying source control repository.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

/** Base class for commands displaying changes to a directory
 */
template<typename postFilter>
class changeDirRef : public document {
protected:
    boost::regex filematch;
    postFilter writer;

public:
    explicit changeDirRef( std::ostream& o, 
			   const boost::regex& filePat = boost::regex(".*") )
	: document(o), filematch(filePat), writer(o) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};

typedef changeDirRef<htmlwriter> htmlDirRef;
typedef changeDirRef<rsswriter> rssDirRef;


/** Base class for commands displaying changes to a directory
 */
template<typename postFilter>
class changeDirContent : public mailParser {
protected:
    postFilter writer;

public:
    changeDirContent( std::ostream& o, 
		      const boost::regex& filePat = boost::regex(".*") )
	: mailParser(o,filePat,writer,true), writer(o) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};

typedef changeDirContent<htmlwriter> htmlDirContent;
typedef changeDirContent<rsswriter> rssDirContent;


class changeAggregate : public document {
public:
    explicit changeAggregate( std::ostream& o )
	: document(o) {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};


#include "changefs.tcc"

#endif
