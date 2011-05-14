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

#include "docbook.hh"
#include "markup.hh"

/** Display docbook as HTML.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


namespace {

    void parseInfo( session& s, const rapidxml::xml_node<>& r ) {
	using namespace rapidxml;

	/* We found an <info> tag, let's parse the meta information 
	   about the article such as the author, the date, etc. */
	for( xml_node<> *n = r.first_node();
	     n != NULL; n = n->next_sibling() ) {
	    switch( n->type() ) {
	    case node_element:
#if 1
		/* \todo be careful with insert. code used to be s.vars[] = */
		/* author, title, date. */		
		s.insert(n->name(),n->value());
#endif
	    default:
		/* Nothing to do except prevent gcc from complaining. */
		break;
	    }
	}
    }

}  // anonymous namespace


docbook::walkNodeEntry docbook::walkers[] = {
    { "abbrev", &docbook::any, &docbook::any },
    { "abstract", &docbook::any, &docbook::any },
    { "accel", &docbook::any, &docbook::any },
    { "acknowledgements", &docbook::any, &docbook::any },
    { "acronym", &docbook::any, &docbook::any },
    { "address", &docbook::any, &docbook::any },
    { "affiliation", &docbook::any, &docbook::any },
    { "alt", &docbook::any, &docbook::any },
    { "anchor", &docbook::any, &docbook::any },
    { "annotation", &docbook::any, &docbook::any },
    { "answer", &docbook::any, &docbook::any },
    { "appendix", &docbook::any, &docbook::any },
    { "application", &docbook::any, &docbook::any },
    { "arc", &docbook::any, &docbook::any },
    { "area", &docbook::any, &docbook::any },
    { "areaset", &docbook::any, &docbook::any },
    { "areaspec", &docbook::any, &docbook::any },
    { "arg", &docbook::any, &docbook::any },
    { "article", &docbook::any, &docbook::any },
    { "artpagenums", &docbook::any, &docbook::any },
    { "attribution", &docbook::any, &docbook::any },
    { "audiodata", &docbook::any, &docbook::any },
    { "audioobject", &docbook::any, &docbook::any },
    { "author", &docbook::any, &docbook::any },
    { "authorgroup", &docbook::any, &docbook::any },
    { "authorinitials", &docbook::any, &docbook::any },
    { "bibliocoverage", &docbook::any, &docbook::any },
    { "bibliodiv", &docbook::any, &docbook::any },
    { "biblioentry", &docbook::any, &docbook::any },
    { "bibliography", &docbook::any, &docbook::any },
    { "biblioid", &docbook::any, &docbook::any },
    { "bibliolist", &docbook::any, &docbook::any },
    { "bibliomisc", &docbook::any, &docbook::any },
    { "bibliomixed", &docbook::any, &docbook::any },
    { "bibliomset", &docbook::any, &docbook::any },
    { "biblioref", &docbook::any, &docbook::any },
    { "bibliorelation", &docbook::any, &docbook::any },
    { "biblioset", &docbook::any, &docbook::any },
    { "bibliosource", &docbook::any, &docbook::any },
    { "blockquote", &docbook::any, &docbook::any },
    { "book", &docbook::any, &docbook::any },
    { "bridgehead", &docbook::any, &docbook::any },
    { "callout", &docbook::any, &docbook::any },
    { "calloutlist", &docbook::any, &docbook::any },
    { "caption", &docbook::captionStart, &docbook::captionEnd },
    { "caution", &docbook::any, &docbook::any },
    { "chapter", &docbook::any, &docbook::any },
    { "citation", &docbook::any, &docbook::any },
    { "citebiblioid", &docbook::any, &docbook::any },
    { "citerefentry", &docbook::any, &docbook::any },
    { "citetitle", &docbook::any, &docbook::any },
    { "city", &docbook::any, &docbook::any },
    { "classname", &docbook::any, &docbook::any },
    { "classsynopsis", &docbook::any, &docbook::any },
    { "classsynopsisinfo", &docbook::any, &docbook::any },
    { "cmdsynopsis", &docbook::any, &docbook::any },
    { "co", &docbook::any, &docbook::any },
    { "code", &docbook::any, &docbook::any },
    { "col", &docbook::any, &docbook::any },
    { "colgroup", &docbook::any, &docbook::any },
    { "collab", &docbook::any, &docbook::any },
    { "colophon", &docbook::any, &docbook::any },
    { "colspec", &docbook::any, &docbook::any },
    { "command", &docbook::any, &docbook::any },
    { "computeroutput", &docbook::any, &docbook::any },
    { "confdates", &docbook::any, &docbook::any },
    { "confgroup", &docbook::any, &docbook::any },
    { "confnum", &docbook::any, &docbook::any },
    { "confsponsor", &docbook::any, &docbook::any },
    { "conftitle", &docbook::any, &docbook::any },
    { "constant", &docbook::any, &docbook::any },
    { "constraint", &docbook::any, &docbook::any },
    { "constraintdef", &docbook::any, &docbook::any },
    { "constructorsynopsis", &docbook::any, &docbook::any },
    { "contractnum", &docbook::any, &docbook::any },
    { "contractsponsor", &docbook::any, &docbook::any },
    { "contrib", &docbook::any, &docbook::any },
    { "copyright", &docbook::any, &docbook::any },
    { "coref", &docbook::any, &docbook::any },
    { "country", &docbook::any, &docbook::any },
    { "cover", &docbook::any, &docbook::any },
    { "database", &docbook::any, &docbook::any },
    { "date", &docbook::any, &docbook::any },
    { "dedication", &docbook::any, &docbook::any },
    { "destructorsynopsis", &docbook::any, &docbook::any },
    { "edition", &docbook::any, &docbook::any },
    { "editor", &docbook::any, &docbook::any },
    { "email", &docbook::any, &docbook::any },
    { "emphasis", &docbook::emphasisStart, &docbook::emphasisEnd },
    { "entry", &docbook::any, &docbook::any },
    { "entrytbl", &docbook::any, &docbook::any },
    { "envar", &docbook::any, &docbook::any },
    { "epigraph", &docbook::any, &docbook::any },
    { "equation", &docbook::any, &docbook::any },
    { "errorcode", &docbook::any, &docbook::any },
    { "errorname", &docbook::any, &docbook::any },
    { "errortext", &docbook::any, &docbook::any },
    { "errortype", &docbook::any, &docbook::any },
    { "example", &docbook::any, &docbook::any },
    { "exceptionname", &docbook::any, &docbook::any },
    { "extendedlink", &docbook::any, &docbook::any },
    { "fax", &docbook::any, &docbook::any },
    { "fieldsynopsis", &docbook::any, &docbook::any },
    { "figure", &docbook::any, &docbook::any },
    { "filename", &docbook::any, &docbook::any },
    { "firstname", &docbook::any, &docbook::any },
    { "firstterm", &docbook::any, &docbook::any },
    { "footnote", &docbook::any, &docbook::any },
    { "footnoteref", &docbook::any, &docbook::any },
    { "foreignphrase", &docbook::any, &docbook::any },
    { "formalpara", &docbook::any, &docbook::any },
    { "funcdef", &docbook::any, &docbook::any },
    { "funcparams", &docbook::any, &docbook::any },
    { "funcprototype", &docbook::any, &docbook::any },
    { "funcsynopsis", &docbook::any, &docbook::any },
    { "funcsynopsisinfo", &docbook::any, &docbook::any },
    { "function", &docbook::any, &docbook::any },
    { "glossary", &docbook::any, &docbook::any },
    { "glossdef", &docbook::any, &docbook::any },
    { "glossdiv", &docbook::any, &docbook::any },
    { "glossentry", &docbook::any, &docbook::any },
    { "glosslist", &docbook::any, &docbook::any },
    { "glosssee", &docbook::any, &docbook::any },
    { "glossseealso", &docbook::any, &docbook::any },
    { "glossterm", &docbook::any, &docbook::any },
    { "group", &docbook::any, &docbook::any },
    { "guibutton", &docbook::any, &docbook::any },
    { "guiicon", &docbook::any, &docbook::any },
    { "guilabel", &docbook::any, &docbook::any },
    { "guimenu", &docbook::any, &docbook::any },
    { "guimenuitem", &docbook::any, &docbook::any },
    { "guisubmenu", &docbook::any, &docbook::any },
    { "hardware", &docbook::any, &docbook::any },
    { "holder", &docbook::any, &docbook::any },
    { "honorific", &docbook::any, &docbook::any },
    { "imagedata", &docbook::imagedataStart, &docbook::imagedataEnd },
    { "imageobject", &docbook::any, &docbook::any },
    { "imageobjectco", &docbook::any, &docbook::any },
    { "important", &docbook::any, &docbook::any },
    { "index", &docbook::any, &docbook::any },
    { "indexdiv", &docbook::any, &docbook::any },
    { "indexentry", &docbook::any, &docbook::any },
    { "indexterm", &docbook::any, &docbook::any },
    { "info", &docbook::infoStart, &docbook::infoEnd },
    { "informalequation", &docbook::any, &docbook::any },
    { "informalexample", &docbook::any, &docbook::any },
    { "informalfigure", &docbook::any, &docbook::any },
    { "informaltable", &docbook::informaltableStart, 
      &docbook::informaltableEnd },
    { "initializer", &docbook::any, &docbook::any },
    { "inlineequation", &docbook::any, &docbook::any },
    { "inlinemediaobject", &docbook::any, &docbook::any },
    { "interfacename", &docbook::any, &docbook::any },
    { "issuenum", &docbook::any, &docbook::any },
    { "itemizedlist", &docbook::listStart, &docbook::listEnd },
    { "itermset", &docbook::any, &docbook::any },
    { "jobtitle", &docbook::any, &docbook::any },
    { "keycap", &docbook::any, &docbook::any },
    { "keycode", &docbook::any, &docbook::any },
    { "keycombo", &docbook::any, &docbook::any },
    { "keysym", &docbook::any, &docbook::any },
    { "keyword", &docbook::any, &docbook::any },
    { "keywordset", &docbook::any, &docbook::any },
    { "label", &docbook::any, &docbook::any },
    { "legalnotice", &docbook::any, &docbook::any },
    { "lhs", &docbook::any, &docbook::any },
    { "lineage", &docbook::any, &docbook::any },
    { "lineannotation", &docbook::any, &docbook::any },
    { "link", &docbook::linkStart, &docbook::linkEnd },
    { "listitem", &docbook::itemStart, &docbook::itemEnd },
    { "literal", &docbook::any, &docbook::any },
    { "literallayout", &docbook::literallayoutStart, 
                       &docbook::literallayoutEnd },
    { "locator", &docbook::any, &docbook::any },
    { "manvolnum", &docbook::any, &docbook::any },
    { "markup", &docbook::any, &docbook::any },
    { "mathphrase", &docbook::any, &docbook::any },
    { "mediaobject", &docbook::any, &docbook::any },
    { "member", &docbook::itemStart, &docbook::itemEnd },
    { "menuchoice", &docbook::any, &docbook::any },
    { "methodname", &docbook::any, &docbook::any },
    { "methodparam", &docbook::any, &docbook::any },
    { "methodsynopsis", &docbook::any, &docbook::any },
    { "modifier", &docbook::any, &docbook::any },
    { "mousebutton", &docbook::any, &docbook::any },
    { "msg", &docbook::any, &docbook::any },
    { "msgaud", &docbook::any, &docbook::any },
    { "msgentry", &docbook::any, &docbook::any },
    { "msgexplan", &docbook::any, &docbook::any },
    { "msginfo", &docbook::any, &docbook::any },
    { "msglevel", &docbook::any, &docbook::any },
    { "msgmain", &docbook::any, &docbook::any },
    { "msgorig", &docbook::any, &docbook::any },
    { "msgrel", &docbook::any, &docbook::any },
    { "msgset", &docbook::any, &docbook::any },
    { "msgsub", &docbook::any, &docbook::any },
    { "msgtext", &docbook::any, &docbook::any },
    { "nonterminal", &docbook::any, &docbook::any },
    { "note", &docbook::any, &docbook::any },
    { "olink", &docbook::any, &docbook::any },
    { "ooclass", &docbook::any, &docbook::any },
    { "ooexception", &docbook::any, &docbook::any },
    { "oointerface", &docbook::any, &docbook::any },
    { "option", &docbook::any, &docbook::any },
    { "optional", &docbook::any, &docbook::any },
    { "orderedlist", &docbook::any, &docbook::any },
    { "org", &docbook::any, &docbook::any },
    { "orgdiv", &docbook::any, &docbook::any },
    { "orgname", &docbook::any, &docbook::any },
    { "otheraddr", &docbook::any, &docbook::any },
    { "othercredit", &docbook::any, &docbook::any },
    { "othername", &docbook::any, &docbook::any },
    { "package", &docbook::any, &docbook::any },
    { "pagenums", &docbook::any, &docbook::any },
    { "para", &docbook::paraStart, &docbook::paraEnd },
    { "paramdef", &docbook::any, &docbook::any },
    { "parameter", &docbook::any, &docbook::any },
    { "part", &docbook::any, &docbook::any },
    { "partintro", &docbook::any, &docbook::any },
    { "person", &docbook::any, &docbook::any },
    { "personblurb", &docbook::any, &docbook::any },
    { "personname", &docbook::any, &docbook::any },
    { "phone", &docbook::any, &docbook::any },
    { "phrase", &docbook::phraseStart, &docbook::phraseEnd },
    { "pob", &docbook::any, &docbook::any },
    { "postcode", &docbook::any, &docbook::any },
    { "preface", &docbook::any, &docbook::any },
    { "primary", &docbook::any, &docbook::any },
    { "primaryie", &docbook::any, &docbook::any },
    { "printhistory", &docbook::any, &docbook::any },
    { "procedure", &docbook::any, &docbook::any },
    { "production", &docbook::any, &docbook::any },
    { "productionrecap", &docbook::any, &docbook::any },
    { "productionset", &docbook::any, &docbook::any },
    { "productname", &docbook::any, &docbook::any },
    { "productnumber", &docbook::any, &docbook::any },
    { "programlisting", &docbook::programlistingStart, 
      &docbook::programlistingEnd },
    { "programlistingco", &docbook::any, &docbook::any },
    { "prompt", &docbook::any, &docbook::any },
    { "property", &docbook::any, &docbook::any },
    { "pubdate", &docbook::any, &docbook::any },
    { "publisher", &docbook::any, &docbook::any },
    { "publishername", &docbook::any, &docbook::any },
    { "qandadiv", &docbook::any, &docbook::any },
    { "qandaentry", &docbook::any, &docbook::any },
    { "qandaset", &docbook::any, &docbook::any },
    { "question", &docbook::any, &docbook::any },
    { "quote", &docbook::any, &docbook::any },
    { "refclass", &docbook::any, &docbook::any },
    { "refdescriptor", &docbook::any, &docbook::any },
    { "refentry", &docbook::any, &docbook::any },
    { "refentrytitle", &docbook::titleStart, &docbook::titleEnd },
    { "reference", &docbook::any, &docbook::any },
    { "refmeta", &docbook::infoStart, &docbook::infoEnd },
    { "refmiscinfo", &docbook::any, &docbook::any },
    { "refname", &docbook::any, &docbook::any },
    { "refnamediv", &docbook::any, &docbook::any },
    { "refpurpose", &docbook::any, &docbook::any },
    { "refsect1", &docbook::any, &docbook::any },
    { "refsect2", &docbook::any, &docbook::any },
    { "refsect3", &docbook::any, &docbook::any },
    { "refsection", &docbook::sectionStart, &docbook::sectionEnd },
    { "refsynopsisdiv", &docbook::any, &docbook::any },
    { "releaseinfo", &docbook::any, &docbook::any },
    { "remark", &docbook::any, &docbook::any },
    { "replaceable", &docbook::any, &docbook::any },
    { "returnvalue", &docbook::any, &docbook::any },
    { "revdescription", &docbook::any, &docbook::any },
    { "revhistory", &docbook::any, &docbook::any },
    { "revision", &docbook::any, &docbook::any },
    { "revnumber", &docbook::any, &docbook::any },
    { "revremark", &docbook::any, &docbook::any },
    { "rhs", &docbook::any, &docbook::any },
    { "row", &docbook::any, &docbook::any },
    { "sbr", &docbook::any, &docbook::any },
    { "screen", &docbook::any, &docbook::any },
    { "screenco", &docbook::any, &docbook::any },
    { "screenshot", &docbook::any, &docbook::any },
    { "secondary", &docbook::any, &docbook::any },
    { "secondaryie", &docbook::any, &docbook::any },
    { "sect1", &docbook::any, &docbook::any },
    { "sect2", &docbook::any, &docbook::any },
    { "sect3", &docbook::any, &docbook::any },
    { "sect4", &docbook::any, &docbook::any },
    { "sect5", &docbook::any, &docbook::any },
    { "section", &docbook::sectionStart, &docbook::sectionEnd },
    { "see", &docbook::any, &docbook::any },
    { "seealso", &docbook::any, &docbook::any },
    { "seealsoie", &docbook::any, &docbook::any },
    { "seeie", &docbook::any, &docbook::any },
    { "seg", &docbook::any, &docbook::any },
    { "seglistitem", &docbook::any, &docbook::any },
    { "segmentedlist", &docbook::any, &docbook::any },
    { "segtitle", &docbook::any, &docbook::any },
    { "seriesvolnums", &docbook::any, &docbook::any },
    { "set", &docbook::any, &docbook::any },
    { "setindex", &docbook::any, &docbook::any },
    { "shortaffil", &docbook::any, &docbook::any },
    { "shortcut", &docbook::any, &docbook::any },
    { "sidebar", &docbook::any, &docbook::any },
    { "simpara", &docbook::any, &docbook::any },
    { "simplelist", &docbook::listStart, &docbook::listEnd },
    { "simplemsgentry", &docbook::any, &docbook::any },
    { "simplesect", &docbook::any, &docbook::any },
    { "spanspec", &docbook::any, &docbook::any },
    { "state", &docbook::any, &docbook::any },
    { "step", &docbook::any, &docbook::any },
    { "stepalternatives", &docbook::any, &docbook::any },
    { "street", &docbook::any, &docbook::any },
    { "subject", &docbook::any, &docbook::any },
    { "subjectset", &docbook::any, &docbook::any },
    { "subjectterm", &docbook::any, &docbook::any },
    { "subscript", &docbook::any, &docbook::any },
    { "substeps", &docbook::any, &docbook::any },
    { "subtitle", &docbook::any, &docbook::any },
    { "superscript", &docbook::any, &docbook::any },
    { "surname", &docbook::any, &docbook::any },
    { "symbol", &docbook::any, &docbook::any },
    { "synopfragment", &docbook::any, &docbook::any },
    { "synopfragmentref", &docbook::any, &docbook::any },
    { "synopsis", &docbook::any, &docbook::any },
    { "systemitem", &docbook::any, &docbook::any },
    { "table", &docbook::tableStart, &docbook::tableEnd },
    { "tag", &docbook::any, &docbook::any },
    { "task", &docbook::any, &docbook::any },
    { "taskprerequisites", &docbook::any, &docbook::any },
    { "taskrelated", &docbook::any, &docbook::any },
    { "tasksummary", &docbook::any, &docbook::any },
    { "tbody", &docbook::any, &docbook::any },
    { "td", &docbook::tdStart, &docbook::tdEnd },
    { "term", &docbook::any, &docbook::any },
    { "termdef", &docbook::any, &docbook::any },
    { "tertiary", &docbook::any, &docbook::any },
    { "tertiaryie", &docbook::any, &docbook::any },
    { "textdata", &docbook::any, &docbook::any },
    { "textobject", &docbook::any, &docbook::any },
    { "tfoot", &docbook::any, &docbook::any },
    { "tgroup", &docbook::any, &docbook::any },
    { "th", &docbook::thStart, &docbook::thEnd },
    { "thead", &docbook::any, &docbook::any },
    { "tip", &docbook::any, &docbook::any },
    { "title", &docbook::titleStart, &docbook::titleEnd },
    { "titleabbrev", &docbook::any, &docbook::any },
    { "toc", &docbook::any, &docbook::any },
    { "tocdiv", &docbook::any, &docbook::any },
    { "tocentry", &docbook::any, &docbook::any },
    { "token", &docbook::any, &docbook::any },
    { "tr", &docbook::trStart, &docbook::trEnd },
    { "trademark", &docbook::any, &docbook::any },
    { "type", &docbook::any, &docbook::any },
    { "uri", &docbook::any, &docbook::any },
    { "userinput", &docbook::any, &docbook::any },
    { "varargs", &docbook::any, &docbook::any },
    { "variablelist", &docbook::any, &docbook::any },
    { "varlistentry", &docbook::any, &docbook::any },
    { "varname", &docbook::any, &docbook::any },
    { "videodata", &docbook::any, &docbook::any },
    { "videoobject", &docbook::any, &docbook::any },
    { "void", &docbook::any, &docbook::any },
    { "volumenum", &docbook::any, &docbook::any },
    { "warning", &docbook::any, &docbook::any },
    { "wordasword", &docbook::any, &docbook::any },
    { "xref", &docbook::xrefStart, &docbook::xrefEnd },
    { "year", &docbook::any, &docbook::any }
};

