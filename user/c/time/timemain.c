/**
 * $Id$
 * Copyright (C) 2008 - 2009 Nils Asmussen
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
#include <esc/cmdargs.h>
#include <esc/proc.h>
#include <stdio.h>
#include <esc/dir.h>
#include <signal.h>
#include <esc/conf.h>
#include <string.h>
#include <errors.h>

static void sigTimer(tSig sig,u32 data);
static void sigHdlr(tSig sig,u32 data);
static void usage(const char *name) {
	fprintf(stderr,"Usage: %s <program> [arguments...]\n",name);
	exit(EXIT_FAILURE);
}

static s32 timerFreq;
static u32 ms = 0;
static s32 waitingPid = 0;

int main(int argc,char **argv) {
	char path[MAX_PATH_LEN + 1] = "/bin/";
	if(argc < 2 || isHelpCmd(argc,argv))
		usage(argv[0]);

	timerFreq = getConf(CONF_TIMER_FREQ);
	if(timerFreq < 0)
		error("Unable to get timer-frequency");

	strcat(path,argv[1]);
	if(setSigHandler(SIG_INTRPT,sigHdlr) < 0)
		error("Unable to set sig-handler for signal %d",SIG_INTRPT);
	if(setSigHandler(SIG_INTRPT_TIMER,sigTimer) < 0)
		error("Unable to set sig-handler for signal %d",SIG_INTRPT_TIMER);

	debug();
	if((waitingPid = fork()) == 0) {
		s32 i;
		argv[0] = path;
		for(i = 1; i < argc - 1; i++)
			argv[i] = argv[i + 1];
		argv[argc - 1] = NULL;
		exec(argv[0],(const char**)argv);
		error("Exec failed");
	}
	else if(waitingPid < 0)
		error("Fork failed");
	else {
		sExitState state;
		s32 res;
		while(1) {
			res = waitChild(&state);
			if(res != ERR_INTERRUPTED)
				break;
		}
		if(res < 0)
			error("Wait failed");
		setSigHandler(SIG_INTRPT_TIMER,SIG_DFL);
		printf("\n");
		printf("Process %d (%s) terminated with exit-code %d\n",state.pid,path,state.exitCode);
		if(state.signal != SIG_COUNT)
			printf("It was terminated by signal %d\n",state.signal);
		printf("User-Cycles:	%08x%08x\n",state.ucycleCount.val32.upper,
				state.ucycleCount.val32.lower);
		printf("Kernel-Cycles:	%08x%08x\n",state.kcycleCount.val32.upper,
				state.kcycleCount.val32.lower);
		printf("Time:			%u ms\n",ms);
		printf("Own mem:		%u KiB\n",state.ownFrames * 4);
		printf("Shared mem:		%u KiB\n",state.sharedFrames * 4);
		printf("Swap mem:		%u KiB\n",state.swapped * 4);
	}
	debug();

	return EXIT_SUCCESS;
}

static void sigTimer(tSig sig,u32 data) {
	UNUSED(sig);
	UNUSED(data);
	ms += 1000 / timerFreq;
}

static void sigHdlr(tSig sig,u32 data) {
	UNUSED(sig);
	UNUSED(data);
	if(waitingPid > 0) {
		/* send SIG_INTRPT to the child */
		if(sendSignalTo(waitingPid,SIG_INTRPT,0) < 0)
			printe("Unable to send signal to %d",waitingPid);
	}
}
