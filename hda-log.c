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
#include "hda-types.h"
#include "hda-log.h"

static int log_level = HDA_LOG_VERB;
static FILE *logfp;
static int no_log_echo;
static int log_color = 1;

static void set_color(int level)
{
	static char *color_seq[] = {
		[HDA_LOG_ERR] = "31;1", /* bold red */
		[HDA_LOG_KERN] = "32", /* green */
		[HDA_LOG_INFO] = "9", /* normal */
		[HDA_LOG_VERB] = "34", /* blue */
	};
	if (!log_color)
		return;
	if (level < 0)
		level = 0;
	else if (level > HDA_LOG_VERB)
		level = HDA_LOG_VERB;
	printf("\x1b[%sm", color_seq[level]);
}

static void reset_color(void)
{
	printf("\x1b[0m");
}

void hda_log(int level, const char *fmt, ...)
{
	va_list ap, ap2;

	if (level > log_level)
		return;

	if (logfp == stdout)
		set_color(level);
	va_start(ap, fmt);
	va_copy(ap2, ap);
	vfprintf(logfp, fmt, ap);
	if (!no_log_echo && logfp != stdout)
		vprintf(fmt, ap2);
	va_end(ap);
	if (logfp == stdout)
		reset_color();
}

void hda_log_echo(int level, const char *fmt, ...)
{
	va_list ap;

	if (no_log_echo || logfp == stdout || level > log_level)
		return;
	va_start(ap, fmt);
	vfprintf(logfp, fmt, ap);
	va_end(ap);
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

	if (flags & HDA_LOG_FLAG_NO_ECHO)
		no_log_echo = 1;
	if (flags & HDA_LOG_FLAG_COLOR)
		log_color = 1;
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
