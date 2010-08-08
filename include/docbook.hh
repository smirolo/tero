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

#ifndef guarddocbook
#define guarddocbook

#include <boost/property_tree/detail/rapidxml.hpp>
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
*/
class docbook : public text {
protected:
    typedef void 
    (docbook::* walkNodePtr) ( const rapidxml::xml_node<>& node );

    struct walkNodeEntry {
	const char* name;
	walkNodePtr start;
	walkNodePtr end;

	bool operator<( const walkNodeEntry& right ) const;
    };

    static walkNodeEntry walkers[];

    char *buffer;
    size_t length;
    rapidxml::xml_document<> doc;

    bool info;
    bool linebreak;
    int sectionLevel;
   
    void any( const rapidxml::xml_node<>& node );

    void captionEnd( const rapidxml::xml_node<>& node );
    void captionStart( const rapidxml::xml_node<>& node );
    void emphasisEnd( const rapidxml::xml_node<>& node );
    void emphasisStart( const rapidxml::xml_node<>& node );
    void imagedataEnd( const rapidxml::xml_node<>& node );
    void imagedataStart( const rapidxml::xml_node<>& node );
    void infoEnd( const rapidxml::xml_node<>& node );
    void infoStart( const rapidxml::xml_node<>& node );
    void informaltableEnd( const rapidxml::xml_node<>& node );
    void informaltableStart( const rapidxml::xml_node<>& node );
    void linkEnd( const rapidxml::xml_node<>& node );
    void linkStart( const rapidxml::xml_node<>& node );
    void literallayoutStart( const rapidxml::xml_node<>& node );
    void literallayoutEnd( const rapidxml::xml_node<>& node );
    void itemEnd( const rapidxml::xml_node<>& node );
    void itemStart( const rapidxml::xml_node<>& node );
    void paraEnd( const rapidxml::xml_node<>& node );
    void paraStart( const rapidxml::xml_node<>& node );
    void programlistingStart( const rapidxml::xml_node<>& node );
    void programlistingEnd( const rapidxml::xml_node<>& node );
    void phraseEnd( const rapidxml::xml_node<>& node );
    void phraseStart( const rapidxml::xml_node<>& node );
    void sectionEnd( const rapidxml::xml_node<>& node );
    void sectionStart( const rapidxml::xml_node<>& node );
    void listEnd( const rapidxml::xml_node<>& node );
    void listStart( const rapidxml::xml_node<>& node );
    void tableEnd( const rapidxml::xml_node<>& node );
    void tableStart( const rapidxml::xml_node<>& node );
    void tdEnd( const rapidxml::xml_node<>& node );
    void tdStart( const rapidxml::xml_node<>& node );
    void thEnd( const rapidxml::xml_node<>& node );
    void thStart( const rapidxml::xml_node<>& node );
    void titleEnd( const rapidxml::xml_node<>& node );
    void titleStart( const rapidxml::xml_node<>& node );
    void trEnd( const rapidxml::xml_node<>& node );
    void trStart( const rapidxml::xml_node<>& node );
    void xrefEnd( const rapidxml::xml_node<>& node );
    void xrefStart( const rapidxml::xml_node<>& node );

    void walk( const rapidxml::xml_node<>& node );

public:
    docbook( std::ostream& o, decorator& l,  decorator& r );

    ~docbook();

    virtual void meta( session& s, const boost::filesystem::path& pathname );

    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};

#endif
