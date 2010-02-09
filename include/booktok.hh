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

#ifndef guardbooktok
#define guardbooktok

#include <list>
#include "xmltok.hh"
#include "slice.hh"

/** Definition of tokens for docbook documents. This file was generated
    by the ll(1) parser generator and further modified to fit the required
    API by the docbook to HTML presentation engine.
 */

enum docbookToken {
    bookEof,
	titleStart,
	titleEnd,
	titleabbrevStart,
	titleabbrevEnd,
	subtitleStart,
	subtitleEnd,
	infoStart,
	infoEnd,
	subjectsetStart,
	subjectsetEnd,
	subjectStart,
	subjectEnd,
	subjecttermStart,
	subjecttermEnd,
	keywordsetStart,
	keywordsetEnd,
	keywordStart,
	keywordEnd,
	procedureStart,
	procedureEnd,
	stepStart,
	stepEnd,
	stepalternativesStart,
	stepalternativesEnd,
	substepsStart,
	substepsEnd,
	sidebarStart,
	sidebarEnd,
	abstractStart,
	abstractEnd,
	personblurbStart,
	personblurbEnd,
	blockquoteStart,
	blockquoteEnd,
	attributionStart,
	attributionEnd,
	bridgeheadStart,
	bridgeheadEnd,
	remarkStart,
	remarkEnd,
	epigraphStart,
	epigraphEnd,
	footnoteStart,
	footnoteEnd,
	formalparaStart,
	formalparaEnd,
	paraStart,
	paraEnd,
	simparaStart,
	simparaEnd,
	itemizedlistStart,
	itemizedlistEnd,
	orderedlistStart,
	orderedlistEnd,
	listitemStart,
	listitemEnd,
	segmentedlistStart,
	segmentedlistEnd,
	segtitleStart,
	segtitleEnd,
	seglistitemStart,
	seglistitemEnd,
	segStart,
	segEnd,
	simplelistStart,
	simplelistEnd,
	memberStart,
	memberEnd,
	variablelistStart,
	variablelistEnd,
	varlistentryStart,
	varlistentryEnd,
	termStart,
	termEnd,
	exampleStart,
	exampleEnd,
	informalexampleStart,
	informalexampleEnd,
	literallayoutStart,
	literallayoutEnd,
	screenStart,
	screenEnd,
	screenshotStart,
	screenshotEnd,
	figureStart,
	figureEnd,
	informalfigureStart,
	informalfigureEnd,
	mediaobjectStart,
	mediaobjectEnd,
	inlinemediaobjectStart,
	inlinemediaobjectEnd,
	videoobjectStart,
	videoobjectEnd,
	audioobjectStart,
	audioobjectEnd,
	imageobjectStart,
	imageobjectEnd,
	textobjectStart,
	textobjectEnd,
	videodataStart,
	videodataEnd,
	audiodataStart,
	audiodataEnd,
	imagedataStart,
	imagedataEnd,
	textdataStart,
	textdataEnd,
	captionStart,
	captionEnd,
	addressStart,
	addressEnd,
	streetStart,
	streetEnd,
	pobStart,
	pobEnd,
	postcodeStart,
	postcodeEnd,
	cityStart,
	cityEnd,
	stateStart,
	stateEnd,
	countryStart,
	countryEnd,
	phoneStart,
	phoneEnd,
	faxStart,
	faxEnd,
	otheraddrStart,
	otheraddrEnd,
	affiliationStart,
	affiliationEnd,
	shortaffilStart,
	shortaffilEnd,
	jobtitleStart,
	jobtitleEnd,
	orgnameStart,
	orgnameEnd,
	orgdivStart,
	orgdivEnd,
	artpagenumsStart,
	artpagenumsEnd,
	personnameStart,
	personnameEnd,
	authorStart,
	authorEnd,
	authorgroupStart,
	authorgroupEnd,
	collabStart,
	collabEnd,
	authorinitialsStart,
	authorinitialsEnd,
	personStart,
	personEnd,
	orgStart,
	orgEnd,
	confgroupStart,
	confgroupEnd,
	confdatesStart,
	confdatesEnd,
	conftitleStart,
	conftitleEnd,
	confnumStart,
	confnumEnd,
	confsponsorStart,
	confsponsorEnd,
	contractnumStart,
	contractnumEnd,
	contractsponsorStart,
	contractsponsorEnd,
	copyrightStart,
	copyrightEnd,
	yearStart,
	yearEnd,
	holderStart,
	holderEnd,
	coverStart,
	coverEnd,
	dateStart,
	dateEnd,
	editionStart,
	editionEnd,
	editorStart,
	editorEnd,
	biblioidStart,
	biblioidEnd,
	citebiblioidStart,
	citebiblioidEnd,
	bibliosourceStart,
	bibliosourceEnd,
	bibliorelationStart,
	bibliorelationEnd,
	bibliocoverageStart,
	bibliocoverageEnd,
	legalnoticeStart,
	legalnoticeEnd,
	othercreditStart,
	othercreditEnd,
	pagenumsStart,
	pagenumsEnd,
	contribStart,
	contribEnd,
	honorificStart,
	honorificEnd,
	firstnameStart,
	firstnameEnd,
	surnameStart,
	surnameEnd,
	lineageStart,
	lineageEnd,
	othernameStart,
	othernameEnd,
	printhistoryStart,
	printhistoryEnd,
	pubdateStart,
	pubdateEnd,
	publisherStart,
	publisherEnd,
	publishernameStart,
	publishernameEnd,
	releaseinfoStart,
	releaseinfoEnd,
	revhistoryStart,
	revhistoryEnd,
	revisionStart,
	revisionEnd,
	revnumberStart,
	revnumberEnd,
	revremarkStart,
	revremarkEnd,
	revdescriptionStart,
	revdescriptionEnd,
	seriesvolnumsStart,
	seriesvolnumsEnd,
	volumenumStart,
	volumenumEnd,
	issuenumStart,
	issuenumEnd,
	packageStart,
	packageEnd,
	emailStart,
	emailEnd,
	lineannotationStart,
	lineannotationEnd,
	parameterStart,
	parameterEnd,
	replaceableStart,
	replaceableEnd,
	uriStart,
	uriEnd,
	abbrevStart,
	abbrevEnd,
	acronymStart,
	acronymEnd,
	citationStart,
	citationEnd,
	citerefentryStart,
	citerefentryEnd,
	refentrytitleStart,
	refentrytitleEnd,
	manvolnumStart,
	manvolnumEnd,
	citetitleStart,
	citetitleEnd,
	emphasisStart,
	emphasisEnd,
	foreignphraseStart,
	foreignphraseEnd,
	phraseStart,
	phraseEnd,
	quoteStart,
	quoteEnd,
	subscriptStart,
	subscriptEnd,
	superscriptStart,
	superscriptEnd,
	trademarkStart,
	trademarkEnd,
	wordaswordStart,
	wordaswordEnd,
	footnoterefStart,
	footnoterefEnd,
	xrefStart,
	xrefEnd,
	linkStart,
	linkEnd,
	olinkStart,
	olinkEnd,
	anchorStart,
	anchorEnd,
	altStart,
	altEnd,
	setStart,
	setEnd,
	bookStart,
	bookEnd,
	dedicationStart,
	dedicationEnd,
	acknowledgementsStart,
	acknowledgementsEnd,
	colophonStart,
	colophonEnd,
	appendixStart,
	appendixEnd,
	chapterStart,
	chapterEnd,
	partStart,
	partEnd,
	prefaceStart,
	prefaceEnd,
	partintroStart,
	partintroEnd,
	sectionStart,
	sectionEnd,
	simplesectStart,
	simplesectEnd,
	articleStart,
	articleEnd,
	annotationStart,
	annotationEnd,
	extendedlinkStart,
	extendedlinkEnd,
	locatorStart,
	locatorEnd,
	arcStart,
	arcEnd,
	sect1Start,
	sect1End,
	sect2Start,
	sect2End,
	sect3Start,
	sect3End,
	sect4Start,
	sect4End,
	sect5Start,
	sect5End,
	referenceStart,
	referenceEnd,
	refentryStart,
	refentryEnd,
	refmetaStart,
	refmetaEnd,
	refmiscinfoStart,
	refmiscinfoEnd,
	refnamedivStart,
	refnamedivEnd,
	refdescriptorStart,
	refdescriptorEnd,
	refnameStart,
	refnameEnd,
	refpurposeStart,
	refpurposeEnd,
	refclassStart,
	refclassEnd,
	refsynopsisdivStart,
	refsynopsisdivEnd,
	refsectionStart,
	refsectionEnd,
	refsect1Start,
	refsect1End,
	refsect2Start,
	refsect2End,
	refsect3Start,
	refsect3End,
	glosslistStart,
	glosslistEnd,
	glossentryStart,
	glossentryEnd,
	glossdefStart,
	glossdefEnd,
	glossseeStart,
	glossseeEnd,
	glossseealsoStart,
	glossseealsoEnd,
	firsttermStart,
	firsttermEnd,
	glosstermStart,
	glosstermEnd,
	glossaryStart,
	glossaryEnd,
	glossdivStart,
	glossdivEnd,
	termdefStart,
	termdefEnd,
	biblioentryStart,
	biblioentryEnd,
	bibliomixedStart,
	bibliomixedEnd,
	bibliosetStart,
	bibliosetEnd,
	bibliomsetStart,
	bibliomsetEnd,
	bibliomiscStart,
	bibliomiscEnd,
	bibliographyStart,
	bibliographyEnd,
	bibliodivStart,
	bibliodivEnd,
	bibliolistStart,
	bibliolistEnd,
	bibliorefStart,
	bibliorefEnd,
	itermsetStart,
	itermsetEnd,
	indextermStart,
	indextermEnd,
	primaryStart,
	primaryEnd,
	secondaryStart,
	secondaryEnd,
	tertiaryStart,
	tertiaryEnd,
	seeStart,
	seeEnd,
	seealsoStart,
	seealsoEnd,
	indexStart,
	indexEnd,
	setindexStart,
	setindexEnd,
	indexdivStart,
	indexdivEnd,
	indexentryStart,
	indexentryEnd,
	primaryieStart,
	primaryieEnd,
	secondaryieStart,
	secondaryieEnd,
	tertiaryieStart,
	tertiaryieEnd,
	seeieStart,
	seeieEnd,
	seealsoieStart,
	seealsoieEnd,
	tocStart,
	tocEnd,
	tocdivStart,
	tocdivEnd,
	tocentryStart,
	tocentryEnd,
	taskStart,
	taskEnd,
	tasksummaryStart,
	tasksummaryEnd,
	taskprerequisitesStart,
	taskprerequisitesEnd,
	taskrelatedStart,
	taskrelatedEnd,
	calloutlistStart,
	calloutlistEnd,
	calloutStart,
	calloutEnd,
	programlistingcoStart,
	programlistingcoEnd,
	areaspecStart,
	areaspecEnd,
	areaStart,
	areaEnd,
	areasetStart,
	areasetEnd,
	screencoStart,
	screencoEnd,
	imageobjectcoStart,
	imageobjectcoEnd,
	coStart,
	coEnd,
	corefStart,
	corefEnd,
	productionsetStart,
	productionsetEnd,
	productionStart,
	productionEnd,
	lhsStart,
	lhsEnd,
	rhsStart,
	rhsEnd,
	nonterminalStart,
	nonterminalEnd,
	constraintStart,
	constraintEnd,
	productionrecapStart,
	productionrecapEnd,
	constraintdefStart,
	constraintdefEnd,
	tgroupStart,
	tgroupEnd,
	colspecStart,
	colspecEnd,
	spanspecStart,
	spanspecEnd,
	theadStart,
	theadEnd,
	tfootStart,
	tfootEnd,
	tbodyStart,
	tbodyEnd,
	rowStart,
	rowEnd,
	entryStart,
	entryEnd,
	entrytblStart,
	entrytblEnd,
	tableStart,
	tableEnd,
	informaltableStart,
	informaltableEnd,
	colStart,
	colEnd,
	colgroupStart,
	colgroupEnd,
	trStart,
	trEnd,
	thStart,
	thEnd,
	tdStart,
	tdEnd,
	msgsetStart,
	msgsetEnd,
	msgentryStart,
	msgentryEnd,
	simplemsgentryStart,
	simplemsgentryEnd,
	msgStart,
	msgEnd,
	msgmainStart,
	msgmainEnd,
	msgsubStart,
	msgsubEnd,
	msgrelStart,
	msgrelEnd,
	msgtextStart,
	msgtextEnd,
	msginfoStart,
	msginfoEnd,
	msglevelStart,
	msglevelEnd,
	msgorigStart,
	msgorigEnd,
	msgaudStart,
	msgaudEnd,
	msgexplanStart,
	msgexplanEnd,
	qandasetStart,
	qandasetEnd,
	qandadivStart,
	qandadivEnd,
	qandaentryStart,
	qandaentryEnd,
	questionStart,
	questionEnd,
	answerStart,
	answerEnd,
	labelStart,
	labelEnd,
	equationStart,
	equationEnd,
	informalequationStart,
	informalequationEnd,
	inlineequationStart,
	inlineequationEnd,
	mathphraseStart,
	mathphraseEnd,
	markupStart,
	markupEnd,
	tagStart,
	tagEnd,
	symbolStart,
	symbolEnd,
	tokenStart,
	tokenEnd,
	literalStart,
	literalEnd,
	codeStart,
	codeEnd,
	constantStart,
	constantEnd,
	productnameStart,
	productnameEnd,
	productnumberStart,
	productnumberEnd,
	databaseStart,
	databaseEnd,
	applicationStart,
	applicationEnd,
	hardwareStart,
	hardwareEnd,
	guibuttonStart,
	guibuttonEnd,
	guiiconStart,
	guiiconEnd,
	guilabelStart,
	guilabelEnd,
	guimenuStart,
	guimenuEnd,
	guimenuitemStart,
	guimenuitemEnd,
	guisubmenuStart,
	guisubmenuEnd,
	menuchoiceStart,
	menuchoiceEnd,
	mousebuttonStart,
	mousebuttonEnd,
	keycapStart,
	keycapEnd,
	keycodeStart,
	keycodeEnd,
	keycomboStart,
	keycomboEnd,
	keysymStart,
	keysymEnd,
	accelStart,
	accelEnd,
	shortcutStart,
	shortcutEnd,
	promptStart,
	promptEnd,
	envarStart,
	envarEnd,
	filenameStart,
	filenameEnd,
	commandStart,
	commandEnd,
	computeroutputStart,
	computeroutputEnd,
	userinputStart,
	userinputEnd,
	cmdsynopsisStart,
	cmdsynopsisEnd,
	argStart,
	argEnd,
	groupStart,
	groupEnd,
	sbrStart,
	sbrEnd,
	synopfragmentStart,
	synopfragmentEnd,
	synopfragmentrefStart,
	synopfragmentrefEnd,
	synopsisStart,
	synopsisEnd,
	funcsynopsisStart,
	funcsynopsisEnd,
	funcsynopsisinfoStart,
	funcsynopsisinfoEnd,
	funcprototypeStart,
	funcprototypeEnd,
	funcdefStart,
	funcdefEnd,
	functionStart,
	functionEnd,
	voidStart,
	voidEnd,
	varargsStart,
	varargsEnd,
	paramdefStart,
	paramdefEnd,
	funcparamsStart,
	funcparamsEnd,
	classsynopsisStart,
	classsynopsisEnd,
	classsynopsisinfoStart,
	classsynopsisinfoEnd,
	ooclassStart,
	ooclassEnd,
	oointerfaceStart,
	oointerfaceEnd,
	ooexceptionStart,
	ooexceptionEnd,
	modifierStart,
	modifierEnd,
	interfacenameStart,
	interfacenameEnd,
	exceptionnameStart,
	exceptionnameEnd,
	fieldsynopsisStart,
	fieldsynopsisEnd,
	initializerStart,
	initializerEnd,
	constructorsynopsisStart,
	constructorsynopsisEnd,
	destructorsynopsisStart,
	destructorsynopsisEnd,
	methodsynopsisStart,
	methodsynopsisEnd,
	methodnameStart,
	methodnameEnd,
	methodparamStart,
	methodparamEnd,
	varnameStart,
	varnameEnd,
	returnvalueStart,
	returnvalueEnd,
	typeStart,
	typeEnd,
	classnameStart,
	classnameEnd,
	programlistingStart,
	programlistingEnd,
	cautionStart,
	cautionEnd,
	importantStart,
	importantEnd,
	noteStart,
	noteEnd,
	tipStart,
	tipEnd,
	warningStart,
	warningEnd,
	errorcodeStart,
	errorcodeEnd,
	errornameStart,
	errornameEnd,
	errortextStart,
	errortextEnd,
	errortypeStart,
	errortypeEnd,
	systemitemStart,
	systemitemEnd,
	optionStart,
	optionEnd,
	optionalStart,
	optionalEnd,
	propertyStart,
	propertyEnd
};

extern const char* docbookKeywords[];


/** *docbookScanner* implements the lexical scanner used
    by the LL(1) parser used to drive the transformation
    of documents with docbook markups into HTML.
*/
class docbookScanner : public xmlTokListener {
public:
    typedef slice<const char> textSlice;

protected:
    std::istream *istr;
    std::list<docbookToken> tokens;
    xmlTokenizer tok;
    textSlice text;

 public:

    explicit docbookScanner( std::istream& is );

    void newline();
    
    void token( xmlToken token, const char *line, 
		int first, int last, bool fragment );

    docbookToken read();

    docbookToken current() const { return *tokens.begin(); }
};

#endif
