/*
 * Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 */

/** \file
    This is the command-line program to invoke the debit parser, glib
    version.

    @todo Switch to the new paradigm of using separate actions in
    debit_file, i.e., first ask for the bitstream to be parsed, then,
    according to flags, perform some actions. This will allow less
    clutter and hooks in the debit parser, will improve readability
    of the file, and will clarify the API. Pipdum already is an instance
    of this.

    \todo wrap mapping of file &al into a high-level function of the
    parser API

    \todo reorganize the data to seprate the bitstream data proper and
    the parsing structures. Some elements must be freed after parsing,
    but the data must not.
*/

#include <glib.h>

#include "bitstream_parser.h"
#include "debitlog.h"
#include "filedump.h"
#include "localpips.h"

static gboolean framedump = FALSE;
static gboolean pipdump = FALSE;

static gchar *ifile = NULL;
static gchar *odir = NULL;
static gchar *datadir = DATADIR;

#if DEBIT_DEBUG > 0
unsigned int debit_debug = 0;
#endif

static int
debit_file(gchar *input_file, gchar *output_dir) {
  gint err = 0;
  bitstream_parsed_t *bit;

  bit = parse_bitstream(input_file);

  if (bit == NULL) {
    err = -1;
    goto out;
  }

  /* Have some action */
  if (framedump)
    design_write_frames(bit,NULL);

  if (pipdump) {
    pip_db_t *pipdb = get_pipdb(datadir);
    if (pipdb != NULL) {
      dump_all_pips(pipdb, bit);
      free_pipdb(pipdb);
    }
  }

  free_bitstream(bit);
 out:
  return err;
}

static GOptionEntry entries[] =
{
  {"input", 'i', 0, G_OPTION_ARG_FILENAME, &ifile, "Read bitstream <ifile>", "<ifile>"},
#if DEBIT_DEBUG > 0
  {"debug", 'g', 0, G_OPTION_ARG_INT, &debit_debug, "Debug verbosity", NULL},
#endif
  {"outdir", 'o', 0, G_OPTION_ARG_FILENAME, &odir, "Write data files in directory <odir>", "<odir>"},
  {"datadir", 'd', 0, G_OPTION_ARG_FILENAME, &datadir, "Read data files from directory <datadir>", "<datadir>"},
  {"framedump", 'f', 0, G_OPTION_ARG_NONE, &framedump, "Dump raw data frames", NULL},
  {"pipdump", 'p', 0, G_OPTION_ARG_NONE, &pipdump, "Dump pips in the bitstream", NULL},
  { NULL }
};

int
main(int argc, char *argv[])
{
  int err;
  GError *error = NULL;
  GOptionContext *context = NULL;

  context = g_option_context_new ("- dump xilinx bitstream data");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_parse (context, &argc, &argv, &error);
  if (error != NULL) {
    g_warning("parse error: %s",error->message);
    g_error_free (error);
    return -1;
  }

  g_option_context_free(context);

  if (!ifile) {
    g_warning("You must specify a bitfile, %s --help for help", argv[0]);
    return -1;
  }

  err = debit_file(ifile, odir);
  return err;
}