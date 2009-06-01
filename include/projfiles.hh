#ifndef guardprojfiles
#define guardprojfiles

#include "document.hh"

class projfiles : public document {
public:
	typedef std::list<boost::regex> filterContainer;

protected:
	filterContainer filters;

	void dirLink( const session& s, const boost::filesystem::path& dir ) const;
	
	void fileLink( const session& s, const boost::filesystem::path& dir ) const;

	bool selects( const boost::filesystem::path& pathname ) const;

public:
	projfiles() {}

	template<typename iter>
	projfiles( iter first, iter last ) {
		std::copy(first,last,std::back_inserter(filters));
	}

    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};


#endif
