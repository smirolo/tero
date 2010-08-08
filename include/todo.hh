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

#ifndef guardtodo
#define guardtodo

#include "document.hh"
#include "mail.hh"
#include <boost/uuid/uuid.hpp>

boost::uuids::uuid todouuid( const boost::filesystem::path& p );


class todoFilter : public postFilter {
public:

    /** Pattern used to select directories containing todo items. 
     */
    static const boost::regex viewPat;

    /** Directory where modifications to todo item are stored.
     */
    const boost::filesystem::path modifs;

public:
    explicit todoFilter( const boost::filesystem::path& m )
	: modifs(m) {}

    todoFilter( const boost::filesystem::path& m, postFilter* n  ) 
	: postFilter(n), modifs(m) {}

    std::string asPath( const std::string& tag );
};


/** Creating an item or commenting on an already existing item 
    use very similar mechanism. This abstract class implements
    such mechanism to append a post to an item.
*/
class todoModifPost : public document {
protected:

    /** Directory where modifications to todo items are stored. */
    const boost::filesystem::path modifs;

    /** Input stream containing the modification post 
	formatted as an e-mail. */
    std::istream *istr;

    boost::filesystem::path asPath( const std::string& tag ) const;

public:
    todoModifPost( const boost::filesystem::path& m,
		    std::ostream& o ) 
	: document(o), modifs(m), istr(NULL) {}

    todoModifPost( const boost::filesystem::path& m,
		    std::ostream& o, 
		    std::istream& is ) 
	: document(o), modifs(m), istr(&is) {}

};


/** Create a new item

    Creating a new item involves computing a unique identifier,
    write the post in a file whose name is based on that identifier
    and finally commit the file into the repository.
*/
class todoCreate : public todoModifPost {
public:
    todoCreate( const boost::filesystem::path& m,
		std::ostream& o, 
		std::istream& is ) 
	: todoModifPost(m,o,is) {}

    void fetch( session& s, const boost::filesystem::path& pathname );
};


/** Comment an item

    Commmenting on a item involves finding the file matching the unique 
    identifier of that item, append the post at the end of that file 
    and finally commit the file back into the repository.
*/
class todoComment : public todoModifPost {
public:
    todoComment( const boost::filesystem::path& m,
		   std::ostream& o, 
		   std::istream& is ) 
	: todoModifPost(m,o,is) {}

    void fetch( session& s, const boost::filesystem::path& pathname );
};


/** Display an index of all items in a directory with one item per row
    with the rows sorted in descending score order.
 */
class todoIndexWriteHtml : public document {
protected:

    /* relative url for registering a vote. */
    const char *voteCommand;

public:
    todoIndexWriteHtml( std::ostream& o, const char *v ) 
	: document(o), voteCommand(v) {}

    void fetch( session& s, const boost::filesystem::path& pathname );
};


/** Callback when the process of voting on an item has been abandonned
 */
class todoVoteAbandon : public document {
public:
    explicit todoVoteAbandon( std::ostream& o ) : document(o) {}

    void fetch( session& s, const boost::filesystem::path& pathname );
};


/** Callback when a vote on an item has successed.
    
    First the item's unique identifier is used to find the file 
    that need to be updated. The file is then searched for the matching
    'Score:' pattern and that line is modified to reflect the vote.
    Finally the file is committed back into the repository.
 */
class todoVoteSuccess : public todoModifPost {
public:
    todoVoteSuccess( const boost::filesystem::path& m, std::ostream& o ) 
	: todoModifPost(m,o) {}

    void fetch( session& s, const boost::filesystem::path& pathname );
};


/** Generate an HTML printout of an item
 */
class todoWriteHtml : public document {
public:
    explicit todoWriteHtml( std::ostream& o ) : document(o) {}

    void meta( session& s, const boost::filesystem::path& pathname );

    void fetch(	session& s, const boost::filesystem::path& pathname );
};


#endif