bool docbook::walkNodeEntry::operator<( const walkNodeEntry& right ) const {
    return strcmp(name,right.name) < 0;
}


docbook::docbook( decorator& l,  decorator& r ) 
    : text(l,r), 
      info(false), linebreak(false), sectionLevel(0) {}


void docbook::any( session& s, const rapidxml::xml_node<>& node ) const {
}

void docbook::captionEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::caption::end;
    }
}

void docbook::captionStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::caption();
    }
}

void docbook::emphasisEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::span::end;
    }
}

void docbook::emphasisStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::span().classref("emphasis");
    }
}

void docbook::imagedataEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::img::end;
    }
}

void docbook::imagedataStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	rapidxml::xml_attribute<> *fileref = node.first_attribute("fileref");
	rapidxml::xml_attribute<> *role = node.first_attribute("role");
	if( fileref != NULL ) {	
	    if( role != NULL ) {
		s.out() << html::img().src(url(fileref->value())).classref(role->value());
	    } else {
		s.out() << html::img().src(url(fileref->value()));
	    }
	} else {
	    s.out() << html::img();
	}
    }
}

void docbook::infoEnd( session& s, const rapidxml::xml_node<>& node ) const {
    info = false;
}

void docbook::infoStart( session& s, const rapidxml::xml_node<>& node ) const {
    info = true;
}

