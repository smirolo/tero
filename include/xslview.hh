#ifndef guardxslview
#define guardxslview

#include "document.hh"

class xslview : public document {
public:

    virtual void fetch( session& s, const boost::filesystem::path& pathname );
};

#endif
