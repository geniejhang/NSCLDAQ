/** @file rfcmdline.h
 *  @brief The header file for the command line option parser
 *  generated by GNU Gengetopt version 2.22.6
 *  http://www.gnu.org/software/gengetopt.
 *  DO NOT modify this file, since it can be overwritten
 *  @author GNU Gengetopt by Lorenzo Bettini */

#ifndef RFCMDLINE_H
#define RFCMDLINE_H

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h> /* for FILE */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CMDLINE_PARSER_PACKAGE
/** @brief the program name (used for printing errors) */
#define CMDLINE_PARSER_PACKAGE "evbclient"
#endif

#ifndef CMDLINE_PARSER_PACKAGE_NAME
/** @brief the complete program name (used for help and version) */
#define CMDLINE_PARSER_PACKAGE_NAME "evbclient"
#endif

#ifndef CMDLINE_PARSER_VERSION
/** @brief the program version */
#define CMDLINE_PARSER_VERSION "1.0"
#endif

/** @brief Where the command line options are stored */
struct gengetopt_args_info
{
  const char *help_help; /**< @brief Print help and exit help description.  */
  const char *version_help; /**< @brief Print version and exit help description.  */
  char * evbhost_arg;	/**< @brief Event builder host name.  */
  char * evbhost_orig;	/**< @brief Event builder host name original value given at command line.  */
  const char *evbhost_help; /**< @brief Event builder host name help description.  */
  char * evbport_arg;	/**< @brief Event builder port (default='managed').  */
  char * evbport_orig;	/**< @brief Event builder port original value given at command line.  */
  const char *evbport_help; /**< @brief Event builder port help description.  */
  char * evbname_arg;	/**< @brief Event builder name.  */
  char * evbname_orig;	/**< @brief Event builder name original value given at command line.  */
  const char *evbname_help; /**< @brief Event builder name help description.  */
  char * info_arg;	/**< @brief Description string when connecting to server.  */
  char * info_orig;	/**< @brief Description string when connecting to server original value given at command line.  */
  const char *info_help; /**< @brief Description string when connecting to server help description.  */
  int* ids_arg;	/**< @brief Source ids emitted by this client.  */
  char ** ids_orig;	/**< @brief Source ids emitted by this client original value given at command line.  */
  unsigned int ids_min; /**< @brief Source ids emitted by this client's minimum occurreces */
  unsigned int ids_max; /**< @brief Source ids emitted by this client's maximum occurreces */
  const char *ids_help; /**< @brief Source ids emitted by this client help description.  */
  int default_id_arg;	/**< @brief Source id to assign data without a predefined source id.  */
  char * default_id_orig;	/**< @brief Source id to assign data without a predefined source id original value given at command line.  */
  const char *default_id_help; /**< @brief Source id to assign data without a predefined source id help description.  */
  char * ring_arg;	/**< @brief URL of ring from which fragments come.  */
  char * ring_orig;	/**< @brief URL of ring from which fragments come original value given at command line.  */
  const char *ring_help; /**< @brief URL of ring from which fragments come help description.  */
  char * timestampextractor_arg;	/**< @brief Shared library with timestamp extraction code.  */
  char * timestampextractor_orig;	/**< @brief Shared library with timestamp extraction code original value given at command line.  */
  const char *timestampextractor_help; /**< @brief Shared library with timestamp extraction code help description.  */
  int expectbodyheaders_flag;	/**< @brief Body headers are expected on every ring item. (default=off).  */
  const char *expectbodyheaders_help; /**< @brief Body headers are expected on every ring item. help description.  */
  int oneshot_arg;	/**< @brief One shot after n end run items (default='1').  */
  char * oneshot_orig;	/**< @brief One shot after n end run items original value given at command line.  */
  const char *oneshot_help; /**< @brief One shot after n end run items help description.  */
  int timeout_arg;	/**< @brief Timeout waiting for end runs in oneshot mode (default='10').  */
  char * timeout_orig;	/**< @brief Timeout waiting for end runs in oneshot mode original value given at command line.  */
  const char *timeout_help; /**< @brief Timeout waiting for end runs in oneshot mode help description.  */
  int offset_arg;	/**< @brief Signed time offset to add to the extracted timestamp (default='0').  */
  char * offset_orig;	/**< @brief Signed time offset to add to the extracted timestamp original value given at command line.  */
  const char *offset_help; /**< @brief Signed time offset to add to the extracted timestamp help description.  */
  
