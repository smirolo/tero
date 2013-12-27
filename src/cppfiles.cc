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

#include "cppfiles.hh"
#include "document.hh"
#include "changelist.hh"
#include "decorator.hh"

namespace tero {

void cppFetch( session& s, std::istream& in, const url& name )
{
    linkLight leftLinkStrm(s, siteTop.value(s));
    linkLight rightLinkStrm(s, siteTop.value(s));
    cppLight leftCppStrm;
    cppLight rightCppStrm;
    decoratorChain leftChain;
    decoratorChain rightChain;
    leftChain.push_back(leftLinkStrm);
    leftChain.push_back(leftCppStrm);
    rightChain.push_back(rightLinkStrm);
    rightChain.push_back(rightCppStrm);
    text cpp(leftChain,rightChain);
    cpp.fetch(s,in);
}


void cppDiff( session& s, const url& name )
{
    using namespace boost;

    linkLight leftLinkStrm(s, siteTop.value(s));
    linkLight rightLinkStrm(s, siteTop.value(s));
    cppLight leftCppStrm;
    cppLight rightCppStrm;
    decoratorChain leftChain;
    decoratorChain rightChain;
    leftChain.push_back(leftLinkStrm);
    leftChain.push_back(leftCppStrm);
    rightChain.push_back(rightLinkStrm);
    rightChain.push_back(rightCppStrm);

    static const boost::regex diffRe("(\\S+)/([0-9a-f]{40}/)?diff/([0-9a-f]{40})");

	smatch m;
    std::string leftRevision;
    std::string rightRevision;
    boost::filesystem::path srcpath;
	if( regex_search(s.abspath(name).string(), m, diffRe) ) {
        srcpath = m.str(1);
        leftRevision = m.str(2);
        rightRevision = m.str(3);
        
    }

    changediff(s,srcpath,leftRevision,rightRevision,&leftChain,&rightChain);
}

}
