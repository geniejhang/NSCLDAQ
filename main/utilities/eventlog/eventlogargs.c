/*
  File autogenerated by gengetopt version 2.22.6
  generated with the following command:
  /usr/bin/gengetopt --file=eventlogargs --set-version=11.4-028 

  The developers of gengetopt consider the fixed text that goes in all
  gengetopt output files to be in the public domain:
  we make no copyright claims on it.
*/

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIX_UNUSED
#define FIX_UNUSED(X) (void) (X) /* avoid warnings for unused params */
#endif

#include <getopt.h>

#include "eventlogargs.h"

const char *gengetopt_args_info_purpose = "Log event data to file";

const char *gengetopt_args_info_usage = "Usage: EventLog [OPTIONS]...";

const char *gengetopt_args_info_versiontext = "";

const char *gengetopt_args_info_description = "";

const char *gengetopt_args_info_help[] = {
  "  -h, --help                   Print help and exit",
  "  -V, --version                Print version and exit",
  "  -s, --source=STRING          URL of source ring buffer",
  "  -p, --path=STRING            Directory in which event files are made",
  "  -S, --segmentsize=STRING     Size of event segments e.g. 2g or 2000m",
  "  -o, --oneshot                Record one run and exit, making synchronization\n                                 files",
  "  -n, --number-of-sources=INT  Number of data sources being built\n                                 (default=`1')",
  "  -r, --run=INT                Run number : Overrides run state information\n                                 ring items",
  "  -c, --checksum               If present, in addition to run files, checksum\n                                 files are produced  (default=off)",
  "  -C, --combine-runs           If present, changes in run number in one-shot\n                                 mode don't cause exit  (default=off)",
  "  -f, --prefix=STRING          Specifies the prefix to use for the output file\n                                 name",
    0
};

typedef enum {ARG_NO
  , ARG_FLAG
  , ARG_STRING
  , ARG_INT
} cmdline_parser_arg_type;

static
void clear_given (struct gengetopt_args_info *args_info);
static
void clear_args (struct gengetopt_args_info *args_info);

static int
cmdline_parser_internal (int argc, char **argv, struct gengetopt_args_info *args_info,
                        struct cmdline_parser_params *params, const char *additional_error);


static char *
gengetopt_strdup (const char *s);

static
void clear_given (struct gengetopt_args_info *args_info)
{
  args_info->help_given = 0 ;
  args_info->version_given = 0 ;
  args_info->source_given = 0 ;
  args_info->path_given = 0 ;
  args_info->segmentsize_given = 0 ;
  args_info->oneshot_given = 0 ;
  args_info->number_of_sources_given = 0 ;
  args_info->run_given = 0 ;
  args_info->checksum_given = 0 ;
  args_info->combine_runs_given = 0 ;
  args_info->prefix_given = 0 ;
}

static
void clear_args (struct gengetopt_args_info *args_info)
{
  FIX_UNUSED (args_info);
  args_info->source_arg = NULL;
  args_info->source_orig = NULL;
  args_info->path_arg = NULL;
  args_info->path_orig = NULL;
  args_info->segmentsize_arg = NULL;
  args_info->segmentsize_orig = NULL;
  args_info->number_of_sources_arg = 1;
  args_info->number_of_sources_orig = NULL;
  args_info->run_orig = NULL;
  args_info->checksum_flag = 0;
  args_info->combine_runs_flag = 0;
  args_info->prefix_arg = NULL;
  args_info->prefix_orig = NULL;
  
}

static
void init_args_info(struct gengetopt_args_info *args_info)
{


  args_info->help_help = gengetopt_args_info_help[0] ;
  args_info->version_help = gengetopt_args_info_help[1] ;
  args_info->source_help = gengetopt_args_info_help[2] ;
  args_info->path_help = gengetopt_args_info_help[3] ;
  args_info->segmentsize_help = gengetopt_args_info_help[4] ;
  args_info->oneshot_help = gengetopt_args_info_help[5] ;
  args_info->number_of_sources_help = gengetopt_args_info_help[6] ;
  args_info->run_help = gengetopt_args_info_help[7] ;
  args_info->checksum_help = gengetopt_args_info_help[8] ;
  args_info->combine_runs_help = gengetopt_args_info_help[9] ;
  args_info->prefix_help = gengetopt_args_info_help[10] ;
  
}

