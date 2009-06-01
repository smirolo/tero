#include <cstdio>
#include "changelist.hh"

void gitcmd::diff( std::ostream& ostr, 
				   const std::string& leftCommit, 
				   const std::string& rightCommit, 
				   const boost::filesystem::path& pathname ) {
	/* The git command needs to be issued from within a directory where .git can be found
	   by walking up the tree structure. */ 
	boost::filesystem::current_path(topSrc);

	std::stringstream cmd;
	cmd << executable << " diff " << leftCommit << " " << rightCommit << " " << pathname; 

	std::cerr << "git cmd: " << cmd.str() << std::endl;

	FILE *diffFile = popen(cmd.str().c_str(),"r");
	if( diffFile == NULL ) {
		std::cerr << "error opening command: " << cmd.str() << std::endl;
		return;
	}
	char line[256];
	std::stringstream diff;
	while( fgets(line,sizeof(line),diffFile) != NULL ) {
		ostr << line;
	}
	pclose(diffFile);
}


void gitcmd::history( std::ostream& ostr, 
					  const boost::filesystem::path& pathname ) {

	/* The git command needs to be issued from within a directory where .git can be found
	   by walking up the tree structure. */ 
	boost::filesystem::current_path(topSrc);

	std::stringstream sstm;
	sstm << executable << " show --summary --pretty=oneline " << pathname; 

	char line[256];
	FILE *summary = popen(sstm.str().c_str(),"r");
	assert( summary != NULL );

	while( fgets(line,sizeof(line),summary) != NULL ) {
		ostr << line;
	}
	pclose(summary);

}
