/**
 * @file options.cc
 * @author Pauli Virtanen
 *
 * Parse command line options and assume sensible defaults.
 */
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include "i18n.hh"
#include "common.hh"
#include "regexp.hh"
#include "config.hh"
#include "options.hh"

#include "tmerror.hh"

using std::string;
using std::vector;
using std::pair;
using std::istringstream;

/** Return the user's home directory */
string get_home_dir()
{
	char const* homedir = getenv("HOME");
	if (homedir)
		return string(homedir);
	else
		return string();
}

/**
 * Deduce the filter needed by the given file by the file extension.
 */
Options::FilterType Options::guess_file_filter(string const& filename)
{
	static RegExp::Flag const rf = RegExp::EXTENDED | RegExp::ICASE
		                                        | RegExp::NOSUB;
	static RegExp roff_re("\\.(ms|mm|me|man)$", rf);
	static RegExp tex_re("\\.(tex)$", rf);
	static RegExp html_re("\\.(htm|html|sgml)$", rf);
	
	if (roff_re.match(filename)) {
		return nroff;
	} else if (tex_re.match(filename)) {
		return tex;
	} else if (html_re.match(filename)) {
		return sgml;
	} else {
		return plain;
	}
}

/**
 * Print usage information
 */
void Options::print_usage()
{
	std::cout << ssprintf(
_("Usage: %s [options] [file]...\n"
  "Options: [FMNLVlfsaAtnhgbxBCPmSdpwWTv]\n"
  "\n"
  " -F <file>  Use given file as the configuration file.\n"
  "\n"
  "The following flags are same for ispell:\n"
  " -v[v]      Print version number and exit.\n"
  " -M         One-line mini menu at the bottom of the screen.\n"
  " -N         No mini menu at the bottom of the screen.\n"
  " -L <num>   Number of context lines.\n"
  " -V         Use \"cat -v\" style for characters not in the 7-bit ANSI\n"
  "            character set.\n"
  " -l         Only output a list of misspelled words.\n"
  " -f <file>  Specify the output file.\n"
  " -s         Issue SIGTSTP at every end of line.\n"
  " -a         Read commands.\n"
  " -A         Read commands and enable a command to include a file.\n"
  " -e[e1234]  Expand affixes.\n"
  " -c         Compress affixes.\n"
  " -D         Dump affix tables.\n"
  " -t         The input is in TeX format.\n"
  " -n         The input is in [nt]roff format.\n"
  " -h         The input is in sgml format.\n"
  " -b         Create backup files.\n"
  " -x         Do not create backup files.\n"
  " -B         Do not allow run-together words.\n"
  " -C         Allow run-together words.\n"
  " -P         Do not generate extra root/affix combinations.\n"
  " -m         Allow root/affix combinations that are not in dictionary.\n"
  " -S         Sort the list of guesses by probable correctness.\n"
  " -d <dict>  Specify an alternate dictionary file.\n"
  " -p <file>  Specify an alternate personal dictionary.\n"
  " -w <chars> Specify additional characters that can be part of a word.\n"
  " -W <len>   Consider words shorter than this always correct.\n"
  " -T <fmt>   Assume a given formatter type for all files.\n"
  " -r <cset>  Specify the character set of the input.\n"),
  PACKAGE) << std::endl;
}

/**
 * Print version information to cout.
 */
static void print_version()
{
	// This should not be localized: some programs look for this.
	std::cout << "@(#) International Ispell Version 3.1.20 compatible "
		  << PACKAGE_STRING
		  << std::endl;
}

/**
 * Form the ispell argv.
 * The array is newly allocated, but its contents are constant, and
 * should not be modified.
 */
typedef char const* argv_t;
char const** Options::get_ispell_argv(string const& prog_name) const
{
	char const** argv = new argv_t[ispell_args_.size() + 2];

	int i = 0;
	
	argv[i++] = prog_name.c_str();

	vector<string>::const_iterator it;
	for (it = ispell_args_.begin(); it != ispell_args_.end(); ++it)
	{
		argv[i++] = it->c_str();
	}

	argv[i] = 0;

	return argv;
}

/**
 * Extract the main dictionary's identifier from the hash file name as given to
 * ispell. This implementation returns the part of the file name between the 
 * last '/' and '.'.
 *
 * Example: /usr/lib/ispell/americanmed+.hash => americanmed+
 */