void
cmdline_parser_print_version (void)
{
  printf ("%s %s\n",
     (strlen(CMDLINE_PARSER_PACKAGE_NAME) ? CMDLINE_PARSER_PACKAGE_NAME : CMDLINE_PARSER_PACKAGE),
     CMDLINE_PARSER_VERSION);

  if (strlen(gengetopt_args_info_versiontext) > 0)
    printf("\n%s\n", gengetopt_args_info_versiontext);
}

static void print_help_common(void) {
  cmdline_parser_print_version ();

  if (strlen(gengetopt_args_info_purpose) > 0)
    printf("\n%s\n", gengetopt_args_info_purpose);

  if (strlen(gengetopt_args_info_usage) > 0)
    printf("\n%s\n", gengetopt_args_info_usage);

  printf("\n");

  if (strlen(gengetopt_args_info_description) > 0)
    printf("%s\n\n", gengetopt_args_info_description);
}

void
cmdline_parser_print_help (void)
{
  int i = 0;
  print_help_common();
  while (gengetopt_args_info_help[i])
    printf("%s\n", gengetopt_args_info_help[i++]);
}

void
cmdline_parser_init (struct gengetopt_args_info *args_info)
{
  clear_given (args_info);
  clear_args (args_info);
  init_args_info (args_info);
}

void
cmdline_parser_params_init(struct cmdline_parser_params *params)
{
  if (params)
    { 
      params->override = 0;
      params->initialize = 1;
      params->check_required = 1;
      params->check_ambiguity = 0;
      params->print_errors = 1;
    }
}

struct cmdline_parser_params *
cmdline_parser_params_create(void)
{
  struct cmdline_parser_params *params = 
    (struct cmdline_parser_params *)malloc(sizeof(struct cmdline_parser_params));
  cmdline_parser_params_init(params);  
  return params;
}

static void
free_string_field (char **s)
{
  if (*s)
    {
      free (*s);
      *s = 0;
    }
}


static void
cmdline_parser_release (struct gengetopt_args_info *args_info)
{

  free_string_field (&(args_info->source_arg));
  free_string_field (&(args_info->source_orig));
  free_string_field (&(args_info->path_arg));
  free_string_field (&(args_info->path_orig));
  free_string_field (&(args_info->segmentsize_arg));
  free_string_field (&(args_info->segmentsize_orig));
  free_string_field (&(args_info->number_of_sources_orig));
  free_string_field (&(args_info->run_orig));
  free_string_field (&(args_info->prefix_arg));
  free_string_field (&(args_info->prefix_orig));
  
  

  clear_given (args_info);
}


static void
write_into_file(FILE *outfile, const char *opt, const char *arg, const char *values[])
{
  FIX_UNUSED (values);
  if (arg) {
    fprintf(outfile, "%s=\"%s\"\n", opt, arg);
  } else {
    fprintf(outfile, "%s\n", opt);
  }
}


int
cmdline_parser_dump(FILE *outfile, struct gengetopt_args_info *args_info)
{
  int i = 0;

  if (!outfile)
    {
      fprintf (stderr, "%s: cannot dump options to stream\n", CMDLINE_PARSER_PACKAGE);
      return EXIT_FAILURE;
    }

  if (args_info->help_given)
    write_into_file(outfile, "help", 0, 0 );
  if (args_info->version_given)
    write_into_file(outfile, "version", 0, 0 );
  if (args_info->source_given)
    write_into_file(outfile, "source", args_info->source_orig, 0);
  if (args_info->path_given)
    write_into_file(outfile, "path", args_info->path_orig, 0);
  if (args_info->segmentsize_given)
    write_into_file(outfile, "segmentsize", args_info->segmentsize_orig, 0);
  if (args_info->oneshot_given)
    write_into_file(outfile, "oneshot", 0, 0 );
  if (args_info->number_of_sources_given)
    write_into_file(outfile, "number-of-sources", args_info->number_of_sources_orig, 0);
  if (args_info->run_given)
    write_into_file(outfile, "run", args_info->run_orig, 0);
  if (args_info->checksum_given)
    write_into_file(outfile, "checksum", 0, 0 );
  if (args_info->combine_runs_given)
    write_into_file(outfile, "combine-runs", 0, 0 );
  if (args_info->prefix_given)
    write_into_file(outfile, "prefix", args_info->prefix_orig, 0);
  

  i = EXIT_SUCCESS;
  return i;
}

