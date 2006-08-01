/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

/*
 * New, simple wiring db implementation
 */

#include <glib.h>
#include <string.h>
#include "wiring.h"

#define STRINGCHUNK_DEFAULT_SIZE 16

typedef enum
  {
    /* log flags */
    WIRE_LOG_DATA             = 1 << G_LOG_LEVEL_USER_SHIFT,
  } WireLevelFlags;

static gint
read_wiredb(GKeyFile **fill, const gchar *filename) {
  GKeyFile *db;
  GError *error = NULL;

  g_log(G_LOG_DOMAIN, WIRE_LOG_DATA, "Loading data from %s", filename);

  db = g_key_file_new();
  if (!db)
    goto out_err;

  g_key_file_load_from_file(db,filename,G_KEY_FILE_NONE,&error);

  if (error != NULL) {
    g_warning("could not read db %s: %s",filename,error->message);
    g_error_free (error);
    goto out_err_free;
  }

  *fill = db;
  return 0;

 out_err_free:
  g_key_file_free (db);
 out_err:
  return -1;
}

/*
 * Allocate a wire db
 */

static inline int
load_wire_atom(const wire_db_t *db, GKeyFile *keyfile,
	       const gchar *wirename)
{
  GError *err = NULL;
  gint id = g_key_file_get_integer(keyfile, wirename, "ID", &err);
  wire_simple_t *wire = &db->wires[id];
  wire_t *detail = &db->details[id];

  if (err)
    goto out_err;

  /* Insert the wirename */
  db->names[id] = g_string_chunk_insert_const(db->wirenames, wirename);

#define GET_STRUCT_MEMBER(structname, structmem, strname) \
do { structname->structmem = g_key_file_get_integer(keyfile, wirename, #strname, &err);\
  if (err)\
    goto out_err;\
} while (0)

  GET_STRUCT_MEMBER(wire, dx, DX);
  GET_STRUCT_MEMBER(wire, dy, DY);
  GET_STRUCT_MEMBER(wire, ep, EP);
  GET_STRUCT_MEMBER(detail, type, TYPE);
  GET_STRUCT_MEMBER(detail, direction, DIR);
  GET_STRUCT_MEMBER(detail, situation, SIT);

  return 0;

 out_err:
  g_warning("%s", err->message);
  return err->code;
}

static inline int
fill_db_from_file(const wire_db_t *wires, GKeyFile *db,
		  const gsize nwires, gchar **wirenames) {
  gint err = 0;
  gsize i;

  for(i = 0; i < nwires; i++) {
    err = load_wire_atom(wires, db, wirenames[i]);
    if (err)
      break;
  }

  return err;
}

/** Fill in a wire db with data from a file
 *
 * This function load from the db argument the wires argument
 *
 * @param db KeyFile containing data
 * @param wires wire database to fill
 * @return an error indication
 */
static int
load_db_from_file(GKeyFile* db, wire_db_t *wires) {
  gint err;
  gsize nwires;
  gchar** wirenames;

  wirenames = g_key_file_get_groups(db, &nwires);

  /* Allocate the array */
  wires->dblen = nwires;
  wires->wires = g_new(wire_simple_t, nwires);
  wires->names = g_new(const gchar *, nwires);
  wires->details = g_new(wire_t, nwires);
  wires->wirenames = g_string_chunk_new(STRINGCHUNK_DEFAULT_SIZE);

  /* Iterate over groups */
  err = fill_db_from_file(wires, db, nwires, wirenames);

  /* Cleanup */
  g_strfreev(wirenames);

  return err;
}


/*
 * High-level function
 */
wire_db_t *get_wiredb(const gchar *datadir) {
  wire_db_t *wiredb = g_new0(wire_db_t, 1);
  GKeyFile *db = NULL;
  gchar *dbname;
  gint err;

  dbname = g_build_filename(datadir,"wires.db",NULL);

  err = read_wiredb(&db, dbname);
  g_free(dbname);
  if (err)
    goto out_err;

  err = load_db_from_file(db, wiredb);
  if (err)
    goto out_err;

  g_key_file_free(db);
  return wiredb;

 out_err:
  g_warning("failed to readback wire db");
  if (db)
    g_key_file_free(db);
  g_free(wiredb);
  return NULL;
}


/*
 * Free a wire db
 */

void free_wiredb(wire_db_t *wires) {
  GStringChunk *wirenames = wires->wirenames;
  if (wirenames)
    g_string_chunk_free(wirenames);
  g_free(wires->details);
  g_free(wires->names);
  g_free(wires->wires);
  g_free(wires);
}

/*
 * Interface functions needed by wiring.h
 */

static inline gint cmp(const gchar *s1, const gchar *s2) {
  return strcmp(s1,s2);
}

/* This one is a get_by_name. The only complicated function, which
   is a dichotomy */

gint parse_wire_simple(const wire_db_t *db, wire_atom_t* res,
		       const gchar *wire) {
  guint low = 0, high = db->dblen-1;
  const gchar **names = db->names;

  //  g_assert(high <= db->dblen);

  do {
    guint middle = (high + low) / 2;
    gint comp = cmp(wire,names[middle]);
    if (!comp) {
      *res = middle;
      return 0;
    }
    if (comp > 0)
      low = middle + 1;
    else
      high = middle - 1;
  } while(low <= high);

  return -1;
}
