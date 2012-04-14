/* Copyright (c) 2009-2012, Fortylines LLC
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

/* Read code coverage information

   Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/

#include "coverage.hh"
#include <sstream>
#include <boost/filesystem/fstream.hpp>

namespace {

	void addCovered( std::vector<int>& ranges, int line )
	{
		if( ranges.empty() ) {
			ranges.push_back(line);
		} else {
			int last = ranges.back();
			if( last + 1 != line ) {
				ranges.push_back(line);
			}
		}
	}
	
} // anonymous


void loadPycoverage( std::vector<int> ranges,
					 const boost::filesystem::path& key,
					 const boost::filesystem::path& filename )
{
	/* .coverage are written by pickle.py in binary format.
	   We donot implement the whole byte code interpreter for pickling
	   and unplicking here, only the part necessary to fetch the lines
	   of code covered in file *key*. */
	using namespace boost::filesystem;
	
	int proto, val, len;
	char buf[256];
	ifstream coverfile(filename);

	while( !coverfile.eof() ) {
		int key = coverfile.get();
		switch( key ) {
		case 0x80:
			/* pickle protocol */
			proto = coverfile.get();
			if( (proto < 0) || (proto > 2) ) {
				boost::throw_exception(
				  parsingError("unsupported pickle protocol"));
			}
			break;
		case '}':  
			/* push empty dict */
			break;
		case ']':
			/* 0x5d: push empty list */
			break;
		case 'q':
			/* 0x71: store stack top in memo; index is 1-byte arg */
			val = coverfile.get();
			break;
		case '(':
			/* 0x28: push special markobject on stack */
			break;
		case 'e':
			/* 0x65: extend list on stack by topmost stack slice */
			break;
		case 's':
			/* 0x73: add key+value pair to dict */
			break;
		case 'u':
			/* 0x75: modify dict by adding topmost key+value pairs */
			break;
		case '.':
			/* every pickle ends with STOP */
			return;
			break;
		case 'U':
			/* push string; counted binary string argument < 256 bytes */
			len = coverfile.get();
			coverfile.get(buf,len + 1);
			break;
		case 'K':
			/* 0x4b: push 1-byte unsigned int */
			addCovered(ranges,coverfile.get());
			break;
		case 'M':
			/* 0x4d: push 2-byte unsigned int */
			val = coverfile.get();
			addCovered(ranges,(coverfile.get() << 8) | val);
			break;
		default: {
			std::stringstream msg;
			msg << filename << ":" << coverfile.gcount()
				<< ":error: unexpected bytecode 0x"
				<< std::hex << key << std::dec;
			boost::throw_exception(parsingError(msg.str()));
		}
		}
	}
}
