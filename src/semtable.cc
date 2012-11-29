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
char bookExt[] = "book";
char corpExt[] = "corp";
char rssExt[] = "rss";
char blogExt[] = "blog";
char blogIndex[] = "blogindex";
char project[] = "project";
char docPage[] = "document";
char todos[] = "todos";
char indexPage[] = "index";
char author[] = "author";
char feed[] = "feed";
char source[] = "source";
char date[] = "date";
char title[] = "title";
char buildView[] = "Build View";


/* The pattern need to be inserted in more specific to more
   generic order since the matcher will apply each the first
   one that yields a positive match. */
fetchEntry entries[] = {
    { "author", boost::regex(".*\\.blog"), 
	  noAuth|noPipe, NULL,  textMeta<author>, NULL },
    { "bytags", boost::regex(".*/blog/.*"),
	  noAuth|noPipe, blogTagLinks<blogPat>, NULL, NULL },
    { "check", boost::regex(".*\\.c"),
	  noAuth|noPipe, checkfileFetch<cppChecker>, NULL, NULL },
    { "check", boost::regex(".*\\.h"),
	  noAuth|noPipe, checkfileFetch<cppChecker>, NULL, NULL },
    { "check", boost::regex(".*\\.cc"),
	  noAuth|noPipe, checkfileFetch<cppChecker>, NULL, NULL },
    { "check", boost::regex(".*\\.hh"),
	  noAuth|noPipe, checkfileFetch<cppChecker>, NULL, NULL },
    { "check", boost::regex(".*\\.tcc"),
	  noAuth|noPipe, checkfileFetch<cppChecker>, NULL, NULL },
    { "check", boost::regex(".*\\.mk"), 
	  noAuth|noPipe, checkfileFetch<shChecker>, NULL, NULL },
    { "check", boost::regex(".*\\.py"),
	  noAuth|noPipe, checkfileFetch<shChecker>, NULL, NULL },
    { "check", boost::regex(".*Makefile"),
	  noAuth|noPipe, checkfileFetch<shChecker>, NULL, NULL },
    /* Widget to display status of static analysis of a project
       source files in the absence of a more restrictive pattern. */
    { "checkstyle", boost::regex(".*"),
	  noAuth|noPipe, checkstyleFetch, NULL, NULL },
	// 1. XXX
    { "date", boost::regex(".*\\.blog"),
	  noAuth|noPipe, NULL, textMeta<date>, NULL },
    { "dates", boost::regex(".*/blog/.*"),
	  noAuth|noPipe, blogDateLinks<blogPat>, NULL, NULL },

    /* The build "document" gives an overview of the set
       of all projects at a glance and can be used to assess
       the stability of the whole as a release candidate. */
    { "document", boost::regex(".*/log/"),
	  noAuth|noPipe, logviewFetch, NULL, NULL },

    /* need this for tero.template */
    { "document", boost::regex(".*\\.template"),
	  noAuth|noPipe, NULL, formattedFetch, NULL },
    { "document", boost::regex(".*\\.c"),
	  noAuth|noPipe, NULL, cppFetch, NULL },
    { "document", boost::regex(".*\\.h"),
	  noAuth|noPipe, NULL, cppFetch, NULL },
    { "document", boost::regex(".*\\.cc"),
	  noAuth|noPipe, NULL, cppFetch, NULL },
    { "document", boost::regex(".*\\.hh"),
	  noAuth|noPipe, NULL, cppFetch, NULL },
    { "document", boost::regex(".*\\.tcc"), 
	  noAuth|noPipe, NULL, cppFetch, NULL },
    { "document", boost::regex(".*\\.c/diff/[0-9a-f]{40}"),
	  noAuth|noPipe, cppDiff, NULL, NULL },
    { "document", boost::regex(".*\\.h/diff/[0-9a-f]{40}"), 
	  noAuth|noPipe, cppDiff, NULL, NULL },
    { "document", boost::regex(".*\\.cc/diff/[0-9a-f]{40}"),
	  noAuth|noPipe, cppDiff, NULL, NULL },
    { "document", boost::regex(".*\\.hh/diff/[0-9a-f]{40}"),
	  noAuth|noPipe, cppDiff, NULL, NULL },
    { "document", boost::regex(".*\\.tcc/diff/[0-9a-f]{40}"),
	  noAuth|noPipe, cppDiff, NULL, NULL },
    { "document", boost::regex(".*\\.mk"),
	  noAuth|noPipe, NULL, shFetch, NULL },
    { "document", boost::regex(".*\\.py"),
	  noAuth|noPipe, NULL, shFetch, NULL },
    { "document", boost::regex(".*Makefile"),
	  noAuth|noPipe, NULL, shFetch, NULL },
    { "document", boost::regex(".*\\.mk/diff/[0-9a-f]{40}"), 
	  noAuth|noPipe, shDiff, NULL, NULL },
    { "document", boost::regex(".*\\.py/diff/[0-9a-f]{40}"), 
	  noAuth|noPipe, shDiff, NULL, NULL },
    { "document", boost::regex(".*Makefile/diff/[0-9a-f]{40}"),
	  noAuth|noPipe, shDiff, NULL, NULL },
    { "document", boost::regex(".*dws\\.xml"),
	  noAuth|noPipe, projindexFetch, NULL, NULL },
    /* Widget to generate a rss feed. Attention: it needs
       to be declared before any of the todoFilter::viewPat
       (i.e. todos/.+) since an rss feed exists for todo items
       as well. */
    { "document", boost::regex("/index\\.rss"),
	  noAuth|noPipe, rssSiteAggregate<docPage>, NULL, NULL },
    { "document", boost::regex(".*\\.git/index\\.rss"),
	  noAuth|noPipe, feedRepository<rsswriter>, NULL, NULL },
    { "document", boost::regex(".*/index\\.rss"),
	  noAuth|noPipe, feedLatestPosts<rsswriter,docPage>, NULL, NULL },
    { "document", boost::regex(".*/todo/"),
	  noAuth|noPipe, todoIndexWriteHtmlFetch, NULL, NULL },
    { "document", boost::regex(".*\\.todo/comment"),
	  noAuth|noPipe, todoCommentFetch, NULL, NULL },
    { "document", boost::regex(".*\\.todo/voteAbandon"),
	  noAuth|noPipe, todoVoteAbandonFetch, NULL, NULL },
    { "document", boost::regex(".*\\.todo/voteSuccess"),
	  noAuth|noPipe, todoVoteSuccessFetch, NULL, NULL },
    { "document", boost::regex(".*\\.blog"),
	  noAuth|noPipe, blogEntryFetch, NULL, NULL },
    { "document", boost::regex(".*/blog/tags-.*"),
      noAuth|noPipe, blogByIntervalTags<docPage,blogPat>, NULL, NULL },
    { "document", boost::regex(".*/blog/tags/"),
	  noAuth|noPipe, blogTagLinks<blogPat>, NULL, NULL },

    { "document", boost::regex(".*/blog/archive-.*"),
      noAuth|noPipe, blogByIntervalDate<docPage,blogPat>, NULL, NULL },
    { "document", boost::regex(".*/blog/"),
	  noAuth|noPipe, mostRecentBlogFetch, NULL, NULL },

    /* contributors, accounts authentication */
    { "document", boost::regex(".*/accounts/login/"),
	  noAuth|noPipe, loginFetch, NULL, NULL },
    { "document", boost::regex(".*/accounts/logout/"),
	  noAuth|noPipe, logoutFetch, NULL, NULL },
    { "document", boost::regex(".*/accounts/password_change/"),
	  noAuth|noPipe, passwdChange, NULL, NULL },
    { "document", boost::regex(".*/accounts/password_reset/"),
	  noAuth|noPipe, passwdReset, NULL, NULL },
    { "document", boost::regex(".*/accounts/register/complete/"),
	  noAuth|noPipe, registerConfirm, NULL, NULL },
    { "document", boost::regex(".*/accounts/register/"),
	  noAuth|noPipe, registerEnter, NULL, NULL },
    { "document", boost::regex(".*/accounts/unregister/"),
	  noAuth|noPipe, unregisterEnter, NULL, NULL },
    { "document", boost::regex(".*accounts/"),
	  noAuth|noPipe, contribIdxFetch, NULL, NULL },
    /* misc pages */
    { "document", boost::regex(".*\\.commit"),
	  noAuth|noPipe, changeShowDetails, NULL, NULL },
    { "document", boost::regex(".*\\.eml"),
	  noAuth|noPipe, mailParserFetch, NULL, NULL },
    { "document", boost::regex(".*\\.ics"),
	  noAuth|noPipe, NULL, calendarFetch,  NULL },
    { "document", boost::regex(".*\\.todo"),
	  noAuth|noPipe, NULL, todoWriteHtmlFetch, NULL },

    /* We transform docbook formatted text into HTML for .book
       and .corp "document" files and interpret all other unknown
       extension files as raw text. In all cases we use a default
       document.template interface "view" to present those files. */
    { "document", boost::regex(".*\\.book"),
	  noAuth|noPipe, docbookFetch, NULL, NULL },
    { "document", boost::regex(".*\\.corp"),
	  noAuth|noPipe, docbookFetch, NULL, NULL },

    { "document", boost::regex(".*"),
	  noAuth|noPipe, NULL, textFetch, NULL },
    /* homepage */
    { "feed", boost::regex(".*\\.git/index\\.feed"),
	  noAuth|noPipe, feedRepository<htmlwriter>, NULL, NULL },
    { "feed", boost::regex(".*/index\\.feed"),
	  noAuth|noPipe, feedLatestPosts<htmlwriter,feed>, NULL, NULL },
    { "feed", boost::regex("/"),
	  noAuth|noPipe, htmlSiteAggregate<feed>, NULL, NULL },
    { "history", boost::regex(".*dws\\.xml"),
	  noAuth|noPipe, feedRepository<htmlwriter>, NULL, NULL },
    /* Widget to display the history of a file under revision control
       in the absence of a more restrictive pattern. */
    { "history", boost::regex(".*"),
	  noAuth|noPipe, changehistoryFetch, NULL, NULL },

    /* just print the value of *name* */
    { "print", boost::regex(".*"),
	  noAuth|noPipe, metaValue, NULL, NULL },

    /* Widget to display a list of files which are part of a project.
       This widget is used through different "view"s to browse
       the source repository. */
    { "projfiles", boost::regex(".*"),
	  noAuth|noPipe, projfilesFetch, NULL, NULL },
    /* A project dws.xml "document" file show a description,
       commits and unit test status of a single project through
       a project "view". */
    { "regressions", boost::regex(".*dws\\.xml"),
	  noAuth|noPipe, regressionsFetch, NULL, NULL },

    { "relates", boost::regex(".*/blog/.*"),
	  noAuth|noPipe, blogRelatedSubjects<blogPat>, NULL, NULL },

    /* use always instead of whenFileExist here because the composer
       is looking for template files into a themeDir and not the local
       directory.
       This is special, we cannot label them as 'document' nor 'view'
       otherwise it creates infinite loops on feed fetches. */
    { "template", boost::regex(".*\\.template"),
	  noAuth|noPipe, compose<none>, NULL, NULL },

    /* Load title from the meta tags in a text file. */
    { "title", boost::regex(".*/log/"),
	  noAuth|noPipe, consMeta<buildView>, NULL, NULL },
    { "title", boost::regex(".*\\.blog"),
	  noAuth|noPipe, NULL, textMeta<title>, NULL },
    { "title", boost::regex(".*/blog/tags-.*"),
	  noAuth|noPipe, blogByIntervalTitle, NULL, NULL },
    { "title", boost::regex(".*/blog/"),
	  noAuth|noPipe, mostRecentBlogTitle, NULL, NULL },
    { "title", boost::regex(".*\\.book"),
	  noAuth|noPipe, docbookMeta, NULL, NULL },
    { "title", boost::regex(".*\\.corp"),
	  noAuth|noPipe, docbookMeta, NULL, NULL },
    { "title", boost::regex(".*\\.todo"),
	  noAuth|noPipe, NULL, todoMeta, NULL },
    { "title", boost::regex(".*\\.template"),
	  noAuth|noPipe, NULL, textMeta<title>, NULL },
    { "title", boost::regex(".*dws\\.xml"),
	  noAuth|noPipe, projectTitle, NULL, NULL },
    { "title", boost::regex(".*"),
	  noAuth|noPipe, metaFetch<title>, NULL, NULL },
    /* If a template file matching the document's extension
       is present in the theme directory, let's use it
       as a composer. */
    { "view", boost::regex(".*\\.todo"),
	  noAuth|noPipe, compose<todoExt>, NULL, NULL },
#if 0
    { "view", boost::regex(".*\\.todo/comment"), always, todoCommentFetch },
#endif
    /* \todo we avoid to generate caches on the header menus for now. */
    { "view", boost::regex(".*\\.corp"),
	  noAuth|noPipe, compose<corpExt>, NULL, NULL },
    { "view", boost::regex(".*\\.rss"),
	  noAuth|noPipe, compose<rssExt>, NULL, NULL },
    { "view", boost::regex(".*\\.blog"),
	  noAuth|noPipe, compose<blogExt>, NULL, NULL },
    { "view", boost::regex(".*\\.book"),
	  noAuth|noPipe, compose<bookExt>, NULL, NULL },
	// XXX
    { "view", boost::regex(".*\\.c"),
	  noAuth|noPipe, compose<source>, NULL, NULL },
    { "view", boost::regex(".*\\.h"),
	  noAuth|noPipe, compose<source>, NULL, NULL },
    { "view", boost::regex(".*\\.cc"),
	  noAuth|noPipe, compose<source>, NULL, NULL },
    { "view", boost::regex(".*\\.hh"),
	  noAuth|noPipe, compose<source>, NULL, NULL },
    { "view", boost::regex(".*\\.tcc"),
	  noAuth|noPipe, compose<source>, NULL, NULL },

    { "view", boost::regex(".*\\.mk"),
	  noAuth|noPipe, compose<source>, NULL, NULL },
    { "view", boost::regex(".*\\.py"),
	  noAuth|noPipe, compose<source>, NULL, NULL },
    { "view", boost::regex(".*Makefile"),
	  noAuth|noPipe, compose<source>, NULL, NULL },
    /* Composer for a project view */
    { "view", boost::regex(".*dws\\.xml"),
	  noAuth|noPipe, compose<project>, NULL, NULL },

    /* We must do this through a "view" and not a "document" because
       the feedback is different if the application is invoked from
       the command line or through the cgi interface. */
    { "view", boost::regex(".*/todo/create"),
	  noAuth|noPipe, todoCreateFetch, NULL, NULL },

    /* Composer and document for the todos index view */
    { "view", boost::regex(".*/todo/"),
	  noAuth|noPipe, compose<todos>, NULL, NULL },

    /* Command to create a new project */
    { "view", boost::regex(".*/reps/.*/create"),
	  noAuth|noPipe, projCreateFetch, NULL, NULL },

    /* comments */
    { "view", boost::regex(std::string("/comments/create")),
	  noAuth|noPipe, commentPage, NULL, NULL },

    /* blog presentation */
    { "view", boost::regex(".*/blog/"),
	  noAuth|noPipe, compose<blogIndex>, NULL, NULL },
    { "view", boost::regex(".*/blog/.*"),
	  noAuth|noPipe, compose<blogExt>, NULL, NULL },

    /* Source code "document" files are syntax-highlighted
       and presented inside a source.template "view". */
    { "view", boost::regex(".*/diff/[0-9a-f]{40}"),
	  noAuth|noPipe, compose<source>, NULL, NULL },

    { "view", boost::regex("/"),
	  noAuth|noPipe, compose<indexPage>, NULL, NULL },

    /* default catch-all:
       We use "always" here such that semcache will not generate .html
       versions of style.css, etc. */
    { "view",boost::regex(".*"),
	  noAuth|noPipe, compose<docPage>, NULL, NULL },
};

dispatchDoc semDocs(entries,sizeof(entries)/sizeof(entries[0]));