static string extract_dictionary_identifier(string const& hash_file_name)
{
	RegExp re1 = RegExp("([^/]*)\\.hash$", RegExp::ICASE|RegExp::EXTENDED);
	RegExp re2 = RegExp("([^/]*)$", RegExp::ICASE|RegExp::EXTENDED);
	
	if (re1.match(hash_file_name)) {
		return re1.sub(hash_file_name, 1);
	} else if (re2.match(hash_file_name)) {
		return re2.sub(hash_file_name, 1);
	} else {
		return hash_file_name;
	}
}

/**
 * Recognizes options and extracts arguments from the program argument list.
 * Arguments are directly concatenated to options or in the following argv
 * entry. For example: -d<argument> or -d <argument>
 */
class OptionParser
{
public:
	OptionParser(int argc, char* const* argv)
		: argc_(argc), argv_(argv), i_(1), argument_in_next_(false)
		{}

	void next_option() {
		++i_; 
		if (argument_in_next_) {
			argument_in_next_ = false;
			++i_;
		}
	}
	bool has_next_option() const { return i_ < argc_; }
	bool is_option(char const* opt=0, char const** arg = 0);

	void push_to(vector<string>& args) const 
		{
			args.push_back(argv_[i_]);
			if (argument_in_next_)
				args.push_back(argv_[i_ + 1]);
		}

	char const* opt() const { return argv_[i_]; }

private:
	int argc_;
	char* const* argv_;
	int i_;
	bool argument_in_next_;
};

bool OptionParser::is_option(char const* opt, char const** arg_p)
{
	char const* s = argv_[i_];

	if (opt == 0) {
		if (s[0] == '-') 
			return true;
		else
			return false;
	}

	int opt_len = strlen(opt);

	if (strncmp(s, opt, opt_len) != 0) return false;
	
	char const* arg = (s + opt_len);
	
	if (arg_p == 0) {
		// If the option should not have an argument, then
		// the whole argv[i] must be the option.
		return *arg == '\0';
	}

	if (*arg == '\0') {
		// The argument is in the next argv position
		argument_in_next_ = true;

		if (i_ + 1 >= argc_) {
			throw Error(_("Missing argument for option %s"), opt);
		}
		*arg_p = argv_[i_ + 1];
	} else {
		argument_in_next_ = false;
		*arg_p = arg;
	}
	return true;
}

/**
 * Parse the given command line arguments, and assume sensible defaults.
 *
 * Some options understood by the real ispell are ignored, because they
 * are irrelevant to the spell checking engine used by this program.
 */
