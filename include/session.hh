/* Copyright (c) 2009, Sebastien Mirolo
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of codespin nor the
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

#ifndef guardsession
#define guardsession

#include <map>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "webserve.hh"

class session {
public:
    typedef std::map<std::string,std::string> variables;
    
public:
    
    static boost::filesystem::path storage;
    
    uint64_t id;
    std::string username;
    variables vars;
    
    
    session() : id(0) {}

    /** document name as an url 
     */
    std::string docAsUrl() const;
    
    /** returns the value of a variable
     */
    const std::string& valueOf( const std::string& name ) const;
    
    /** absolute name to the user personal directory
     */
    boost::filesystem::path userPath() const;
    
    bool exists() const { return id > 0; }
    
    /** \brief Value of a variable as an absolute pathname
	
	If the variable exists in the session and its value can
	be interpreted as an absolute path, that value is returned 
	as such.
	If the variable exists in the session and its value can
	be interpreted as a relative path, the following directories
	are searched until a match is found:
	- pwd
	- topSrc
    */
    boost::filesystem::path pathname( const std::string& name ) const;
    
    boost::filesystem::path 
    findFile( const boost::filesystem::path& name ) const;
    
    /** \brief Load a session from persistent storage 
     */
    void restore( const boost::program_options::variables_map& params );
    
    std::string root() const {
	return std::string("");
    }
    
    /** \brief Display debug information for the session
     */
    void show( std::ostream& ostr );
    
    /** Store session information into persistent storage 
     */
    void store();
};

#endif