  unsigned int help_given ;	/**< @brief Whether help was given.  */
  unsigned int version_given ;	/**< @brief Whether version was given.  */
  unsigned int evbhost_given ;	/**< @brief Whether evbhost was given.  */
  unsigned int evbport_given ;	/**< @brief Whether evbport was given.  */
  unsigned int evbname_given ;	/**< @brief Whether evbname was given.  */
  unsigned int info_given ;	/**< @brief Whether info was given.  */
  unsigned int ids_given ;	/**< @brief Whether ids was given.  */
  unsigned int default_id_given ;	/**< @brief Whether default-id was given.  */
  unsigned int ring_given ;	/**< @brief Whether ring was given.  */
  unsigned int timestampextractor_given ;	/**< @brief Whether timestampextractor was given.  */
  unsigned int expectbodyheaders_given ;	/**< @brief Whether expectbodyheaders was given.  */
  unsigned int oneshot_given ;	/**< @brief Whether oneshot was given.  */
  unsigned int timeout_given ;	/**< @brief Whether timeout was given.  */
  unsigned int offset_given ;	/**< @brief Whether offset was given.  */

} ;

/** @brief The additional parameters to pass to parser functions */
struct cmdline_parser_params
{
  int override; /**< @brief whether to override possibly already present options (default 0) */
  int initialize; /**< @brief whether to initialize the option structure gengetopt_args_info (default 1) */
  int check_required; /**< @brief whether to check that all required options were provided (default 1) */
  int check_ambiguity; /**< @brief whether to check for options already specified in the option structure gengetopt_args_info (default 0) */
  int print_errors; /**< @brief whether getopt_long should print an error message for a bad option (default 1) */
} ;

/** @brief the purpose string of the program */
extern const char *gengetopt_args_info_purpose;
/** @brief the usage string of the program */
extern const char *gengetopt_args_info_usage;
/** @brief the description string of the program */
extern const char *gengetopt_args_info_description;
/** @brief all the lines making the help output */
extern const char *gengetopt_args_info_help[];

/**
 * The command line parser
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser (int argc, char **argv,
  struct gengetopt_args_info *args_info);

/**
 * The command line parser (version with additional parameters - deprecated)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param override whether to override possibly already present options
 * @param initialize whether to initialize the option structure my_args_info
 * @param check_required whether to check that all required options were provided
 * @return 0 if everything went fine, NON 0 if an error took place
 * @deprecated use cmdline_parser_ext() instead
 */
int cmdline_parser2 (int argc, char **argv,
  struct gengetopt_args_info *args_info,
  int override, int initialize, int check_required);

/**
 * The command line parser (version with additional parameters)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param params additional parameters for the parser
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_ext (int argc, char **argv,
  struct gengetopt_args_info *args_info,
  struct cmdline_parser_params *params);

/**
 * Save the contents of the option struct into an already open FILE stream.
 * @param outfile the stream where to dump options
 * @param args_info the option struct to dump
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_dump(FILE *outfile,
  struct gengetopt_args_info *args_info);

/**
 * Save the contents of the option struct into a (text) file.
 * This file can be read by the config file parser (if generated by gengetopt)
 * @param filename the file where to save
 * @param args_info the option struct to save
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_file_save(const char *filename,
  struct gengetopt_args_info *args_info);

/**
 * Print the help
 */
void cmdline_parser_print_help(void);
/**
 * Print the version
 */
void cmdline_parser_print_version(void);

/**
 * Initializes all the fields a cmdline_parser_params structure 
 * to their default values
 * @param params the structure to initialize
 */
void cmdline_parser_params_init(struct cmdline_parser_params *params);

/**
 * Allocates dynamically a cmdline_parser_params structure and initializes
 * all its fields to their default values
 * @return the created and initialized cmdline_parser_params structure
 */
struct cmdline_parser_params *cmdline_parser_params_create(void);

/**
 * Initializes the passed gengetopt_args_info structure's fields
 * (also set default values for options that have a default)
 * @param args_info the structure to initialize
 */
void cmdline_parser_init (struct gengetopt_args_info *args_info);
/**
 * Deallocates the string fields of the gengetopt_args_info structure
 * (but does not deallocate the structure itself)
 * @param args_info the structure to deallocate
 */
void cmdline_parser_free (struct gengetopt_args_info *args_info);

/**
 * Checks that all the required options were specified
 * @param args_info the structure to check
 * @param prog_name the name of the program that will be used to print
 *   possible errors
 * @return
 */
int cmdline_parser_required (struct gengetopt_args_info *args_info,
  const char *prog_name);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* RFCMDLINE_H */