Options::Options(int argc, char* const* argv)
	: mode_(normal), // Normal operation mode
	  backups_(true), // Backup files
	  pipe_include_command_(), // Pipe mode include command disabled
	  sigstop_at_eol_(false), // No SIGTSTOP needed
	  dictionary_("default"), // Use default dictionary
	  personal_dictionary_(), // Default determined later
	  spellchecker_entry_(0), // No active spell checker by default
	  extra_word_characters_(), // No extra word characters by default
	  legal_word_length_(0), // All strings of word characters are words
	  default_filter_(plain), // Use plain text filter 
	  files_(), // No files to check
	  ansi7_(false), // No 7 bit ANSI
	  mini_menu_(true), // Mini menu enabled
	  context_lines_(-1), // Number of context lines by screen size
	  output_file_(), // Output to stdout
	  config_file_(CONFIG_FILE), // Default configuration file
	  user_encoding_(),
	  ispell_args_()
{
	FilterType next_filter = plain;
	FilterType default_filter = plain;
	bool default_filter_set = false;
	bool next_filter_set = false;

	for (OptionParser p(argc, argv);
	     p.has_next_option();
	     p.next_option()) {
		
		const char* arg;

		if (p.is_option("-F", &arg)) { // Configuration file
			config_file_ = arg;
			continue; // This argument will not be passed to ispell

		} else if (p.is_option("-v") ||
			   p.is_option("--version")) { // Print version
			print_version();
			mode_ = quit;
			return;
		} else if (p.is_option("-vv")) { // Print extra information
			mode_ = ispell;

		} else if (p.is_option("--help")) { // Print usage
			print_usage();
			mode_ = quit;
			return;

		} else if (p.is_option("-M")) { // One-line mini menu at bottom
			mini_menu_ = true;

		} else if (p.is_option("-N")) { // Suppress the mini menu
			mini_menu_ = false;

		} else if (p.is_option("-L", &arg)) {// Number of context lines
			istringstream str(arg);
			str >> context_lines_;
			
		} else if (p.is_option("-V")) {// Chars not in 7-bit ansi
			                       // displayed as by cat -v
			ansi7_ = true;

		} else if (p.is_option("-l")) { // List misspelled words
			mode_ = list;

		} else if (p.is_option("-f", &arg)) { // The output file
			output_file_ = arg;
			
		} else if (p.is_option("-s")) { // SIGTSTP at end of line
			sigstop_at_eol_ = true;

		} else if (p.is_option("-a")) { // pipe controlled mode
			mode_ = pipe;
			pipe_include_command_ = string();

		} else if (p.is_option("-A")) { // pipe controlled
			                        // include-enabled mode
			mode_ = pipe;
			pipe_include_command_ = "&Include_File&";
			if (getenv("INCLUDE_STRING")) {
				pipe_include_command_=getenv("INCLUDE_STRING");
			}

		} else if (p.is_option("-e") || // Affix expansion mode
			   p.is_option("-ee") || // Affix expansion mode
			   p.is_option("-e1") || // Affix expansion mode
			   p.is_option("-e2") || // Affix expansion mode
			   p.is_option("-e3") || // Affix expansion mode
			   p.is_option("-e4") || // Affix expansion mode
			   p.is_option("-c") || // Affix compression mode
			   p.is_option("-D")) { // Dump affix tables
			mode_ = ispell;
		
		} else if (p.is_option("-t")) { // Input in TeX format
			next_filter = tex;
			next_filter_set = true;

		} else if (p.is_option("-n")) {// Input in [nt]roff format
			next_filter = nroff;
			next_filter_set = true;

		} else if (p.is_option("-h")) { // Input in SGML format
			next_filter = sgml;
			next_filter_set = true;

		} else if (p.is_option("-b")) { // Create .bak backup files
			backups_ = true;

		} else if (p.is_option("-x")) { // Do not create backup files
			backups_ = false;

		} else if (p.is_option("-B") || // Disallow run-together words
			   p.is_option("-C") || // Allow run-together words
			   p.is_option("-P") || // No extra affix combinations
			   p.is_option("-m") || // Extra affix combinations
			   p.is_option("-S")) { // Sort guesses by correctness
			// (ignored)

		} else if (p.is_option("-d", &arg)) { // Dictionary to use
			dictionary_ = arg;
			dictionary_identifier_ = 
				extract_dictionary_identifier(dictionary_);

		} else if (p.is_option("-p", &arg)) { // Personal dict. to use
			personal_dictionary_ = arg;

		} else if (p.is_option("-w", &arg)) { // Additional word chars
			extra_word_characters_.assign(arg,
						      arg + strlen(arg));

		} else if (p.is_option("-W", &arg)) { // Min. length of words
			                              // to check.
			istringstream str(arg);
			str >> legal_word_length_;

		} else if (p.is_option("-T", &arg)) { // Assume input format
			                              // for all files
			if (strcmp(arg, "plain") == 0)
				default_filter = plain;
			else if (strcmp(arg, "nroff") == 0)
				default_filter = nroff;
			else if (strcmp(arg, "tex") == 0)
				default_filter = tex;
			else if (strcmp(arg, "sgml") == 0 ||
				 strcmp(arg, "html") == 0)
				default_filter = sgml;

			next_filter = default_filter;
			default_filter_set = true;

		} else if (p.is_option("-r", &arg)) { // Set input charset

			user_encoding_ = arg;

		} else if (p.is_option()) { // An unknown option
			print_usage();
			throw Error(_("Unknown option %s"), p.opt());

		} else { // A file
			if (!next_filter_set)
				next_filter = guess_file_filter(p.opt());

			files_.push_back(
				pair<string, FilterType>(p.opt(),
							 next_filter));
			next_filter = default_filter;
			next_filter_set = default_filter_set;
		}

		p.push_to(ispell_args_);
	}

	// Set the personal dictionary according to dictionary,
	// if the user hasn't set it.
	if (personal_dictionary_.empty()) {
		personal_dictionary_ = 
			get_home_dir() + "/.ispell_" + dictionary_identifier_;
	}

	if (default_filter_set)
		default_filter_ = default_filter;
	else if (next_filter_set)
		default_filter_ = next_filter;
}
