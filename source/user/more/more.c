/**
 * $Id$
 * Copyright (C) 2008 - 2014 Nils Asmussen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <esc/common.h>
#include <esc/driver/screen.h>
#include <esc/driver/vterm.h>
#include <esc/cmdargs.h>
#include <esc/esccodes.h>
#include <esc/keycodes.h>
#include <esc/messages.h>
#include <esc/dir.h>
#include <esc/io.h>
#include <stdio.h>
#include <stdlib.h>

static void waitForKeyPress(FILE *vt);
static void usage(const char *name) {
	fprintf(stderr,"Usage: %s [<file>]\n",name);
	exit(EXIT_FAILURE);
}

static bool run = true;

int main(int argc,const char *argv[]) {
	static char vtermPath[MAX_PATH_LEN];
	const char **args;
	FILE *vt,*in = stdin;
	size_t line,col;
	sScreenMode mode;
	int vtFd;
	int c;

	/* parse args */
	int res = ca_parse(argc,argv,CA_MAX1_FREE,"");
	if(res < 0) {
		fprintf(stderr,"Invalid arguments: %s\n",ca_error(res));
		usage(argv[0]);
	}
	if(ca_hasHelp())
		usage(argv[0]);

	/* open file, if any */
	args = ca_getFree();
	if(args[0]) {
		in = fopen(args[0],"r");
		if(!in)
			error("Unable to open '%s'",args[0]);
	}
	else if(isatty(STDIN_FILENO))
		error("If stdin is a terminal, you have to provide a file");

	/* open the "real" stdin, because stdin maybe redirected to something else */
	strnzcpy(vtermPath,getenv("TERM"),sizeof(vtermPath));
	vtermPath[sizeof(vtermPath) - 1] = '\0';
	vt = fopen(vtermPath,"rm");
	if(!vt)
		error("Unable to open '%s'",vtermPath);
	vtFd = fileno(vt);
	vterm_setReadline(vtFd,false);
	/* get vterm-size */
	if(screen_getMode(vtFd,&mode) < 0)
		error("Unable to get vterm-size");

	/* read until vterm is full, ask for continue, and so on */
	line = 0;
	col = 0;
	while(run && (c = fgetc(in)) != EOF) {
		putchar(c);
		col++;

		if(col == mode.cols - 1 || c == '\n') {
			line++;
			col = 0;
		}
		if(line == mode.rows - 1) {
			fflush(stdout);
			waitForKeyPress(vt);
			line = 0;
			col = 0;
		}
	}
	if(ferror(in))
		error("Read failed");

	/* clean up */
	vterm_setReadline(vtFd,true);
	if(args[0])
		fclose(in);
	fclose(vt);
	return EXIT_SUCCESS;
}

static void waitForKeyPress(FILE *vt) {
	int c;
	while((c = fgetc(vt)) != EOF) {
		if(c == '\033') {
			int n1,n2,n3;
			int cmd = freadesc(vt,&n1,&n2,&n3);
			if(cmd == ESCC_KEYCODE && !(n3 & STATE_BREAK)) {
				if(n2 == VK_SPACE)
					break;
				if(n2 == VK_Q) {
					run = false;
					break;
				}
			}
		}
	}
}