int
cmdline_parser_file_save(const char *filename, struct gengetopt_args_info *args_info)
{
  FILE *outfile;
  int i = 0;

  outfile = fopen(filename, "w");

  if (!outfile)
    {
      fprintf (stderr, "%s: cannot open file for writing: %s\n", CMDLINE_PARSER_PACKAGE, filename);
      return EXIT_FAILURE;
    }

  i = cmdline_parser_dump(outfile, args_info);
  fclose (outfile);

  return i;
}

void
cmdline_parser_free (struct gengetopt_args_info *args_info)
{
  cmdline_parser_release (args_info);
}

/** @brief replacement of strdup, which is not standard */
char *
gengetopt_strdup (const char *s)
{
  char *result = 0;
  if (!s)
    return result;

  result = (char*)malloc(strlen(s) + 1);
  if (result == (char*)0)
    return (char*)0;
  strcpy(result, s);
  return result;
}

int
cmdline_parser (int argc, char **argv, struct gengetopt_args_info *args_info)
{
  return cmdline_parser2 (argc, argv, args_info, 0, 1, 1);
}

int
cmdline_parser_ext (int argc, char **argv, struct gengetopt_args_info *args_info,
                   struct cmdline_parser_params *params)
{
  int result;
  result = cmdline_parser_internal (argc, argv, args_info, params, 0);

  if (result == EXIT_FAILURE)
    {
      cmdline_parser_free (args_info);
      exit (EXIT_FAILURE);
    }
  
  return result;
}

int
cmdline_parser2 (int argc, char **argv, struct gengetopt_args_info *args_info, int override, int initialize, int check_required)
{
  int result;
  struct cmdline_parser_params params;
  
  params.override = override;
  params.initialize = initialize;
  params.check_required = check_required;
  params.check_ambiguity = 0;
  params.print_errors = 1;

  result = cmdline_parser_internal (argc, argv, args_info, &params, 0);

  if (result == EXIT_FAILURE)
    {
      cmdline_parser_free (args_info);
      exit (EXIT_FAILURE);
    }
  
  return result;
}

int
cmdline_parser_required (struct gengetopt_args_info *args_info, const char *prog_name)
{
  FIX_UNUSED (args_info);
  FIX_UNUSED (prog_name);
  return EXIT_SUCCESS;
}


static char *package_name = 0;

/**
 * @brief updates an option
 * @param field the generic pointer to the field to update
 * @param orig_field the pointer to the orig field
 * @param field_given the pointer to the number of occurrence of this option
 * @param prev_given the pointer to the number of occurrence already seen
 * @param value the argument for this option (if null no arg was specified)
 * @param possible_values the possible values for this option (if specified)
 * @param default_value the default value (in case the option only accepts fixed values)
 * @param arg_type the type of this option
 * @param check_ambiguity @see cmdline_parser_params.check_ambiguity
 * @param override @see cmdline_parser_params.override
 * @param no_free whether to free a possible previous value
 * @param multiple_option whether this is a multiple option
 * @param long_opt the corresponding long option
 * @param short_opt the corresponding short option (or '-' if none)
 * @param additional_error possible further error specification
 */
