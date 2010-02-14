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
	"abbrev",
	"abstract",
	"accel",
	"acknowledgements",
	"acronym",
	"address",
	"affiliation",
	"alt",
	"anchor",
	"annotation",
	"answer",
	"appendix",
	"application",
	"arc",
	"area",
	"areaset",
	"areaspec",
	"arg",
	"article",
	"artpagenums",
	"attribution",
	"audiodata",
	"audioobject",
	"author",
	"authorgroup",
	"authorinitials",
	"bibliocoverage",
	"bibliodiv",
	"biblioentry",
	"bibliography",
	"biblioid",
	"bibliolist",
	"bibliomisc",
	"bibliomixed",
	"bibliomset",
	"biblioref",
	"bibliorelation",
	"biblioset",
	"bibliosource",
	"blockquote",
	"book",
	"bridgehead",
	"callout",
	"calloutlist",
	"caption",
	"caution",
	"chapter",
	"citation",
	"citebiblioid",
	"citerefentry",
	"citetitle",
	"city",
	"classname",
	"classsynopsis",
	"classsynopsisinfo",
	"cmdsynopsis",
	"co",
	"code",
	"col",
	"colgroup",
	"collab",
	"colophon",
	"colspec",
	"command",
	"computeroutput",
	"confdates",
	"confgroup",
	"confnum",
	"confsponsor",
	"conftitle",
	"constant",
	"constraint",
	"constraintdef",
	"constructorsynopsis",
	"contractnum",
	"contractsponsor",
	"contrib",
	"copyright",
	"coref",
	"country",
	"cover",
	"database",
	"date",
	"dedication",
	"destructorsynopsis",
	"edition",
	"editor",
	"email",
	"emphasis",
	"entry",
	"entrytbl",
	"envar",
	"epigraph",
	"equation",
	"errorcode",
	"errorname",
	"errortext",
	"errortype",
	"example",
	"exceptionname",
	"extendedlink",
	"fax",
	"fieldsynopsis",
	"figure",
	"filename",
	"firstname",
	"firstterm",
	"footnote",
	"footnoteref",
	"foreignphrase",
	"formalpara",
	"funcdef",
	"funcparams",
	"funcprototype",
	"funcsynopsis",
	"funcsynopsisinfo",
	"function",
	"glossary",
	"glossdef",
	"glossdiv",
	"glossentry",
	"glosslist",
	"glosssee",
	"glossseealso",
	"glossterm",
	"group",
	"guibutton",
	"guiicon",
	"guilabel",
	"guimenu",
	"guimenuitem",
	"guisubmenu",
	"hardware",
	"holder",
	"honorific",
	"imagedata",
	"imageobject",
	"imageobjectco",
	"important",
	"index",
	"indexdiv",
	"indexentry",
	"indexterm",
	"info",
	"informalequation",
	"informalexample",
	"informalfigure",
	"informaltable",
	"initializer",
	"inlineequation",
	"inlinemediaobject",
	"interfacename",
	"issuenum",
	"itemizedlist",
	"itermset",
	"jobtitle",
	"keycap",
	"keycode",
	"keycombo",
	"keysym",
	"keyword",
	"keywordset",
	"label",
	"legalnotice",
	"lhs",
	"lineage",
	"lineannotation",
	"link",
	"listitem",
	"literal",
	"literallayout",
	"locator",
	"manvolnum",
	"markup",
	"mathphrase",
	"mediaobject",
	"member",
	"menuchoice",
	"methodname",
	"methodparam",
	"methodsynopsis",
	"modifier",
	"mousebutton",
	"msg",
	"msgaud",
	"msgentry",
	"msgexplan",
	"msginfo",
	"msglevel",
	"msgmain",
	"msgorig",
	"msgrel",
	"msgset",
	"msgsub",
	"msgtext",
	"nonterminal",
	"note",
	"olink",
	"ooclass",
	"ooexception",
	"oointerface",
	"option",
	"optional",
	"orderedlist",
	"org",
	"orgdiv",
	"orgname",
	"otheraddr",
	"othercredit",
	"othername",
	"package",
	"pagenums",
	"para",
	"paramdef",
	"parameter",
	"part",
	"partintro",
	"person",
	"personblurb",
	"personname",
	"phone",
	"phrase",
	"pob",
	"postcode",
	"preface",
	"primary",
	"primaryie",
	"printhistory",
	"procedure",
	"production",
	"productionrecap",
	"productionset",
	"productname",
	"productnumber",
	"programlisting",
	"programlistingco",
	"prompt",
	"property",
	"pubdate",
	"publisher",
	"publishername",
	"qandadiv",
	"qandaentry",
	"qandaset",
	"question",
	"quote",
	"refclass",
	"refdescriptor",
	"refentry",
	"refentrytitle",
	"reference",
	"refmeta",
	"refmiscinfo",
	"refname",
	"refnamediv",
	"refpurpose",
	"refsect1",
	"refsect2",
	"refsect3",
	"refsection",
	"refsynopsisdiv",
	"releaseinfo",
	"remark",
	"replaceable",
	"returnvalue",
	"revdescription",
	"revhistory",
	"revision",
	"revnumber",
	"revremark",
	"rhs",
	"row",
	"sbr",
	"screen",
	"screenco",
	"screenshot",
	"secondary",
	"secondaryie",
	"sect1",
	"sect2",
	"sect3",
	"sect4",
	"sect5",
	"section",
	"see",
	"seealso",
	"seealsoie",
	"seeie",
	"seg",
	"seglistitem",
	"segmentedlist",
	"segtitle",
	"seriesvolnums",
	"set",
	"setindex",
	"shortaffil",
	"shortcut",
	"sidebar",
	"simpara",
	"simplelist",
	"simplemsgentry",
	"simplesect",
	"spanspec",
	"state",
	"step",
	"stepalternatives",
	"street",
	"subject",
	"subjectset",
	"subjectterm",
	"subscript",
	"substeps",
	"subtitle",
	"superscript",
	"surname",
	"symbol",
	"synopfragment",
	"synopfragmentref",
	"synopsis",
	"systemitem",
	"table",
	"tag",
	"task",
	"taskprerequisites",
	"taskrelated",
	"tasksummary",
	"tbody",
	"td",
	"term",
	"termdef",
	"tertiary",
	"tertiaryie",
	"textdata",
	"textobject",
	"tfoot",
	"tgroup",
	"th",
	"thead",
	"tip",
	"title",
	"titleabbrev",
	"toc",
	"tocdiv",
	"tocentry",
	"token",
	"tr",
	"trademark",
	"type",
	"uri",
	"userinput",
	"varargs",
	"variablelist",
	"varlistentry",
	"varname",
	"videodata",
	"videoobject",
	"void",
	"volumenum",
	"warning",
	"wordasword",
	"xref",
	"year"
};


