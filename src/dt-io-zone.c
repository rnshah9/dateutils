/*** dt-io-zone.c -- abstract from raw zone interface
 *
 * Copyright (C) 2010-2014 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of dateutils.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the author nor the names of any contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***/
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <string.h>
#include "tzmap.h"
#include "dt-io.h"
#include "dt-io-zone.h"
#include "alist.h"

static size_t
xstrlncpy(char *restrict dst, ssize_t dsz, const char *src, ssize_t ssz)
{
	if (ssz > dsz) {
		ssz = dsz - 1U;
	}
	memcpy(dst, src, ssz);
	dst[ssz] = '\0';
	return ssz;
}


/* extended zone handling, tzmaps and stuff */
#if !defined PATH_MAX
# define PATH_MAX	256U
#endif	/* !PATH_MAX */

static struct alist_s zones[1U];
static struct alist_s tzmaps[1U];

static zif_t
__io_zone(const char *spec)
{
	zif_t res;

	/* try looking up SPEC first */
	if ((res = alist_assoc(zones, spec)) == NULL) {
		/* open 'im */
		if ((res = zif_open(spec)) != NULL) {
			/* cache 'im */
			alist_put(zones, spec, res);
		}
	}
	return res;
}

zif_t
dt_io_zone(const char *spec)
{
	static const char tzmap_suffix[] = ".tzmcc";
	char *p;

	/* see if SPEC is a MAP:KEY */
	if ((p = strchr(spec, ':')) != NULL) {
		char tzmfn[PATH_MAX];
		tzmap_t tzm;

		xstrlncpy(tzmfn, sizeof(tzmfn), spec, p - spec);

		/* check tzmaps alist first */
		if ((tzm = alist_assoc(tzmaps, tzmfn)) == NULL) {
			char *suf = tzmfn + (p - spec);

			/* try and find it the hard way */
			xstrlncpy(
				suf, sizeof(tzmfn) - (p - spec),
				tzmap_suffix, sizeof(tzmap_suffix) - 1U);

			/* try and open the thing, then try and look up SPEC */
			if ((tzm = tzm_open(tzmfn)) == NULL) {
				error(0, "\
Cannot find `%s' in the tzmaps search path\n", tzmfn);
				return NULL;
			}
			/* otherwise cache him */
			*suf = '\0';
			alist_put(tzmaps, tzmfn, tzm);
		}
		/* look up key bit in tzmap and use that if found */
		if ((spec = tzm_find(tzm, ++p)) == NULL) {
			return NULL;
		}
	}
	return __io_zone(spec);
}

/* dt-io-zone.c ends here */
