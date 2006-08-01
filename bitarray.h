/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#ifndef _BITARRAY_H
#define _BITARRAY_H

#include <glib.h>
#include <inttypes.h>
#include <stdlib.h>

typedef struct bitarray {
  uint8_t *data; // data is written little-endian in there
  size_t len;
  off_t pos; // pos of next bit to read or to be replaced by a put
} bitarray_t;
//note to self: the cursor moves like fopen with "w+"

/* read-only byte-aligned array */
typedef struct bytearray {
  const gchar *data; // data is written little-endian in there
  size_t len;
  off_t pos; // pos of next bit to read or to be replaced by a put
} bytearray_t;

bytearray_t *new_bytearray_with_data(size_t len, const gchar *data);
gchar *delete_bytearray_keep_data(bytearray_t *ba);

static inline
void bytearray_init(bytearray_t *lba,
		    const size_t len, const size_t pos,
		    const gchar *data) {
  lba->data = data;
  lba->len = len;
  lba->pos = pos;
}

static inline
const gchar *bytearray_get_ptr(const bytearray_t *ba) {
  return &ba->data[ba->pos];
}

static inline
off_t bytearray_available(const bytearray_t *ba) {
  return ba->len - ba->pos;
}

static inline
gchar bytearray_get_uint8(bytearray_t *ba)
{
  g_assert(bytearray_available(ba) < 1);
  return ba->data[ba->pos++];
}

static inline
guint32 bytearray_peek_uint32(const bytearray_t *ba) {
  guint32 c;
  g_assert(bytearray_available(ba) >= sizeof(uint32_t));

  /** \todo We read as big endian for now,
      but later use standard conversion macros outside this */

  c = GUINT32_FROM_BE(*(guint32 *)&ba->data[ba->pos]);
  return c;
}

static inline
guint32 bytearray_get_uint32(bytearray_t *ba)
{
  guint32 c;
  c = bytearray_peek_uint32(ba);
  ba->pos += sizeof(c);
  return c;
}

#endif /* bitarray.h */