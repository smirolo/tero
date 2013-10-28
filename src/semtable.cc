/* Copyright (c) 2009-2013, Fortylines LLC
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
char todoExt[] = "todo.html";
char bookExt[] = "book.html";
char rssExt[] = "index.rss";
char blogExt[] = "blog.html";
char project[] = "project.html";
char topMenu[] = "topMenu.html";
char docLayout[] = "document.html";
char basePage[] = "base.html";
char docPage[] = "document";
char todos[] = "todos.html";
char indexPage[] = "index.html";
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

    { "base", boost::regex(".*\\.rss"),
	  noAuth|noPipe, compose<rssExt>, NULL, NULL },
    /* We must do this through a "base" and not a "document" because
       the feedback is different if the application is invoked from
       the command line or through the cgi interface. */
    { "base", boost::regex(".*/todo/create"),
	  noAuth|noPipe, todoCreateFetch, NULL, NULL },
    /* comments */
    { "base", boost::regex(std::string("/comments/create")),
	  noAuth|noPipe, commentPage, NULL, NULL },

    /* default catch-all:
       We use "always" here such that semcache will not generate .html
       versions of style.css, etc. */
    { "base",boost::regex(".*"),
	  noAuth|noPipe, compose<basePage>, NULL, NULL },

    { "bytags", boost::regex(".*/blog/.*"),
	  noAuth|noPipe, blogTagLinks<blogPat>, NULL, NULL },
    { "check", boost::regex(".*\\.c"),
	  noAuth|noPipe, NULL, NULL, checkfileFetch<cppChecker> },
    { "check", boost::regex(".*\\.h"),
	  noAuth|noPipe,  NULL, NULL, checkfileFetch<cppChecker> },
    { "check", boost::regex(".*\\.cc"),
	  noAuth|noPipe, NULL, NULL, checkfileFetch<cppChecker> },
    { "check", boost::regex(".*\\.hh"),
	  noAuth|noPipe, NULL, NULL, checkfileFetch<cppChecker> },
    { "check", boost::regex(".*\\.tcc"),
	  noAuth|noPipe, NULL, NULL, checkfileFetch<cppChecker> },
    { "check", boost::regex(".*\\.mk"),
	  noAuth|noPipe, NULL, NULL, checkfileFetch<shChecker> },
    { "check", boost::regex(".*\\.py"),
	  noAuth|noPipe, NULL, NULL, checkfileFetch<shChecker> },
    { "check", boost::regex(".*Makefile"),
	  noAuth|noPipe, NULL, NULL, checkfileFetch<shChecker> },
    /* Widget to display status of static analysis of a project
       source files in the absence of a more restrictive pattern. */
    { "checkstyle", boost::regex(".*"),
	  noAuth|noPipe, checkstyleFetch, NULL, NULL },

    /* The build "document" gives an overview of the set
       of all projects at a glance and can be used to assess
       the stability of the whole as a release candidate. */
    { "content", boost::regex(".*/log/"),
	  noAuth|noPipe, logviewFetch, NULL, NULL },
    { "content", boost::regex(".*\\.(cc|hh|tcc)$"),
	  noAuth|noPipe, NULL, cppFetch, NULL },
    { "content", boost::regex(".*\\.(c|h)$"),
	  noAuth|noPipe, NULL, cppFetch, NULL },
    { "content", boost::regex(".*\\.(cc|hh|tcc)/diff/[0-9a-f]{40}"),
	  noAuth|noPipe, cppDiff, NULL, NULL },
    { "content", boost::regex(".*\\.(c|h)/diff/[0-9a-f]{40}"),
	  noAuth|noPipe, cppDiff, NULL, NULL },
    { "content", boost::regex(".*\\.mk"),
	  noAuth|noPipe, NULL, shFetch, NULL },
    { "content", boost::regex(".*\\.py"),
	  noAuth|noPipe, NULL, shFetch, NULL },
    { "content", boost::regex(".*Makefile"),
	  noAuth|noPipe, NULL, shFetch, NULL },
    { "content", boost::regex(".*\\.mk/diff/[0-9a-f]{40}"),
	  noAuth|noPipe, shDiff, NULL, NULL },
    { "content", boost::regex(".*\\.py/diff/[0-9a-f]{40}"),
	  noAuth|noPipe, shDiff, NULL, NULL },
    { "content", boost::regex(".*Makefile/diff/[0-9a-f]{40}"),
	  noAuth|noPipe, shDiff, NULL, NULL },
    { "content", boost::regex(".*\\.todo/comment"),
	  noAuth|noPipe, todoCommentFetch, NULL, NULL },
    { "content", boost::regex(".*\\.todo/voteAbandon"),
	  noAuth|noPipe, todoVoteAbandonFetch, NULL, NULL },
    { "content", boost::regex(".*\\.todo/voteSuccess"),
	  noAuth|noPipe, todoVoteSuccessFetch, NULL, NULL },
    { "content", boost::regex(".*\\.book"),
      noAuth|noPipe, docbookFetch, NULL, NULL },
    { "content", boost::regex(".*\\.blog"),
      noAuth|noPipe, blogEntryFetch, NULL, NULL },
    { "content", boost::regex(".*/blog/tags-.*"),
      noAuth|noPipe, blogByIntervalTags<docPage,blogPat>, NULL, NULL },
    { "content", boost::regex(".*/blog/tags/"),
	  noAuth|noPipe, blogTagLinks<blogPat>, NULL, NULL },

    { "content", boost::regex(".*/blog/archive-.*"),
      noAuth|noPipe, blogByIntervalDate<docPage,blogPat>, NULL, NULL },
    { "content", boost::regex(".*/blog/"),
	  noAuth|noPipe, mostRecentBlogFetch, NULL, NULL },

    /* contributors, accounts authentication */
    { "content", boost::regex(".*/accounts/login/"),
	  noAuth|noPipe, loginFetch, NULL, NULL },
    { "content", boost::regex(".*/accounts/logout/"),
	  noAuth|noPipe, logoutFetch, NULL, NULL },
    { "content", boost::regex(".*/accounts/password_change/"),
	  noAuth|noPipe, passwdChange, NULL, NULL },
    { "content", boost::regex(".*/accounts/password_reset/"),
	  noAuth|noPipe, passwdReset, NULL, NULL },
    { "content", boost::regex(".*/accounts/register/complete/"),
	  noAuth|noPipe, registerConfirm, NULL, NULL },
    { "content", boost::regex(".*/accounts/register/"),
	  noAuth|noPipe, registerEnter, NULL, NULL },
    { "content", boost::regex(".*/accounts/unregister/"),
	  noAuth|noPipe, unregisterEnter, NULL, NULL },
    { "content", boost::regex(".*accounts/"),
	  noAuth|noPipe, contribIdxFetch, NULL, NULL },
    /* misc pages */
    { "content", boost::regex(".*/commit/[0-9a-f]{40}"),
	  noAuth|noPipe, changeShowDetails, NULL, NULL },
    { "content", boost::regex(".*/tests/.+\\.xml"),
      noAuth|noPipe, NULL, NULL, junitContent },
    { "content", boost::regex(".*/todo/"),
	  noAuth|noPipe, todoIndexWriteHtmlFetch, NULL, NULL },
    { "content", boost::regex(".*\\.eml"),
	  noAuth|noPipe, mailParserFetch, NULL, NULL },
    { "content", boost::regex(".*\\.ics"),
	  noAuth|noPipe, NULL, calendarFetch,  NULL },
    { "content", boost::regex(".*\\.todo"),
	  noAuth|noPipe, todoWriteHtmlFetch, NULL, NULL },
    { "content", boost::regex(".*dws\\.xml"),
	  noAuth|noPipe, NULL, NULL, projindexFetch },
    { "content", boost::regex(".*"),
      noAuth|noPipe, NULL, textFetch, NULL },

	// 1. XXX
    { "date", boost::regex(".*\\.blog"),
	  noAuth|noPipe, NULL, textMeta<date>, NULL },
    { "date", boost::regex(".*/tests/.+\\.xml"),
      noAuth|noPipe, junitDate, NULL, NULL },
    { "dates", boost::regex(".*/blog/.*"),
	  noAuth|noPipe, blogDateLinks<blogPat>, NULL, NULL },

    /* Documents split a page into a *content* and *sidebar* sections.
       Both are based on the context as specified by url. */
    { "document", boost::regex(".*/diff/[0-9a-f]{40}"),
      noAuth|noPipe, compose<source>, NULL, NULL },
    { "document", boost::regex(".*\\.(cc|hh|tcc)$"),
      noAuth|noPipe, compose<source>, NULL, NULL },
    { "document", boost::regex(".*\\.(c|h)$"),
      noAuth|noPipe, compose<source>, NULL, NULL },
    { "document", boost::regex(".*\\.mk"),
      noAuth|noPipe, compose<source>, NULL, NULL },
    { "document", boost::regex(".*\\.py"),
      noAuth|noPipe, compose<source>, NULL, NULL },
    { "document", boost::regex(".*Makefile"),
      noAuth|noPipe, compose<source>, NULL, NULL },
    { "document", boost::regex(".*\\.book"),
      noAuth|noPipe, compose<bookExt>, NULL, NULL },
    { "document", boost::regex(".*dws\\.xml"),
      noAuth|noPipe, compose<project>, NULL, NULL },

     /* Widget to generate a rss feed. Attention: it needs
       to be declared before any of the todoFilter::viewPat
       (i.e. todos/.+) since an rss feed exists for todo items
       as well. */
    { "document", boost::regex("/index\\.rss"),
      noAuth|noPipe, rssSiteAggregate<docPage>, NULL, NULL },
    { "document", boost::regex(".*\\.git/index\\.rss"),
      noAuth|noPipe, feedRepository<rsswriter>, NULL, NULL },
    { "document", boost::regex(".*/index\\.rss"),
      noAuth|noPipe, feedLatestPosts<rsswriter, docPage>, NULL, NULL },

    /* All files in the blog/ subpath will use a taylored *content*
       and *sidebar*. */
    { "document", boost::regex(".*/blog/.*"),
      noAuth|noPipe, compose<blogExt>, NULL, NULL },
    { "document", boost::regex(".*\\.todo"),
      noAuth|noPipe, compose<todoExt>, NULL, NULL },
    /* Composer and document for the todos index view */
    { "document", boost::regex(".*/todo/"),
      noAuth|noPipe, compose<todos>, NULL, NULL },
    { "document", boost::regex("/"),
      noAuth|noPipe, compose<indexPage>, NULL, NULL },
    { "document", boost::regex(".*"),
      noAuth|noPipe, compose<docLayout>, NULL, NULL },

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
       This widget is used through different "base"s to browse
       the source repository. */
    { "projfiles", boost::regex(".*"),
	  noAuth|noPipe, projfilesFetch, NULL, NULL },
    /* A project dws.xml "document" file show a description,
       commits and unit test status of a single project through
       a project "base". */
    { "regressions", boost::regex(".*dws\\.xml"),
	  noAuth|noPipe, regressionsFetch, NULL, NULL },

    { "relates", boost::regex(".*/blog/.*"),
	  noAuth|noPipe, blogRelatedSubjects<blogPat>, NULL, NULL },

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
    { "title", boost::regex(".*\\.todo"),
	  noAuth|noPipe, NULL, todoMeta, NULL },
    { "title", boost::regex(".*dws\\.xml"),
	  noAuth|noPipe, projectTitle, NULL, NULL },
    { "title", boost::regex(".*"),
	  noAuth|noPipe, metaFetch<title>, NULL, NULL },

    { "topMenu", boost::regex(".*"),
	  noAuth|noPipe, compose<topMenu>, NULL, NULL },
};

dispatchDoc semDocs(entries,sizeof(entries)/sizeof(entries[0]));
