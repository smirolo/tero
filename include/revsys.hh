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

#ifndef guardrevsys
#define guardrevsys

#include "post.hh"
#include "decorator.hh"
#if 1
#include <boost/system/error_code.hpp>
#include <boost/filesystem/operations.hpp>
#endif

class historyref {
public:
    historyref() {}

    virtual url asUrl( const boost::filesystem::path& doc,
        const std::string& rev ) const = 0;
};


/** Base class definition for interacting with a source code control system.
 */
class revisionsys {
public:
    typedef std::vector<revisionsys*> revsSet;
    static revsSet revs;

    boost::filesystem::path rootpath;

    /** Directory name in which revision control meta information
        is stored (example: .git).
    */
    const char* metadir;

    std::map<std::string,std::string> config;

public:
    explicit revisionsys( const char* m ) : metadir(m) {}

    virtual void loadconfig( session& s ) = 0;

    const std::string& configval( const std::string& key );

    boost::filesystem::path
    absolute( const boost::filesystem::path& p ) const;

    virtual boost::filesystem::path
    relative( const boost::filesystem::path& p ) const;

    virtual void diff( std::ostream& ostr,
        const std::string& leftCommit,
        const std::string& rightCommit,
        const boost::filesystem::path& pathname ) = 0;

    virtual void history( std::ostream& ostr,
        const session& s,
        const boost::filesystem::path& pathname,
        historyref& r ) = 0;

    virtual void checkins( const session& s,
        const boost::filesystem::path& pathname,
        postFilter& filter ) = 0;

    virtual void showDetails( std::ostream& ostr,
        const std::string& commit ) = 0;

    /** Show a file in the repository at a specified *commit*,
        using a decorator to do syntax highlighting. */
    virtual void show( std::ostream& ostr,
        const boost::filesystem::path& pathname,
        const std::string& commit,
        decorator *dec = NULL ) = 0;

    virtual slice<char> loadtext( const boost::filesystem::path& pathname,
        const std::string& commit ) = 0;

    virtual std::streambuf* openfile( const boost::filesystem::path& pathname,
        const std::string& commit ) = 0;

    /** returns a revision system when dirname can be reliably determined
        to be a bare repository or a top level repository clone.
    */
    static revisionsys*
    exists( session& s, const boost::filesystem::path& dirname );

    /** returns the revision system associated with a pathname
     */
    static revisionsys*
    findRev( session& s, const boost::filesystem::path& pathname );

    /** returns the revision system associated with a specific metadir
     */
    static revisionsys*
    findRevByMetadir( session& s, const std::string& metadir );

    static std::streambuf* findRevOpenfile(
        session& s,
        const boost::filesystem::path& pathname );

    /** Load a text file using the most appropriate revision control
        system based on *pathname* */
    static slice<char> loadtext(
        session& s,
        const boost::filesystem::path& pathname );

};

class rev_directory_iterator;

namespace detail {

    struct rev_dir_itr_imp {
        boost::filesystem::directory_entry  dir_entry;
        std::list<std::string> entries;
        std::list<std::string>::const_iterator next;

        rev_dir_itr_imp() {}

        ~rev_dir_itr_imp() {} // never throws
    };

    void rev_directory_iterator_construct( rev_directory_iterator& it,
        const boost::filesystem::path& p, boost::system::error_code* ec,
        session& ses );

    void rev_directory_iterator_increment( rev_directory_iterator& it,
        boost::system::error_code* ec);

}

/** Iterator for revisionned controlled systems.
 */
class rev_directory_iterator
    : public boost::iterator_facade< rev_directory_iterator,
                                     boost::filesystem::directory_entry,
                                     boost::single_pass_traversal_tag > {
public:

    rev_directory_iterator(){}  // creates the "end" iterator

    // iterator_facade derived classes don't seem to like implementations in
    // separate translation unit dll's, so forward to detail functions
    rev_directory_iterator(
        const boost::filesystem::path& p, session& ses )
        : m_imp(new detail::rev_dir_itr_imp)
    { detail::rev_directory_iterator_construct(*this, p, 0, ses); }

    rev_directory_iterator(
        const boost::filesystem::path& p,
        boost::system::error_code& ec,
        session& ses )
        : m_imp(new detail::rev_dir_itr_imp)
    { detail::rev_directory_iterator_construct(*this, p, &ec, ses); }

    ~rev_directory_iterator() {} // never throws

    rev_directory_iterator& increment( boost::system::error_code& ec ) {
        detail::rev_directory_iterator_increment(*this, &ec);
        return *this;
    }

private:

    friend struct detail::rev_dir_itr_imp;

    friend void detail::rev_directory_iterator_construct(
        rev_directory_iterator& it,
        const boost::filesystem::path& p,
        boost::system::error_code* ec,
        session& ses );

    friend void detail::rev_directory_iterator_increment(
        rev_directory_iterator& it,
        boost::system::error_code* ec );

    // shared_ptr provides shallow-copy semantics required for InputIterators.
    // m_imp.get()==0 indicates the end iterator.
    boost::shared_ptr< detail::rev_dir_itr_imp >  m_imp;

    friend class boost::iterator_core_access;

    boost::iterator_facade<
      rev_directory_iterator,
      boost::filesystem::directory_entry,
      boost::single_pass_traversal_tag >::reference dereference() const {
        BOOST_ASSERT_MSG(m_imp.get(), "attempt to dereference end iterator");
        return m_imp->dir_entry;
    }

    void increment() { detail::rev_directory_iterator_increment(*this, 0); }

    bool equal(const rev_directory_iterator& rhs) const
      { return m_imp == rhs.m_imp; }
};



#endif
