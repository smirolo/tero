/* Copyright (c) 2009-2011, Fortylines LLC
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

/** Authentication

    Primary Author(s): Sebastien Mirolo <smirolo@fortylines.com>
*/


namespace {

    sessionVariable credentials("credentials","password");
    sessionVariable username("username","username");
    pathVariable contribTop("contribTop",
			    "root to the contributors directories");
    sessionVariable authMessage("message",
			    "Message inserted in the contributor's hours log");


boost::posix_time::time_duration
aggregate( const session& s ) {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;

    ifstream file;
    openfile(file,contributorLog(s));
    
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


boost::posix_time::time_duration stop( session& s ) {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;

    ptime start = startTime.value(s);
    ptime stop = second_clock::local_time();

    ofstream file(contributorLog(s),std::ios_base::app);
    if( file.fail() ) {
	boost::throw_exception(basic_filesystem_error<path>(
				 std::string("error opening file"),
				 contributorLog(s), 
				 error_code()));
    }
    std::string msg = authMessage.value(s);
    if( !msg.empty() ) {
	file << msg << ':' << std::endl;
    }
    time_duration aggregate = stop - start;
    file << start << ' ' << stop << std::endl;
    file.close();
    s.remove();

    return aggregate;
}

} // anonymous namespace


boost::filesystem::path contributorLog( const session& s ) {
    return contribTop.value(s) / boost::filesystem::path("hours");
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
	    value = (char*)malloc((credentials.value(*s).size() + 1)
				  * sizeof(char));
	    strcpy(value,credentials.value(*s).c_str());
	    break;
	case PAM_PROMPT_ECHO_ON:
	    std::cerr << msgm[count]->msg;
	    value = (char*)malloc((username.value(*s).size() + 1)
				  * sizeof(char));
	    strcpy(value,username.value(*s).c_str());
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


void authAddSessionVars( boost::program_options::options_description& all,
			 boost::program_options::options_description& visible )
{
    using namespace boost::program_options;
    
    options_description localOptions("authentication");
    localOptions.add(username.option());
    localOptions.add(credentials.option());
    localOptions.add(contribTop.option());
    localOptions.add(authMessage.option());
    all.add(localOptions);
    visible.add(localOptions);
}


void authFetch( session& s, const boost::filesystem::path& pathname ) {

    /* This code is used to debug initial permission problems. */
    struct passwd *pw;
    struct group *grp;
    
    pw = getpwuid(getuid());
    grp = getgrgid(getgid());
#if 0
    std::cerr << "real      : " << getuid() 
	      << '(' << pw->pw_name << ")\t" << getgid() 
	      << '(' << grp->gr_name << ')' << std::endl;
#endif
    pw = getpwuid(geteuid());
    grp = getgrgid(getegid());
#if 0
    std::cerr << "effective : " << geteuid() 
	      << '(' << pw->pw_name << ")\t" << getegid() 
	      << '(' << grp->gr_name << ')' << std::endl;
#endif
    s.state("username",username.value(s));
    s.store();
}


void loginFetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;
    
    if( username.value(s).empty() ) {
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

    /* The authentication will create a persistent ("username",value)
       pair that, in turn, will create a sessionId and a startTime. */
    authFetch(s,pathname);
    
    /* Set a session cookie */
    httpHeaders.setCookie(s.sessionName,
			  s.id()).location(url(document.value(s).string()));
}


void deauthFetch(  session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::posix_time;

#if 0
    /* \todo !!! find where this should go! */
    variables::iterator iter = vars.find("message");
    if( iter != vars.end() ) iter->second.source = sessionfile;
#endif

    if( !s.exists() ) {
	using namespace boost::filesystem;
	for( directory_iterator entry 
		 = directory_iterator(s.stateDir()); 
	     entry != directory_iterator(); ++entry ) {
	    path filename(entry->filename());
	    s.id(boost::filesystem::basename(filename));
	    s.restore(0,NULL,boost::program_options::options_description());
	    break;
	}
    }

    time_duration logged = stop(s);
    aggregate(s);
    s.out() << "last session ran for ";
    const char *sep = "";
    if( logged.hours() > 0 ) {
	s.out() << logged.hours() << " hours";
	sep = " ";
    }
    if( logged.minutes() > 0 ) {
	s.out() << sep << logged.minutes() << " mins";
	sep = " ";
    }
    if( strlen(sep) == 0 ) {
	s.out() << "less than a minute";
    }
    s.out() << "." << std::endl;
}

char logoutLayout[] = "logout.ui";

void logoutFetch( session& s, const boost::filesystem::path& pathname ) {
    using namespace boost::system;
    using namespace boost::posix_time;
    using namespace boost::filesystem;

    if( !s.id().empty() ) {
	httpHeaders.setCookie(s.sessionName,s.id(),
			      boost::posix_time::ptime::date_duration_type(-1));
    }
    time_duration logged = stop(s);
    std::stringstream logstr;
    logstr << logged.hours() << " hours logged." << std::endl;
    s.state("hours",logstr.str());    
    composerFetch<logoutLayout>(s,path("document"));
}