void docbook::informaltableEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::table::end;
    }
}

void docbook::informaltableStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::table();
    }
}

void docbook::linkEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::a::end;
    }
}

void docbook::linkStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	rapidxml::xml_attribute<> *href = node.first_attribute("xlink:href");
	if( href != NULL ) {
	    s.out() << html::a().href(href->value());
	} else {
	    href = node.first_attribute("linkend");
	    if( href != NULL ) {
		/* works for both fop and semilla but validbook complains 
		   /Volumes/Home/smirolo/workspace/fortylines/dev/reps/whitepapers/doc/glossary.book:18: validity error : xml:id : attribute value glossary.book#localMachine is not an NCName
		   <glossentry xml:id="glossary.book#localMachine">
		*/
		url u(href->value());
		url h = u;
		std::string name(boost::filesystem::basename(u.pathname) + std::string(".book"));
		std::string ext = boost::filesystem::extension(u.pathname);
		if( !ext.empty() ) {
		    ext[0] = '#';
		    name += ext;
		}
		h.pathname = name;
		s.out() << html::a().href(h.string());
	    } else {
		s.out() << html::a();
	    }
	}
    }
}


void docbook::literallayoutEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	linebreak = false;
    }
}


void docbook::literallayoutStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	linebreak = true;
    }
}


