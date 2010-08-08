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

#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <boost/date_time.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem/fstream.hpp>
#include "auth.hh"
#include "composer.hh"

#include <security/pam_appl.h>

void getPassword(char *recvbuf)
{
    int i=0, c;
   char buf[20];

   std::cerr << "asking for password..." << std::endl;
   while((c = fgetc(stdin)) != '\n')
         buf[i++] = c;
   buf[i] = '\0';

   strcpy(recvbuf,buf);

}


int su_conv( int num_msg,
	     const struct pam_message **msgm,
	     struct pam_response **resp,
	     void *appdata)
{
    session *s = (session*)appdata;
    
    int count;
    struct pam_response *r;   
    r = (struct pam_response*)calloc(num_msg,sizeof(struct pam_response));
    
    for( count=0; count < num_msg; ++count) {	
	char *value = NULL;
	switch(msgm[count]->msg_style) {
        case PAM_PROMPT_ECHO_OFF:
	    std::cerr << msgm[count]->msg;
	    value = (char*)malloc((s->valueOf("credentials").size() + 1)
				  * sizeof(char));
	    strcpy(value,s->valueOf("credentials").c_str());
	    break;
	case PAM_PROMPT_ECHO_ON:
	    std::cerr << msgm[count]->msg;
	    value = (char*)malloc((s->username.size() + 1)
				  * sizeof(char));
	    strcpy(value,s->username.c_str());
	    break;	    
        case PAM_ERROR_MSG:
	case PAM_TEXT_INFO:
	    std::cerr << msgm[count]->msg;
	    break;
        default:
	    std::cerr << "Erroneous Conversation (" 
		      << msgm[count]->msg_style << ")" << std::endl;
	}
	r[count].resp_retcode = 0;
	r[count].resp = value;
    }
    *resp = r;
    return PAM_SUCCESS;
}


void auth::fetch( session& s, const boost::filesystem::path& pathname ) {
#if 0
    /* This code is used to debug initial permission problems. */
    struct passwd *pw;
    struct group *grp;
    
    pw = getpwuid(getuid());
    grp = getgrgid(getgid());
    cerr << "real      : " << getuid() 
	 << '(' << pw->pw_name << ")\t" << getgid() 
	 << '(' << grp->gr_name << ')' << endl;
    pw = getpwuid(geteuid());
    grp = getgrgid(getegid());
    cerr << "effective : " << geteuid() 
	 << '(' << pw->pw_name << ")\t" << getegid() 
	 << '(' << grp->gr_name << ')' << endl;
#endif

    s.start();
}


void login::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;
    
    session::variables::const_iterator credentials = s.vars.find("credentials");
    if( s.username.empty() ) {
	throw invalidAuthentication();
    }

    /* PAM authentication 
       http://www.freebsd.org/doc/en/articles/pam/pam-essentials.html
       http://linux.die.net/man/3/pam
       http://www.linuxjournal.com/article/5940
     */
    struct pam_conv conv = {
        su_conv,        //function pointer to the
                        //conversation function
        &s
    };

    pam_handle_t *pamh = NULL;
    int retval = 0;
    /* service, user, pam_conv, handle */
    retval =  pam_start("semilla",NULL,&conv,&pamh);
    if( retval != PAM_SUCCESS ) {
	std::cerr << "error: pam_start" << std::endl;
	throw invalidAuthentication();
    }
    /* handle, flags */
    retval = pam_authenticate(pamh,0);
    if(retval == PAM_SUCCESS) {
        std::cerr << "Authenticated." << std::endl;
    } else {
	std::cerr << "Authentication Failed." << std::endl;
	throw invalidAuthentication();
    }
    int pam_status = 0; 
    if(pam_end(pamh,pam_status) != PAM_SUCCESS) { 
	pamh = NULL;        
    }

    s.start();
    
    /* \todo generate a random unique session id */
    uint64_t sessionId = 12345678;
    s.id = sessionId;
    s.store();
    
    /* Set a session cookie */
    std::stringstream id;
    id << sessionId;
    *ostr << cookie("session",id.str());
    *ostr << redirect(s.docAsUrl()) << '\n';
}


boost::posix_time::time_duration
deauth::aggregate( const session& s ) const {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;

    ifstream file(s.contributorLog());
    if( file.fail() ) {
	boost::throw_exception(basic_filesystem_error<path>(
				 std::string("error opening file"),
				 s.contributorLog(), 
				 error_code()));
    }

    time_duration aggr;
    ptime start, stop, prev;
    while( !file.eof() ) {
	std::string line;
	getline(file,line);
	try {
	    size_t first = 0;
	    size_t last = line.find_first_of(' ',first);
	    last = line.find_first_of(' ',last + 1);
	    start = time_from_string(line.substr(first,last));

	    first = last + 1;
	    last = line.find_first_of(' ',first);
	    last = line.find_first_of(' ',last + 1);
	    stop = time_from_string(line.substr(first,last-first));

	    aggr += stop - start;
	    if( !prev.date().is_not_a_date() 
		&& start.date().month() != prev.date().month() ) {
		aggr = hours(aggr.hours()) 
		    + (( aggr.minutes() > 0 ) ? hours(1) : hours(0));
		std::cout << prev.date().year() << ' ' 
			  << prev.date().month() << ": " 
			  << aggr.hours() << " hours"  << std::endl;
		aggr = time_duration();
	    }
	    prev = start;
	} catch(...) {
	    /* It is ok if we cannot interpret some lines in the log;
	       most notably the last empty line. */
	}
    }
    file.close();
    aggr = hours(aggr.hours())
	+ (( aggr.minutes() > 0 ) ? hours(1) : hours(0));
    std::cout << prev.date().year() << ' ' 
	      << prev.date().month() << ": " 
	      << aggr.hours() << " hours" << std::endl;

    return aggr;
}


void deauth::fetch(  session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::posix_time;
    time_duration logged = s.stop();
    aggregate(s);
    *ostr << "last session ran for ";
    const char *sep = "";
    if( logged.hours() > 0 ) {
	*ostr << logged.hours() << " hours";
	sep = " ";
    }
    if( logged.minutes() > 0 ) {
	*ostr << sep << logged.minutes() << " mins";
	sep = " ";
    }
    if( strlen(sep) == 0 ) {
	*ostr << "less than a minute";
    }
    *ostr << "." << std::endl;
}


void logout::fetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;

    if( s.id != 0 ) {
	std::stringstream id;
	id << s.id;
	*ostr << cookie("session",id.str(),boost::posix_time::ptime::date_duration_type(-1));
    }
    time_duration logged = s.stop();
    std::stringstream logstr;
    logstr << logged.hours() << " hours logged." << std::endl;
    
    *ostr << htmlContent << std::endl;
    s.vars["hours"] = logstr.str();
    path uiPath(s.vars["uiDir"] + std::string("/logout.ui"));
    composer pres(*ostr,uiPath,composer::error);
    pres.fetch(s,"document");
}


