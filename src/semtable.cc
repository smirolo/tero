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

/** dispatch table specific for semilla/semcache.

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

#include "feeds.hh"
#include "changelist.hh"
#include "composer.hh"
#include "docbook.hh"
#include "project.hh"
#include "logview.hh"
#include "checkstyle.hh"
#include "calendar.hh"
#include "comments.hh"
#include "contrib.hh"
#include "todo.hh"
#include "blog.hh"
#include "webserve.hh"
#include "payment.hh"
#include "cppfiles.hh"
#include "shfiles.hh"
#include "auth.hh"

char none[] = "";
char todoExt[] = "todo";
char corpExt[] = "corp";
char rssExt[] = "rss";
char blogExt[] = "blog";
char blogIndex[] = "blogindex";
char project[] = "project";
char docPage[] = "document";
char todos[] = "todos";
char blogPat[] = ".*\\.blog$";
char indexPage[] = "index";
char author[] = "author";
char feed[] = "feed";
char source[] = "source";
char date[] = "date";
char title[] = "title";
char buildView[] = "Build View";

std::string active("todos/active");


/* The pattern need to be inserted in more specific to more 
   generic order since the matcher will apply each the first
   one that yields a positive match. */
fetchEntry entries[] = {
    { "author", boost::regex(".*\\.blog"), always, textMeta<author> },
    
    { "check", boost::regex(".*\\.c"), whenFileExist, checkfileFetch<cppChecker> },
    { "check", boost::regex(".*\\.h"), whenFileExist, checkfileFetch<cppChecker> },
    { "check", boost::regex(".*\\.cc"), whenFileExist, checkfileFetch<cppChecker> },

    { "check", boost::regex(".*\\.hh"), whenFileExist, checkfileFetch<cppChecker> },
    { "check", boost::regex(".*\\.tcc"), whenFileExist, checkfileFetch<cppChecker> },
    { "check", boost::regex(".*\\.mk"), whenFileExist, checkfileFetch<shChecker> },
    { "check", boost::regex(".*\\.py"), whenFileExist, checkfileFetch<shChecker> },
    { "check", boost::regex(".*Makefile"), whenFileExist, checkfileFetch<shChecker> },

    /* Widget to display status of static analysis of a project 
       source files in the absence of a more restrictive pattern. */
    { "checkstyle", boost::regex(".*"), always, checkstyleFetch },

    { "date", boost::regex(".*\\.blog"), always, textMeta<date> },
    { "dates", boost::regex(".*/blog/.*"), whenNotCached, blogDateLinks<blogPat> },

    /* The build "document" gives an overview of the set 
       of all projects at a glance and can be used to assess 
       the stability of the whole as a release candidate. */
    { "document", boost::regex(".*/log/"), whenNotCached, logviewFetch },

    /* need this for tero.template */
    { "document", boost::regex(".*\\.template"), whenFileExist, formattedFetch },

    { "document", boost::regex(".*\\.c"), whenFileExist, cppFetch },
    { "document", boost::regex(".*\\.h"), whenFileExist, cppFetch },
    { "document", boost::regex(".*\\.cc"), whenFileExist, cppFetch },
    { "document", boost::regex(".*\\.hh"), whenFileExist, cppFetch },
    { "document", boost::regex(".*\\.tcc"), whenFileExist, cppFetch },

    { "document", boost::regex(".*\\.c/diff"), always, cppDiff },
    { "document", boost::regex(".*\\.h/diff"), always, cppDiff },
    { "document", boost::regex(".*\\.cc/diff"), always, cppDiff },
    { "document", boost::regex(".*\\.hh/diff"), always, cppDiff },
    { "document", boost::regex(".*\\.tcc/diff"), always, cppDiff },

    { "document", boost::regex(".*\\.mk"), whenFileExist, shFetch },
    { "document", boost::regex(".*\\.py"), whenFileExist, shFetch },
    { "document", boost::regex(".*Makefile"), whenFileExist, shFetch },

    { "document", boost::regex(".*\\.mk/diff"), always, shDiff },
    { "document", boost::regex(".*\\.py/diff"), always, shDiff },
    { "document", boost::regex(".*Makefile/diff"), always, shDiff },


    { "document", boost::regex(".*dws\\.xml"),always, projindexFetch },

    /* Widget to generate a rss feed. Attention: it needs 
       to be declared before any of the todoFilter::viewPat 
       (i.e. todos/.+) since an rss feed exists for todo items
       as well. */	    
    { "document", boost::regex("/index\\.rss"), always, rssSiteAggregate<docPage> },
    { "document", boost::regex(".*\\.git/index\\.rss"), always,
      feedRepository<rsswriter> },
    { "document", boost::regex(".*/index\\.rss"), always,
      feedLatestPosts<rsswriter,docPage> },

    { "document", boost::regex(std::string(".*") + active), always,
      todoIndexWriteHtmlFetch },

    { "document", boost::regex(".*\\.todo/comment"), always, todoCommentFetch },
    { "document", boost::regex(".*\\.todo/voteAbandon"), always, todoVoteAbandonFetch },
    { "document", boost::regex(".*\\.todo/voteSuccess"), always, todoVoteSuccessFetch },

    { "document", boost::regex(".*\\.blog"), whenNotCached, blogEntryFetch },
    { "document", boost::regex(".*/blog/tags-.*"), 
      whenNotCached, blogByIntervalTags<docPage,blogPat> },
    { "document", boost::regex(".*/blog/archive-.*"), 
      whenNotCached, blogByIntervalDate<docPage,blogPat> },
    { "document", boost::regex(".*/blog/"), whenNotCached, mostRecentBlogFetch},

    /* contributors, accounts authentication */
	{ "document", boost::regex(".*/accounts/login/"), always, loginFetch },
	{ "document", boost::regex(".*/accounts/logout/"), always, logoutFetch },
	{ "document", boost::regex(".*/accounts/password_change/"), always, passwdChange },
	{ "document", boost::regex(".*/accounts/password_reset/"), always, passwdReset },
    { "document", boost::regex(".*/accounts/register/complete/"), always, registerConfirm },
	{ "document", boost::regex(".*/accounts/register/"), always, registerEnter },
	{ "document", boost::regex(".*/accounts/unregister/"), always, unregisterEnter },
    { "document", boost::regex(".*accounts/"), always, contribIdxFetch },
        
	/* misc pages */
    { "document", boost::regex(".*\\.commit"), always, changeShowDetails },
    { "document", boost::regex(".*\\.eml"), always, mailParserFetch },
    { "document", boost::regex(".*\\.ics"), whenFileExist, calendarFetch },
    { "document", boost::regex(".*\\.todo"), whenFileExist, todoWriteHtmlFetch },

    /* We transform docbook formatted text into HTML for .book 
       and .corp "document" files and interpret all other unknown 
       extension files as raw text. In all cases we use a default
       document.template interface "view" to present those files. */ 
    { "document", boost::regex(".*\\.book"), whenFileExist, docbookFetch },
    { "document", boost::regex(".*\\.corp"), whenFileExist, docbookFetch },

    { "document", boost::regex(".*"), whenFileExist, textFetch },

    /* homepage */
    { "feed", boost::regex(".*\\.git/index\\.feed"), always, feedRepository<htmlwriter> },
    { "feed", boost::regex(".*/index\\.feed"), always, feedLatestPosts<htmlwriter,feed> },
    { "feed", boost::regex("/"), always, htmlSiteAggregate<feed> },

    { "history", boost::regex(".*dws\\.xml"), always, feedRepository<htmlwriter> },
    /* Widget to display the history of a file under revision control
       in the absence of a more restrictive pattern. */	   
    { "history", boost::regex(".*"), always, changehistoryFetch },

    /* just print the value of *name* */
    { "print", boost::regex(".*"), notAFile, metaValue },

    /* Widget to display a list of files which are part of a project.
       This widget is used through different "view"s to browse 
       the source repository. */
    { "projfiles", boost::regex(".*"), always, projfilesFetch },

    /* A project dws.xml "document" file show a description,
       commits and unit test status of a single project through 
       a project "view". */
    { "regressions", boost::regex(".*dws\\.xml"), whenFileExist, regressionsFetch },

    { "relates", boost::regex(".*/blog/.*"), whenNotCached, blogRelatedSubjects<blogPat> },
    { "tags", boost::regex(".*/blog/.*"), whenNotCached, blogTagLinks<blogPat> },

    /* use always instead of whenFileExist here because the composer
       is looking for template files into a themeDir and not the local
       directory. 
       This is special, we cannot label them as 'document' nor 'view'
       otherwise it creates infinite loops on feed fetches. */
    { "template", boost::regex(".*\\.template"), always, compose<none> },

    /* Load title from the meta tags in a text file. */
    { "title", boost::regex(".*/log/"),   always, consMeta<buildView> },
    { "title", boost::regex(".*\\.blog"), whenFileExist, textMeta<title> },
    { "title", boost::regex(".*/blog/tags-.*"), 
      whenNotCached, blogByIntervalTitle },
    { "title", boost::regex(".*/blog/"), whenNotCached, mostRecentBlogTitle },
    { "title", boost::regex(".*\\.book"), whenFileExist, docbookMeta },
    { "title", boost::regex(".*\\.corp"), whenFileExist, docbookMeta },
    { "title", boost::regex(".*\\.todo"), whenFileExist, todoMeta },
    { "title", boost::regex(".*\\.template"), whenFileExist, textMeta<title> },
    { "title", boost::regex(".*dws\\.xml"), whenFileExist, projectTitle },
    { "title", boost::regex(".*"), always, metaFetch<title> },
   
    /* If a template file matching the document's extension
       is present in the theme directory, let's use it
       as a composer. */
    { "view", boost::regex(".*\\.todo"), whenNotCached, compose<todoExt> },
#if 0
    { "view", boost::regex(".*\\.todo/comment"), always, todoCommentFetch },
#endif
	/* \todo we avoid to generate caches on the header menus for now. */
    { "view", boost::regex(".*\\.corp"), always, compose<corpExt> },
    { "view", boost::regex(".*\\.rss"), whenNotCached, compose<rssExt> },
    { "view", boost::regex(".*\\.blog"), whenFileExist, compose<blogExt> },

    { "view", boost::regex(".*\\.c"), whenFileExist, compose<source> },
    { "view", boost::regex(".*\\.h"), whenFileExist, compose<source> },
    { "view", boost::regex(".*\\.cc"), whenFileExist, compose<source> },
    { "view", boost::regex(".*\\.hh"), whenFileExist, compose<source> },
    { "view", boost::regex(".*\\.tcc"), whenFileExist, compose<source> },

    { "view", boost::regex(".*\\.mk"), whenFileExist, compose<source> },
    { "view", boost::regex(".*\\.py"), whenFileExist, compose<source> },
    { "view", boost::regex(".*Makefile"), whenFileExist, compose<source> },

    /* Command to create a new project */
    { "view", boost::regex(".*/reps/.*/create"), always, projCreateFetch },

    /* Composer for a project view */
    { "view", boost::regex(".*dws\\.xml"), always, compose<project> },

    /* Composer and document for the todos index view */
    { "view", boost::regex(".*todos/.+"), always, compose<todos> },

	/* We must do this through a "view" and not a "document" because 
	   the feedback is different if the application is invoked from
	   the command line or through the cgi interface. */
    { "view", boost::regex(".*/todoCreate"), always, todoCreateFetch },

    /* comments */
    { "view", boost::regex(std::string("/comments/create")), always, commentPage },

    /* blog presentation */ 
    { "view", boost::regex(".*/blog/"), whenNotCached, compose<blogIndex> },
    { "view", boost::regex(".*/blog/.*"), whenNotCached, compose<blogExt> },
    
    /* Source code "document" files are syntax-highlighted 
       and presented inside a source.template "view" */    
    { "view", boost::regex(".*/diff"), always, compose<source> },

    { "view", boost::regex("/"), always, compose<indexPage> },

    /* default catch-all:
       We use "always" here such that semcache will not generate .html
       versions of style.css, etc. */
    { "view",boost::regex(".*"), always, compose<docPage> },

#if 0
    /* button to Amazon payment */    
    { "payproc", boost::regex(".*"), always, paymentFetch },
#endif    
};

dispatchDoc semDocs(entries,sizeof(entries)/sizeof(entries[0]));
