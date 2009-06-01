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

	boost::filesystem::path findFile( const boost::filesystem::path& name ) const;
	
	/** \brief Load a session from persistent storage 
	 */
	void restore( const boost::program_options::variables_map& params );

	std::string root() const {
		return std::string("/dev/bin/wiki");
	}

	/** \brief Display debug information for the session
	 */
	void show( std::ostream& ostr );

	/** Store session information into persistent storage 
	 */
	void store();
};

#endif
