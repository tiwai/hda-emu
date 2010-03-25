/*
 * hda-emu - simple HD-audio codec emulator for debugging snd-hda-intel driver
 *
 * Logging stuff
 *
 * Copyright (c) Takashi Iwai <tiwai@suse.de>
 *
 *  This driver is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This driver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "hda-types.h"
#include "hda-log.h"

static int log_level = HDA_LOG_VERB;
static FILE *logfp;
static int log_flags = HDA_LOG_FLAG_COLOR;

int hda_log_assert_on_error;

static void set_color(int level)
{
	static char *color_seq[] = {
		[HDA_LOG_ERR] = "31;1", /* bold red */
		[HDA_LOG_WARN] = "31", /* red */
		[HDA_LOG_KERN] = "32", /* green */
		[HDA_LOG_INFO] = "9", /* normal */
		[HDA_LOG_VERB] = "34", /* blue */
	};
	if (!(log_flags & HDA_LOG_FLAG_COLOR))
		return;
	if (level < 0)
		level = 0;
	else if (level > HDA_LOG_VERB)
		level = HDA_LOG_VERB;
	printf("\x1b[%sm", color_seq[level]);
}

static void reset_color(void)
{
	if (!(log_flags & HDA_LOG_FLAG_COLOR))
		return;
	printf("\x1b[0m");
}

static void _hda_log(int level, const char *fmt, va_list ap)
{
	va_list ap2;

	if (level > log_level)
		return;

	if (logfp == stdout)
		set_color(level);
	va_copy(ap2, ap);
	vfprintf(logfp, fmt, ap);
	if (!(log_flags & HDA_LOG_FLAG_NO_ECHO) && logfp != stdout)
		vprintf(fmt, ap2);
	va_end(ap2);
	if (logfp == stdout)
		reset_color();
	if (level == HDA_LOG_ERR && hda_log_assert_on_error)
		assert(0);
}

void hda_log(int level, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	_hda_log(level, fmt, ap);
	va_end(ap);
}

void hda_log_printk(const char *fmt, ...)
{
	va_list ap;
	int level = 7;

	va_start(ap, fmt);
	if (fmt[0] == '<' && isdigit(fmt[1]) && fmt[2] == '>') {
		level = fmt[1] - '0';
		fmt += 3;
	}
	if (level >= 5)
		level = HDA_LOG_KERN;
	else if (level == 4)
		level = HDA_LOG_WARN;
	else
		level = HDA_LOG_ERR;
	_hda_log(level, fmt, ap);
	va_end(ap);
}

void hda_log_echo(int level, const char *fmt, ...)
{
	va_list ap;

	if ((log_flags & HDA_LOG_FLAG_NO_ECHO) || logfp == stdout ||
	    level > log_level)
		return;
	va_start(ap, fmt);
	vfprintf(logfp, fmt, ap);
	va_end(ap);
	if (level == HDA_LOG_ERR && hda_log_assert_on_error)
		assert(0);
}

int hda_log_init(const char *file, unsigned int flags)
{
	if (!file || !strcmp(file, "-") || !strcmp(file, "stdout"))
		logfp = stdout;
	else {
		logfp = fopen(file, "w");
		if (!logfp) {
			fprintf(stderr, "cannot open log file %s\n", file);
			exit(1);
		}
	}

	log_flags = flags;
	return 0;
}

int hda_log_level_set(int level)
{
	int saved = log_level;
	log_level = level;
	return saved;
}

FILE *hda_get_logfp(void)
{
	return logfp;
}
