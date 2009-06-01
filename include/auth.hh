#ifndef guardauth
#define guardauth

#include <exception>
#include "document.hh"

class invalidAuthentication : public std::exception {
public:

    virtual const char* what() const throw() {
		return "invalidAuthentication";
    }
};


class login : public document {
public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};


class logout : public document {
public:
    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};


#endif