static
int update_arg(void *field, char **orig_field,
               unsigned int *field_given, unsigned int *prev_given, 
               char *value, const char *possible_values[],
               const char *default_value,
               cmdline_parser_arg_type arg_type,
               int check_ambiguity, int override,
               int no_free, int multiple_option,
               const char *long_opt, char short_opt,
               const char *additional_error)
{
  char *stop_char = 0;
  const char *val = value;
  int found;
  char **string_field;
  FIX_UNUSED (field);

  stop_char = 0;
  found = 0;

  if (!multiple_option && prev_given && (*prev_given || (check_ambiguity && *field_given)))
    {
      if (short_opt != '-')
        fprintf (stderr, "%s: `--%s' (`-%c') option given more than once%s\n", 
               package_name, long_opt, short_opt,
               (additional_error ? additional_error : ""));
      else
        fprintf (stderr, "%s: `--%s' option given more than once%s\n", 
               package_name, long_opt,
               (additional_error ? additional_error : ""));
      return 1; /* failure */
    }

  FIX_UNUSED (default_value);
    
  if (field_given && *field_given && ! override)
    return 0;
  if (prev_given)
    (*prev_given)++;
  if (field_given)
    (*field_given)++;
  if (possible_values)
    val = possible_values[found];

  switch(arg_type) {
  case ARG_FLAG:
    *((int *)field) = !*((int *)field);
    break;
  case ARG_INT:
    if (val) *((int *)field) = strtol (val, &stop_char, 0);
    break;
  case ARG_STRING:
    if (val) {
      string_field = (char **)field;
      if (!no_free && *string_field)
        free (*string_field); /* free previous string */
      *string_field = gengetopt_strdup (val);
    }
    break;
  default:
    break;
  };

  /* check numeric conversion */
  switch(arg_type) {
  case ARG_INT:
    if (val && !(stop_char && *stop_char == '\0')) {
      fprintf(stderr, "%s: invalid numeric value: %s\n", package_name, val);
      return 1; /* failure */
    }
    break;
  default:
    ;
  };

  /* store the original value */
  switch(arg_type) {
  case ARG_NO:
  case ARG_FLAG:
    break;
  default:
    if (value && orig_field) {
      if (no_free) {
        *orig_field = value;
      } else {
        if (*orig_field)
          free (*orig_field); /* free previous string */
        *orig_field = gengetopt_strdup (value);
      }
    }
  };

  return 0; /* OK */
}


int
cmdline_parser_internal (
  int argc, char **argv, struct gengetopt_args_info *args_info,
                        struct cmdline_parser_params *params, const char *additional_error)
{
  int c;	/* Character of the parsed option.  */

  int error_occurred = 0;
  struct gengetopt_args_info local_args_info;
  
  int override;
  int initialize;
  int check_required;
  int check_ambiguity;
  
  package_name = argv[0];
  
  override = params->override;
  initialize = params->initialize;
  check_required = params->check_required;
  check_ambiguity = params->check_ambiguity;

  if (initialize)
    cmdline_parser_init (args_info);

  cmdline_parser_init (&local_args_info);

  optarg = 0;
  optind = 0;
  opterr = params->print_errors;
  optopt = '?';

