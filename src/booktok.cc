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

#include <cstring>
#include <iostream>
#include "booktok.hh"

const char* docbookKeywords[] = {
	"title",
	"titleabbrev",
	"subtitle",
	"info",
	"subjectset",
	"subject",
	"subjectterm",
	"keywordset",
	"keyword",
	"procedure",
	"step",
	"stepalternatives",
	"substeps",
	"sidebar",
	"abstract",
	"personblurb",
	"blockquote",
	"attribution",
	"bridgehead",
	"remark",
	"epigraph",
	"footnote",
	"formalpara",
	"para",
	"simpara",
	"itemizedlist",
	"orderedlist",
	"listitem",
	"segmentedlist",
	"segtitle",
	"seglistitem",
	"seg",
	"simplelist",
	"member",
	"variablelist",
	"varlistentry",
	"term",
	"example",
	"informalexample",
	"literallayout",
	"screen",
	"screenshot",
	"figure",
	"informalfigure",
	"mediaobject",
	"inlinemediaobject",
	"videoobject",
	"audioobject",
	"imageobject",
	"textobject",
	"videodata",
	"audiodata",
	"imagedata",
	"textdata",
	"caption",
	"address",
	"street",
	"pob",
	"postcode",
	"city",
	"state",
	"country",
	"phone",
	"fax",
	"otheraddr",
	"affiliation",
	"shortaffil",
	"jobtitle",
	"orgname",
	"orgdiv",
	"artpagenums",
	"personname",
	"author",
	"authorgroup",
	"collab",
	"authorinitials",
	"person",
	"org",
	"confgroup",
	"confdates",
	"conftitle",
	"confnum",
	"confsponsor",
	"contractnum",
	"contractsponsor",
	"copyright",
	"year",
	"holder",
	"cover",
	"date",
	"edition",
	"editor",
	"biblioid",
	"citebiblioid",
	"bibliosource",
	"bibliorelation",
	"bibliocoverage",
	"legalnotice",
	"othercredit",
	"pagenums",
	"contrib",
	"honorific",
	"firstname",
	"surname",
	"lineage",
	"othername",
	"printhistory",
	"pubdate",
	"publisher",
	"publishername",
	"releaseinfo",
	"revhistory",
	"revision",
	"revnumber",
	"revremark",
	"revdescription",
	"seriesvolnums",
	"volumenum",
	"issuenum",
	"package",
	"email",
	"lineannotation",
	"parameter",
	"replaceable",
	"uri",
	"abbrev",
	"acronym",
	"citation",
	"citerefentry",
	"refentrytitle",
	"manvolnum",
	"citetitle",
	"emphasis",
	"foreignphrase",
	"phrase",
	"quote",
	"subscript",
	"superscript",
	"trademark",
	"wordasword",
	"footnoteref",
	"xref",
	"link",
	"olink",
	"anchor",
	"alt",
	"set",
	"book",
	"dedication",
	"acknowledgements",
	"colophon",
	"appendix",
	"chapter",
	"part",
	"preface",
	"partintro",
	"section",
	"simplesect",
	"article",
	"annotation",
	"extendedlink",
	"locator",
	"arc",
	"sect1",
	"sect2",
	"sect3",
	"sect4",
	"sect5",
	"reference",
	"refentry",
	"refmeta",
	"refmiscinfo",
	"refnamediv",
	"refdescriptor",
	"refname",
	"refpurpose",
	"refclass",
	"refsynopsisdiv",
	"refsection",
	"refsect1",
	"refsect2",
	"refsect3",
	"glosslist",
	"glossentry",
	"glossdef",
	"glosssee",
	"glossseealso",
	"firstterm",
	"glossterm",
	"glossary",
	"glossdiv",
	"termdef",
	"biblioentry",
	"bibliomixed",
	"biblioset",
	"bibliomset",
	"bibliomisc",
	"bibliography",
	"bibliodiv",
	"bibliolist",
	"biblioref",
	"itermset",
	"indexterm",
	"primary",
	"secondary",
	"tertiary",
	"see",
	"seealso",
	"index",
	"setindex",
	"indexdiv",
	"indexentry",
	"primaryie",
	"secondaryie",
	"tertiaryie",
	"seeie",
	"seealsoie",
	"toc",
	"tocdiv",
	"tocentry",
	"task",
	"tasksummary",
	"taskprerequisites",
	"taskrelated",
	"calloutlist",
	"callout",
	"programlistingco",
	"areaspec",
	"area",
	"areaset",
	"screenco",
	"imageobjectco",
	"co",
	"coref",
	"productionset",
	"production",
	"lhs",
	"rhs",
	"nonterminal",
	"constraint",
	"productionrecap",
	"constraintdef",
	"tgroup",
	"colspec",
	"spanspec",
	"thead",
	"tfoot",
	"tbody",
	"row",
	"entry",
	"entrytbl",
	"table",
	"informaltable",
	"col",
	"colgroup",
	"tr",
	"th",
	"td",
	"msgset",
	"msgentry",
	"simplemsgentry",
	"msg",
	"msgmain",
	"msgsub",
	"msgrel",
	"msgtext",
	"msginfo",
	"msglevel",
	"msgorig",
	"msgaud",
	"msgexplan",
	"qandaset",
	"qandadiv",
	"qandaentry",
	"question",
	"answer",
	"label",
	"equation",
	"informalequation",
	"inlineequation",
	"mathphrase",
	"markup",
	"tag",
	"symbol",
	"token",
	"literal",
	"code",
	"constant",
	"productname",
	"productnumber",
	"database",
	"application",
	"hardware",
	"guibutton",
	"guiicon",
	"guilabel",
	"guimenu",
	"guimenuitem",
	"guisubmenu",
	"menuchoice",
	"mousebutton",
	"keycap",
	"keycode",
	"keycombo",
	"keysym",
	"accel",
	"shortcut",
	"prompt",
	"envar",
	"filename",
	"command",
	"computeroutput",
	"userinput",
	"cmdsynopsis",
	"arg",
	"group",
	"sbr",
	"synopfragment",
	"synopfragmentref",
	"synopsis",
	"funcsynopsis",
	"funcsynopsisinfo",
	"funcprototype",
	"funcdef",
	"function",
	"void",
	"varargs",
	"paramdef",
	"funcparams",
	"classsynopsis",
	"classsynopsisinfo",
	"ooclass",
	"oointerface",
	"ooexception",
	"modifier",
	"interfacename",
	"exceptionname",
	"fieldsynopsis",
	"initializer",
	"constructorsynopsis",
	"destructorsynopsis",
	"methodsynopsis",
	"methodname",
	"methodparam",
	"varname",
	"returnvalue",
	"type",
	"classname",
	"programlisting",
	"caution",
	"important",
	"note",
	"tip",
	"warning",
	"errorcode",
	"errorname",
	"errortext",
	"errortype",
	"systemitem",
	"option",
	"optional",
	"property"
};