docbookBufferTokenizer::docbookBufferTokenizer( char *b, size_t l )
    : buffer(b), length(l), curr(0)
{
}

 
void docbookBufferTokenizer::newline( const char *line, 
				      int first, int last ) {
    textSlice value(&line[first],&line[last]);
    text += value;
}
    

void docbookBufferTokenizer::token( xmlToken token, const char *line, 
			    int first, int last, bool fragment ) {
    /* translate XML start and end tags into docbook tokens */
    const char **key;
    const char **lastKeyword = &docbookKeywords[sizeof(docbookKeywords) / sizeof(char*)];	
    textSlice value(&line[first],&line[last]);

    if( token == xmlContent ) {
	text += value;
    } else {
	text = textSlice();
	switch( token ) {
	case xmlElementStart:
	    elementStart = true;
	    break;
	case xmlElementEnd:
	    elementStart = false;
	    break;
	case xmlEmptyElementEnd:
	    elementStart = false;
	    tokens.push_back((docbookToken)(tokens.back() - 1));
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
		if( strlen(*key) == value.size() 
		    && strncmp(*key,value.begin(),value.size()) == 0 ) {
		    break;
		}
	    }
#endif
	    if( key != lastKeyword ) {
		tokens.push_back((docbookToken)(2 * std::distance(docbookKeywords,
								  key) 
						+ (elementStart ? 1 : 0) + 1));
	    }
	    break;
	}
    }
}
 
  
docbookToken docbookBufferTokenizer::read() {
    tokens.pop_front();
    while( curr < length && tokens.empty() ) {
	curr += tok.tokenize(&buffer[curr],length - curr);
    }
    return current();
}


docbookStreamTokenizer::docbookStreamTokenizer( std::istream& is ) 
  : istr(&is) 
{    
}


void docbookStreamTokenizer::newline( const char *line, 
				      int first, int last ) {
}
    

void docbookStreamTokenizer::token( xmlToken token, const char *line, 
			    int first, int last, bool fragment ) {
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
	text = textSlice();
	elementStart = false;
	break;
    case xmlEmptyElementEnd:
	elementStart = false;
	tokens.push_back((docbookToken)(tokens.back() - 1));
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
	    if( strlen(*key) == value.size() 
		&& strncmp(*key,value.begin(),value.size()) == 0 ) {
		break;
	    }
	}
#endif
	if( key != lastKeyword ) {
	    tokens.push_back((docbookToken)(2 * std::distance(docbookKeywords,
							      key) 
					    + (elementStart ? 1 : 0) + 1));
	}
	break;
    case xmlContent:
	text += value;
	break;
    }
}


docbookToken docbookStreamTokenizer::read() {
    tokens.pop_front();
    while( !istr->eof() && tokens.empty() ) {
	std::string s;
	std::getline(*istr,s);
	tok.tokenize(s.c_str(),s.size());
    }
    return tokens.empty() ? bookEof : *tokens.begin();
}
