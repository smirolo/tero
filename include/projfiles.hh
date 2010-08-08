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

#ifndef guardprojfiles
#define guardprojfiles

#include "document.hh"

class projfiles : public document {
public:
    typedef std::list<boost::regex> filterContainer;

    enum stateCode {
	start,
	toplevelFiles,
	direntryFiles
    };

    stateCode state;

protected:
    filterContainer filters;
    
    /** directory in the source tree which is the root of the project (srcDir)
     */
    boost::filesystem::path projdir;
    
    virtual void 
    addDir( const session& s, const boost::filesystem::path& pathname );

    virtual void 
    addFile( const session& s, const boost::filesystem::path& pathname );

    virtual void flush();

    /** returns true when the pathname matches one of the pattern in *filters*.
     */
    bool selects( const boost::filesystem::path& pathname ) const;
    
public:
    explicit projfiles( std::ostream& o ) : document(o) {}
    
    template<typename iter>
    projfiles( std::ostream& o, iter first, iter last )
	: document(o) {
	std::copy(first,last,std::back_inserter(filters));
    }

    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};


#endif
