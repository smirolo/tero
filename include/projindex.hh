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

#ifndef guardprojindex
#define guardprojindex

#include "projfiles.hh"

class checkstyle : public projfiles {
protected:    
    virtual void 
    addDir( const session& s, const boost::filesystem::path& pathname );

    virtual void 
    addFile( const session& s, const boost::filesystem::path& pathname );

    virtual void flush();
    
public:
    checkstyle() {}

    template<typename iter>
    checkstyle( iter first, iter last ) : projfiles(first,last) {}
};

/** Show a top-level page index of project.

    The project view description and dependencies of a project as stated 
    in the index.xml. A project view also contains the list of unit 
    failures, checkstyle failures and open issues. There are also links 
    to download <!-- through e-commerce transaction? --> the project as 
    a package, browse the source code and sign-on to the rss feed.
*/
class projindex : public document {
protected:
    /** name of the project 
     */
    std::string name;

public:
    projindex() {}

    virtual void fetch( session& s, const boost::filesystem::path& pathname );

    virtual void meta( session& s, const boost::filesystem::path& pathname );
};

#endif
