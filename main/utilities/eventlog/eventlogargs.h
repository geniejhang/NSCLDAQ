/** @file eventlogargs.h
 *  @brief The header file for the command line option parser
 *  generated by GNU Gengetopt version 2.22.6
 *  http://www.gnu.org/software/gengetopt.
 *  DO NOT modify this file, since it can be overwritten
 *  @author GNU Gengetopt by Lorenzo Bettini */

#ifndef EVENTLOGARGS_H
#define EVENTLOGARGS_H

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
#define CMDLINE_PARSER_PACKAGE "EventLog"
#endif

#ifndef CMDLINE_PARSER_PACKAGE_NAME
/** @brief the complete program name (used for help and version) */
#define CMDLINE_PARSER_PACKAGE_NAME "EventLog"
#endif

#ifndef CMDLINE_PARSER_VERSION
/** @brief the program version */
#define CMDLINE_PARSER_VERSION "11.4-029"
#endif

/** @brief Where the command line options are stored */
struct gengetopt_args_info
{
  const char *help_help; /**< @brief Print help and exit help description.  */
  const char *version_help; /**< @brief Print version and exit help description.  */
  char * source_arg;	/**< @brief URL of source ring buffer.  */
  char * source_orig;	/**< @brief URL of source ring buffer original value given at command line.  */
  const char *source_help; /**< @brief URL of source ring buffer help description.  */
  char * path_arg;	/**< @brief Directory in which event files are made.  */
  char * path_orig;	/**< @brief Directory in which event files are made original value given at command line.  */
  const char *path_help; /**< @brief Directory in which event files are made help description.  */
  char * segmentsize_arg;	/**< @brief Size of event segments e.g. 2g or 2000m.  */
  char * segmentsize_orig;	/**< @brief Size of event segments e.g. 2g or 2000m original value given at command line.  */
  const char *segmentsize_help; /**< @brief Size of event segments e.g. 2g or 2000m help description.  */
  const char *oneshot_help; /**< @brief Record one run and exit, making synchronization files help description.  */
  int number_of_sources_arg;	/**< @brief Number of data sources being built (default='1').  */
  char * number_of_sources_orig;	/**< @brief Number of data sources being built original value given at command line.  */
  const char *number_of_sources_help; /**< @brief Number of data sources being built help description.  */
  int run_arg;	/**< @brief Run number : Overrides run state information ring items.  */
  char * run_orig;	/**< @brief Run number : Overrides run state information ring items original value given at command line.  */
  const char *run_help; /**< @brief Run number : Overrides run state information ring items help description.  */
  int checksum_flag;	/**< @brief If present, in addition to run files, checksum files are produced (default=off).  */
  const char *checksum_help; /**< @brief If present, in addition to run files, checksum files are produced help description.  */
  int combine_runs_flag;	/**< @brief If present, changes in run number in one-shot mode don't cause exit (default=off).  */
  const char *combine_runs_help; /**< @brief If present, changes in run number in one-shot mode don't cause exit help description.  */
  char * prefix_arg;	/**< @brief Specifies the prefix to use for the output file name.  */
  char * prefix_orig;	/**< @brief Specifies the prefix to use for the output file name original value given at command line.  */
  const char *prefix_help; /**< @brief Specifies the prefix to use for the output file name help description.  */
  
  unsigned int help_given ;	/**< @brief Whether help was given.  */
  unsigned int version_given ;	/**< @brief Whether version was given.  */
  unsigned int source_given ;	/**< @brief Whether source was given.  */
  unsigned int path_given ;	/**< @brief Whether path was given.  */
  unsigned int segmentsize_given ;	/**< @brief Whether segmentsize was given.  */
  unsigned int oneshot_given ;	/**< @brief Whether oneshot was given.  */
  unsigned int number_of_sources_given ;	/**< @brief Whether number-of-sources was given.  */
  unsigned int run_given ;	/**< @brief Whether run was given.  */
  unsigned int checksum_given ;	/**< @brief Whether checksum was given.  */
  unsigned int combine_runs_given ;	/**< @brief Whether combine-runs was given.  */
  unsigned int prefix_given ;	/**< @brief Whether prefix was given.  */

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
#endif /* EVENTLOGARGS_H */