void docbook::itemEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::li::end;
    }
}

void docbook::itemStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::li();
    }
}

void docbook::paraEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	rapidxml::xml_attribute<> *role = node.first_attribute("role");
	if( role != NULL ) { 
	    if( strncmp(role->value(),"code",4) == 0 ) {
		s.out() << html::pre::end;
	    } else {
		s.out() << html::p::end;
	    }
	} else {
	    s.out() << html::p::end;
	}
    }
}

void docbook::paraStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	rapidxml::xml_attribute<> *role = node.first_attribute("role");
	if( role != NULL ) { 
	    if( strncmp(role->value(),"code",4) == 0 ) {
		s.out() << html::pre().classref(role->value());
	    } else {
		s.out() << html::p().classref(role->value());
	    }
	} else {
	    s.out() << html::p();
	}
    }
}

void docbook::phraseEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::span::end;
    }
}

void docbook::phraseStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	rapidxml::xml_attribute<> *role = node.first_attribute("role");
	if( role != NULL ) { 
	    s.out() << html::span().classref(role->value());
	} else {
	    s.out() << html::span();
	}
    }
}

void docbook::programlistingEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << code::end;
    }
}

void docbook::programlistingStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << code();
    }
}

void docbook::sectionEnd( session& s, const rapidxml::xml_node<>& node ) const {
    --sectionLevel;
}