  while (1)
    {
      int option_index = 0;

      static struct option long_options[] = {
        { "help",	0, NULL, 'h' },
        { "version",	0, NULL, 'V' },
        { "source",	1, NULL, 's' },
        { "path",	1, NULL, 'p' },
        { "segmentsize",	1, NULL, 'S' },
        { "oneshot",	0, NULL, 'o' },
        { "number-of-sources",	1, NULL, 'n' },
        { "run",	1, NULL, 'r' },
        { "checksum",	0, NULL, 'c' },
        { "combine-runs",	0, NULL, 'C' },
        { "prefix",	1, NULL, 'f' },
        { 0,  0, 0, 0 }
      };

      c = getopt_long (argc, argv, "hVs:p:S:on:r:cCf:", long_options, &option_index);

      if (c == -1) break;	/* Exit from `while (1)' loop.  */

      switch (c)
        {
        case 'h':	/* Print help and exit.  */
          cmdline_parser_print_help ();
          cmdline_parser_free (&local_args_info);
          exit (EXIT_SUCCESS);

        case 'V':	/* Print version and exit.  */
          cmdline_parser_print_version ();
          cmdline_parser_free (&local_args_info);
          exit (EXIT_SUCCESS);

        case 's':	/* URL of source ring buffer.  */
        
        
          if (update_arg( (void *)&(args_info->source_arg), 
               &(args_info->source_orig), &(args_info->source_given),
              &(local_args_info.source_given), optarg, 0, 0, ARG_STRING,
              check_ambiguity, override, 0, 0,
              "source", 's',
              additional_error))
            goto failure;
        
          break;
        case 'p':	/* Directory in which event files are made.  */
        
        
          if (update_arg( (void *)&(args_info->path_arg), 
               &(args_info->path_orig), &(args_info->path_given),
              &(local_args_info.path_given), optarg, 0, 0, ARG_STRING,
              check_ambiguity, override, 0, 0,
              "path", 'p',
              additional_error))
            goto failure;
        
          break;
        case 'S':	/* Size of event segments e.g. 2g or 2000m.  */
        
        
          if (update_arg( (void *)&(args_info->segmentsize_arg), 
               &(args_info->segmentsize_orig), &(args_info->segmentsize_given),
              &(local_args_info.segmentsize_given), optarg, 0, 0, ARG_STRING,
              check_ambiguity, override, 0, 0,
              "segmentsize", 'S',
              additional_error))
            goto failure;
        
          break;
        case 'o':	/* Record one run and exit, making synchronization files.  */
        
        
          if (update_arg( 0 , 
               0 , &(args_info->oneshot_given),
              &(local_args_info.oneshot_given), optarg, 0, 0, ARG_NO,
              check_ambiguity, override, 0, 0,
              "oneshot", 'o',
              additional_error))
            goto failure;
        
          break;
        case 'n':	/* Number of data sources being built.  */
        
        
          if (update_arg( (void *)&(args_info->number_of_sources_arg), 
               &(args_info->number_of_sources_orig), &(args_info->number_of_sources_given),
              &(local_args_info.number_of_sources_given), optarg, 0, "1", ARG_INT,
              check_ambiguity, override, 0, 0,
              "number-of-sources", 'n',
              additional_error))
            goto failure;
        
          break;
        case 'r':	/* Run number : Overrides run state information ring items.  */
        
        
          if (update_arg( (void *)&(args_info->run_arg), 
               &(args_info->run_orig), &(args_info->run_given),
              &(local_args_info.run_given), optarg, 0, 0, ARG_INT,
              check_ambiguity, override, 0, 0,
              "run", 'r',
              additional_error))
            goto failure;
        
          break;
        case 'c':	/* If present, in addition to run files, checksum files are produced.  */
        
        
          if (update_arg((void *)&(args_info->checksum_flag), 0, &(args_info->checksum_given),
              &(local_args_info.checksum_given), optarg, 0, 0, ARG_FLAG,
              check_ambiguity, override, 1, 0, "checksum", 'c',
              additional_error))
            goto failure;
        
          break;
        case 'C':	/* If present, changes in run number in one-shot mode don't cause exit.  */
        
        
          if (update_arg((void *)&(args_info->combine_runs_flag), 0, &(args_info->combine_runs_given),
              &(local_args_info.combine_runs_given), optarg, 0, 0, ARG_FLAG,
              check_ambiguity, override, 1, 0, "combine-runs", 'C',
              additional_error))
            goto failure;
        
          break;
        case 'f':	/* Specifies the prefix to use for the output file name.  */
        
        
          if (update_arg( (void *)&(args_info->prefix_arg), 
               &(args_info->prefix_orig), &(args_info->prefix_given),
              &(local_args_info.prefix_given), optarg, 0, 0, ARG_STRING,
              check_ambiguity, override, 0, 0,
              "prefix", 'f',
              additional_error))
            goto failure;
        
          break;

        case 0:	/* Long option with no short option */
        case '?':	/* Invalid option.  */
          /* `getopt_long' already printed an error message.  */
          goto failure;

        default:	/* bug: option not considered.  */
          fprintf (stderr, "%s: option unknown: %c%s\n", CMDLINE_PARSER_PACKAGE, c, (additional_error ? additional_error : ""));
          abort ();
        } /* switch */
    } /* while */




  cmdline_parser_release (&local_args_info);

  if ( error_occurred )
    return (EXIT_FAILURE);

  return 0;

failure:
  
  cmdline_parser_release (&local_args_info);
  return (EXIT_FAILURE);
}