docbookScanner::docbookScanner( std::istream& is ) 
  : istr(&is), tok(*this) {
    tokens.push_back(bookEof);
}


void docbookScanner::newline() {
    std::cerr << "newline()" << std::endl;
}
    

void docbookScanner::token( xmlToken token, const char *line, 
			    int first, int last, bool fragment ) {
    char str[255];
    strncpy(str,&line[first],last-first);
    str[last-first] = '\0';
    std::cerr << "xmlToken(" << token << ",\"" << str << "\"[" << first << ',' << last << "]," << fragment << ')' << std::endl; 
    /* translate XML start and end tags into docbook tokens */
    const char **key;
    const char **lastKeyword = &docbookKeywords[sizeof(docbookKeywords) / sizeof(char*)];	
    textSlice value(&line[first],&line[last]);
    switch( token ) {
    case xmlElementStart:
	text = textSlice();
	elementStart = true;
	break;
    case xmlElementEnd:
	elementStart = false;
	break;
    case xmlName:
#if 0
	keyworSet::const_iterator index 
	    = std::lower_bound(docbookKeywords.begin(),docbookKeywords.end(),
			       );
#else
	for( key = docbookKeywords; 
	     key != lastKeyword;
	     ++key ) {
	    if( strncmp(*key,value.begin(),value.size()) == 0 ) {
		break;
	    }
	}
#endif
	if( key != lastKeyword ) {
	    std::cerr << "push_back(" << *key << ")" << std::endl;
	    tokens.push_back((docbookToken)(2 *std::distance(docbookKeywords,key) 
					    + (elementStart ? 0 : 1) + 1));
	}
	break;
    case xmlContent:
	text += value;
	break;
    }
}


docbookToken docbookScanner::read() {
    tokens.pop_front();
    while( !istr->eof() && tokens.empty() ) {
	std::string s;
	std::getline(*istr,s);
	tok.tokenize(s.c_str(),s.size());
    }
    return tokens.empty() ? bookEof : *tokens.begin();
}