void docbook::sectionStart( session& s, const rapidxml::xml_node<>& node ) const {
    ++sectionLevel;
}

void docbook::listEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::ul::end;
    }
}

void docbook::listStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::ul();
    }
}

void docbook::tableEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::table::end;
    }
}

void docbook::tableStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::table();
    }
}

void docbook::tdEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::td::end;
    }
}

void docbook::tdStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::td();
    }
}

void docbook::thEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::th::end;
    }
}

void docbook::thStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::th();
    }
}

void docbook::titleEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::h(sectionLevel - 1).end();
    }
}

void docbook::titleStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::h(sectionLevel - 1);
    }
}

void docbook::trEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::tr::end;
    }
}

void docbook::trStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
        s.out() << html::tr();
    }
}


void docbook::xrefEnd( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	s.out() << html::a::end;
    }
}

void docbook::xrefStart( session& s, const rapidxml::xml_node<>& node ) const {
    if( !info ) {
	rapidxml::xml_attribute<> *linkend = node.first_attribute("linkend");
	if( linkend != NULL ) {	
	    s.out() << html::a().href(std::string("glossary.book#") 
					+ linkend->value());
	    s.out() << linkend->value();
	} else {
	    s.out() << html::a();
	}
    }
}

void docbook::walk( session& s, const rapidxml::xml_node<>& node ) const {
    using namespace rapidxml;

    walkNodeEntry *walkersEnd 
	= &walkers[sizeof(walkers)/sizeof(walkNodeEntry)];
    walkNodeEntry *d;
    walkNodeEntry key;

    switch( node.type() ) {
    case node_element:
	key.name = node.name();
	d = std::lower_bound(walkers,walkersEnd,key);
	if( d != walkersEnd ) (this->*(d->start))(s,node);
	break;
    case node_data:
    case node_cdata:
	if( !info ) {
	    s.out() << node.value();
	    if( linebreak ) s.out() << "<br />" << std::endl;
	}
	break;
    default:
	/* \todo currently we only prevent gcc from complaining but
	   there are more cases that might have to be handled here. */
	break;
    }

    for( xml_node<> *child = node.first_node();
	 child != NULL; child = child->next_sibling() ) {
	walk(s,*child);
    }

    switch( node.type() ) {
    case node_element:
	if( d != walkersEnd ) (this->*(d->end))(s,node);
	break;
    default:
	/* \todo currently we only prevent gcc from complaining but
	   there are more cases that might have to be handled here. */
	break;
    }
}

