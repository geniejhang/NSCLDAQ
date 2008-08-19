/* sclclientargs.h */

/* File autogenerated by gengetopt version 2.18  */

#ifndef SCLCLIENTARGS_H
#define SCLCLIENTARGS_H

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CMDLINE_PARSER_PACKAGE
#define CMDLINE_PARSER_PACKAGE "SclClient"
#endif

#ifndef CMDLINE_PARSER_VERSION
#define CMDLINE_PARSER_VERSION "1.0"
#endif

struct gengetopt_args_info
{
  const char *help_help; /* Print help and exit help description.  */
  const char *version_help; /* Print version and exit help description.  */
  char * source_arg;	/* URL of the source ring buffer.  */
  char * source_orig;	/* URL of the source ring buffer original value given at command line.  */
  const char *source_help; /* URL of the source ring buffer help description.  */
  char * host_arg;	/* Name of host in which TclServer is running.  */
  char * host_orig;	/* Name of host in which TclServer is running original value given at command line.  */
  const char *host_help; /* Name of host in which TclServer is running help description.  */
  char * port_arg;	/* Port on which to connect to tclserver.  */
  char * port_orig;	/* Port on which to connect to tclserver original value given at command line.  */
  const char *port_help; /* Port on which to connect to tclserver help description.  */
  
  int help_given ;	/* Whether help was given.  */
  int version_given ;	/* Whether version was given.  */
  int source_given ;	/* Whether source was given.  */
  int host_given ;	/* Whether host was given.  */
  int port_given ;	/* Whether port was given.  */

} ;

extern const char *gengetopt_args_info_purpose;
extern const char *gengetopt_args_info_usage;
extern const char *gengetopt_args_info_help[];

int cmdline_parser (int argc, char * const *argv,
  struct gengetopt_args_info *args_info);
int cmdline_parser2 (int argc, char * const *argv,
  struct gengetopt_args_info *args_info,
  int override, int initialize, int check_required);
int cmdline_parser_file_save(const char *filename,
  struct gengetopt_args_info *args_info);

void cmdline_parser_print_help(void);
void cmdline_parser_print_version(void);

void cmdline_parser_init (struct gengetopt_args_info *args_info);
void cmdline_parser_free (struct gengetopt_args_info *args_info);

int cmdline_parser_required (struct gengetopt_args_info *args_info,
  const char *prog_name);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* SCLCLIENTARGS_H */
