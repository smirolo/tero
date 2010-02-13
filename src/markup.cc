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

#include "markup.hh"

namespace html {

    const char* a::name = "a";
    const detail::nodeEnd a::end(a::name);
    const char *a::attrNames[] = {
	"href",
	"title"
    };

    const char* caption::name = "caption";
    const detail::nodeEnd caption::end(caption::name,true);

    const char* div::name = "div";
    const detail::nodeEnd div::end(div::name,true);
    const char *div::attrNames[] = {
	"class"
    };

    const char *h::names[] = {
	"h1",
	"h2",
	"h3",
	"h4",
	"h5"
    };

    const char* img::name = "img";
    const detail::nodeEnd img::end(img::name);
    const char *img::attrNames[] = {
	"class",
	"src"
    };

    const char* li::name = "li";
    const detail::nodeEnd li::end(li::name,true);

    const char* p::name = "p";
    const detail::nodeEnd p::end(p::name,true);

    const char* pre::name = "pre";
    const detail::nodeEnd pre::end(pre::name,true);
    const char *pre::attrNames[] = {
	"class"
    };

    const char* span::name = "span";
    const detail::nodeEnd span::end(span::name);
    const char *span::attrNames[] = {
	"class"
    };

    const char* table::name = "table";
    const detail::nodeEnd table::end(table::name,true);

    const char* td::name = "td";
    const detail::nodeEnd td::end(td::name,true);
    const char *td::attrNames[] = {
	"class",
	"colspan"
    };

    const char* th::name = "th";
    const detail::nodeEnd th::end(th::name,true);
    const char *th::attrNames[] = {
	"class",
	"colspan"
    };

    const char* tr::name = "tr";
    const detail::nodeEnd tr::end(tr::name,true);
    const char *tr::attrNames[] = {
	"class"
    };

    const char* ul::name = "ul";
    const detail::nodeEnd ul::end(ul::name,true);
    
} // namespace html

const char* author::name = "author";
const detail::nodeEnd author::end(author::name);

const char* channel::name = "channel";
const detail::nodeEnd channel::end(channel::name);

const char* description::name = "description";
const detail::nodeEnd description::end(description::name);

const char* guid::name = "guid";
const detail::nodeEnd guid::end(guid::name);

const char* item::name = "item";
const detail::nodeEnd item::end(item::name);

const char* rsslink::name = "link";
const detail::nodeEnd rsslink::end(rsslink::name);

const char* pubDate::name = "pubDate";
const detail::nodeEnd pubDate::end(pubDate::name);

const char* rss::name = "rss";
const detail::nodeEnd rss::end(rss::name);
const char *rss::attrNames[] = {
    "version"
};


const char* title::name = "title";
const detail::nodeEnd title::end(title::name);