namespace {
char titleMeta[] = "title";
} // anonymous


void docbookMeta( session& s, const boost::filesystem::path& pathname )
{
    using namespace rapidxml;

    /* \todo should only load one but how does it sits with dispatchDoc
     that initializes s[varname] by default to "document"? */

    rapidxml::xml_document<> *doc = s.loadxml(pathname);

    xml_node<> *root = doc->first_node();
    if( root != NULL ) {
	xml_node<> *info = root->first_node("info");
	if( info == NULL ) {
	    info = root->first_node("refmeta");
	}
	if( info != NULL ) {
	    parseInfo(s,*info);
	}
    }
    session::variables::const_iterator found = s.find("title");
    if( !s.found(found) ) {
	s.insert("title",document.value(s).string());
    }    
    metaFetch<titleMeta>(s,pathname);
}


void docbookFetch( session& s, const boost::filesystem::path& pathname )
{    
    linkLight leftFormatedText(s);
    linkLight rightFormatedText(s);
    docbook d(leftFormatedText,rightFormatedText);

    d.doc = s.loadxml(pathname);
    d.leftDec->attach(s.out());
    rapidxml::xml_node<> *root = d.doc->first_node();
    if( root != NULL ) {
	d.walk(s,*root);
    }
    d.leftDec->detach();
}
