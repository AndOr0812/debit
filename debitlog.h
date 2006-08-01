/*
  Copyright Jean-Baptiste Note <jean-baptiste.note@m4x.org>
*/

#ifndef _HAS_DEBIT_DEBUG_H
#define _HAS_DEBIT_DEBUG_H

#ifndef DEBIT_DEBUG
#define DEBIT_DEBUG 2
#endif

enum {
	/* generic debug */
	L_DEBUG = (DEBIT_DEBUG > 1) * 0x0001,
	/* function tracing */
	L_FUNC = (DEBIT_DEBUG > 0) * 0x0002,

	/* Bitstream control data */
	L_BITSTREAM = (DEBIT_DEBUG > 0) * 0x0004,
	/* Pip debug data */
	L_PIPS =  (DEBIT_DEBUG > 1) * 0x0008,
	/* Wires debug */
	L_WIRES = (DEBIT_DEBUG > 1) * 0x0010,
	/* Connexity algorithms */
	L_CONNEXITY = (DEBIT_DEBUG > 1) * 0x0020,
	L_ANY = 0xffff,
};

#if DEBIT_DEBUG > 0
extern unsigned int     debit_debug;
#else
enum { debit_debug = 0 };
#endif

#if DEBIT_DEBUG > 0

#define debit_log(chan, args...) \
	do { \
		if (debit_debug & (chan)) \
			g_warning(args); \
	} while (0)

#else

#define debit_log(chan, args...)

#endif

#endif /* _HAS_DEBIT_DEBUG_H */