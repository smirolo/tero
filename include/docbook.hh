/* Copyright (c) 2009-2011, Fortylines LLC
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

#ifndef guarddocbook
#define guarddocbook

#include "document.hh"
#include "decorator.hh"
#include "booktok.hh"


/** Presents a file formatted with docbook markup as HTML.

    In semilla, formatted docbook is usually out-of-line design 
    documentation intended to tie up with the source code.
    The articles we are dealing with are relatively short
    and wiki-style so we are transforming them entirely 
    in the following C++ class without resorting to external
    XSLT processors, etc.

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>

    \todo the buffering of the input document was removed 
    in order to split docbook from docbookMeta. This was necessary
    to simplify generation of feeds.
    (We need the meta information at this point but will only print 
    the formatted text when fetch() is called later on.
    We load the text *buffer* and keep it around in order to parse 
    the XML only once.)
*/
class docbook : public text {
protected:

    friend void docbookFetch( session& s, 
				  const boost::filesystem::path& pathname );

    typedef void 
    (docbook::* walkNodePtr) ( session& s, const rapidxml::xml_node<>& node ) const;

    struct walkNodeEntry {
	const char* name;
	walkNodePtr start;
	walkNodePtr end;

	bool operator<( const walkNodeEntry& right ) const;
    };

    static walkNodeEntry walkers[];

    mutable rapidxml::xml_document<> *doc;

    mutable bool info;
    mutable bool linebreak;
    mutable int sectionLevel;
   
    void any( session& s, const rapidxml::xml_node<>& node ) const;

    void captionEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void captionStart( session& s, const rapidxml::xml_node<>& node ) const;
    void emphasisEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void emphasisStart( session& s, const rapidxml::xml_node<>& node ) const;
    void imagedataEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void imagedataStart( session& s, const rapidxml::xml_node<>& node ) const;
    void infoEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void infoStart( session& s, const rapidxml::xml_node<>& node ) const;
    void informaltableEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void informaltableStart( session& s, const rapidxml::xml_node<>& node ) const;
    void linkEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void linkStart( session& s, const rapidxml::xml_node<>& node ) const;
    void literallayoutStart( session& s, const rapidxml::xml_node<>& node ) const;
    void literallayoutEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void itemEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void itemStart( session& s, const rapidxml::xml_node<>& node ) const;
    void paraEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void paraStart( session& s, const rapidxml::xml_node<>& node ) const;
    void programlistingStart( session& s, const rapidxml::xml_node<>& node ) const;
    void programlistingEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void phraseEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void phraseStart( session& s, const rapidxml::xml_node<>& node ) const;
    void sectionEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void sectionStart( session& s, const rapidxml::xml_node<>& node ) const;
    void listEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void listStart( session& s, const rapidxml::xml_node<>& node ) const;
    void tableEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void tableStart( session& s, const rapidxml::xml_node<>& node ) const;
    void tdEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void tdStart( session& s, const rapidxml::xml_node<>& node ) const;
    void thEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void thStart( session& s, const rapidxml::xml_node<>& node ) const;
    void titleEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void titleStart( session& s, const rapidxml::xml_node<>& node ) const;
    void trEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void trStart( session& s, const rapidxml::xml_node<>& node ) const;
    void xrefEnd( session& s, const rapidxml::xml_node<>& node ) const;
    void xrefStart( session& s, const rapidxml::xml_node<>& node ) const;

    void walk( session& s, const rapidxml::xml_node<>& node ) const;

public:
    docbook( decorator& l,  decorator& r );
};

void docbookFetch( session& s, const boost::filesystem::path& pathname );


/** Load meta information associated to a docbook file. (*whenFileExist*) 

    class docbookMeta : public meta    
 */
void docbookMeta( session& s, const boost::filesystem::path& pathname );

#endif
