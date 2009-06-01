#ifndef guardcppcode
#define guardcppcode

#if 0

#include <ostream>
#include "cpptok.hh"

template<typename charT, typename traitsT = std::char_traits<charT> >
class basicCppLight;

typedef basicCppLight<char> cppLight;


/** \brief source code token coloring for C++.
 */
template<typename charT, typename traitsT>
class basicCppLight : virtual public std::basic_ostream<charT, traitsT> {
public:
	// Types (inherited from basic_ios (27.4.4)):
	typedef charT                     		   char_type;
	typedef typename traitsT::int_type 		   int_type;
	typedef typename traitsT::pos_type 	   pos_type;
	typedef typename traitsT::off_type 		   off_type;
	typedef traitsT                    		   traits_type;
	
	// Non-standard types
	typedef basicCppLight<charT, traitsT>	   self;
	typedef std::basic_ostream<charT, traitsT> super;

protected:

	cppTokenizer tokenizer;
	htmlCppTokListener<charT,traitsT> listener;

public:
    explicit basicCppLight( super& ostr )
	: listener(ostr), tokenizer(listener) {}
    
    /** \brief  Character string insertion.
     */
    self& write( const char_type* s, std::streamsize n );
    
    /** \brief  Synchronizing the stream buffer.
		\return  *this
    */
    self& flush();
};


template<typename charT, typename traitsT>
typename basicCppLight<charT,traitsT>::self& 
basicCppLight<charT,traitsT>::write( const char_type* s, std::streamsize n ) {
	tokenizer.tokenize(s);
}


template<typename charT, typename traitsT>
typename basicCppLight<charT,traitsT>::self&
basicCppLight<charT,traitsT>::flush() {
    listener.flush();
    return *this;
}

#endif


#endif
