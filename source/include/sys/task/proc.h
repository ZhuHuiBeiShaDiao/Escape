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

#ifndef SYS_PROC_H_
#define SYS_PROC_H_

#include <sys/common.h>
#include <sys/mem/region.h>
#include <sys/mem/paging.h>
#include <sys/task/elf.h>
#include <sys/task/groups.h>
#include <sys/vfs/node.h>
#include <sys/intrpt.h>

#ifdef __i386__
#include <sys/arch/i586/task/proc.h>
#endif
#ifdef __eco32__
#include <sys/arch/eco32/task/proc.h>
#endif
#ifdef __mmix__
#include <sys/arch/mmix/task/proc.h>
#endif

/* max number of coexistent processes */
#define MAX_PROC_COUNT		8192
#define MAX_FD_COUNT		64

/* for marking unused */
#define INVALID_PID			(MAX_PROC_COUNT + 1)
#define KERNEL_PID			MAX_PROC_COUNT

#define ROOT_UID			0
#define ROOT_GID			0

/* process flags */
#define P_ZOMBIE			1

typedef struct {
	pid_t pid;
	/* the signal that killed the process (SIG_COUNT if none) */
	sig_t signal;
	/* exit-code the process gave us via exit() */
	int exitCode;
	/* memory it has used */
	ulong ownFrames;
	ulong sharedFrames;
	ulong swapped;
	/* cycle-count */
	uLongLong ucycleCount;
	uLongLong kcycleCount;
	/* other stats */
	ulong schedCount;
	ulong syscalls;
} sExitState;

/* represents a process */
typedef struct {
	/* flags for vm86 and zombie */
	uint8_t flags;
	/* process id (2^16 processes should be enough :)) */
	pid_t pid;
	/* parent process id */
	pid_t parentPid;
	/* real, effective and saved user-id */
	uid_t ruid;
	uid_t euid;
	uid_t suid;
	/* real, effective and saved group-id */
	gid_t rgid;
	gid_t egid;
	gid_t sgid;
	/* all groups (may include egid or not) of this process */
	sProcGroups *groups;
	/* the physical address for the page-directory of this process */
	tPageDir pagedir;
	/* the number of frames the process owns, i.e. no cow, no shared stuff, no mapPhysical.
	 * paging-structures are counted, too */
	ulong ownFrames;
	/* the number of frames the process uses, but maybe other processes as well */
	ulong sharedFrames;
	/* pages that are in swap */
	ulong swapped;
	/* the regions */
	size_t regSize;
	void *regions;
	/* the entrypoint of the binary */
	uintptr_t entryPoint;
	/* file descriptors: indices of the global file table */
	file_t fileDescs[MAX_FD_COUNT];
	/* channels to send/receive messages to/from fs (needed in vfs/real.c) */
	sSLList *fsChans;
	/* environment-variables of this process */
	sSLList *env;
	/* for the waiting parent */
	sExitState *exitState;
	/* the address of the sigRet "function" */
	uintptr_t sigRetAddr;
	/* the io-map (NULL by default) */
	uint8_t *ioMap;
	/* start-command */
	const char *command;
	/* threads of this process */
	sSLList *threads;
	/* the directory-node in the VFS of this process */
	sVFSNode *threadDir;
	struct {
		/* I/O stats */
		ulong input;
		ulong output;
	} stats;
} sProc;

/* the area for proc_changeSize() */
typedef enum {CHG_DATA,CHG_STACK} eChgArea;

/**
 * Initializes the process-management
 */
void proc_init(void);

/**
 * Sets the command of <p> to <cmd> and frees the current command-string, if necessary
 *
 * @param p the process
 * @param cmd the new command-name
 */
void proc_setCommand(sProc *p,const char *cmd);

/**
 * Searches for a free pid and returns it or 0 if there is no free process-slot
 *
 * @return the pid or 0
 */
pid_t proc_getFreePid(void);

/**
 * @return the running process
 */
sProc *proc_getRunning(void);

/**
 * @param pid the pid of the process
 * @return the process with given pid
 */
sProc *proc_getByPid(pid_t pid);

/**
 * Checks whether the process with given id exists
 *
 * @param pid the process-id
 * @return true if so
 */
bool proc_exists(pid_t pid);

/**
 * @return the number of existing processes
 */
size_t proc_getCount(void);

/**
 * Returns the file-number for the given file-descriptor
 *
 * @param fd the file-descriptor
 * @return the file-number or < 0 if the fd is invalid
 */
file_t proc_fdToFile(int fd);

/**
 * Searches for a free file-descriptor
 *
 * @return the file-descriptor or the error-code (< 0)
 */
int proc_getFreeFd(void);

/**
 * Associates the given file-descriptor with the given file-number
 *
 * @param fd the file-descriptor
 * @param fileNo the file-number
 * @return 0 on success
 */
int proc_assocFd(int fd,file_t fileNo);

/**
 * Duplicates the given file-descriptor
 *
 * @param fd the file-descriptor
 * @return the error-code or the new file-descriptor
 */
int proc_dupFd(int fd);

/**
 * Redirects <src> to <dst>. <src> will be closed. Note that both fds have to exist!
 *
 * @param src the source-file-descriptor
 * @param dst the destination-file-descriptor
 * @return the error-code or 0 if successfull
 */
int proc_redirFd(int src,int dst);

/**
 * Releases the given file-descriptor (marks it unused)
 *
 * @param fd the file-descriptor
 * @return the file-number that was associated with the fd (or ERR_INVALID_FD)
 */
file_t proc_unassocFd(int fd);

/**
 * Searches for a process with given binary
 *
 * @param bin the binary
 * @param rno will be set to the region-number if found
 * @return the process with the binary or NULL if not found
 */
sProc *proc_getProcWithBin(const sBinDesc *bin,vmreg_t *rno);

/**
 * Determines the least recently used region of all processes
 *
 * @return the region (may be NULL)
 */
sRegion *proc_getLRURegion(void);

/**
 * Determines the mem-usage of all processes
 *
 * @param paging will point to the number of bytes used for paging-structures
 * @param dataShared will point to the number of shared bytes
 * @param dataOwn will point to the number of own bytes
 * @param dataReal will point to the number of really used bytes (considers sharing)
 */
void proc_getMemUsage(size_t *paging,size_t *dataShared,size_t *dataOwn,size_t *dataReal);

/**
 * Determines whether the given process has a child
 *
 * @param pid the process-id
 * @return true if it has a child
 */
bool proc_hasChild(pid_t pid);

/**
 * Clones the current process into the given one, gives the new process a clone of the current
 * thread and saves this thread in proc_clone() so that it will start there on thread_resume().
 * The function returns -1 if there is not enough memory.
 *
 * @param newPid the target-pid
 * @param flags the flags to set for the process (e.g. P_VM86)
 * @return -1 if an error occurred, 0 for parent, 1 for child
 */
int proc_clone(pid_t newPid,uint8_t flags);

/**
 * Starts a new thread at given entry-point. Will clone the kernel-stack from the current thread
 *
 * @param entryPoint the address where to start
 * @param arg the argument
 * @return < 0 if an error occurred or the new tid
 */
int proc_startThread(uintptr_t entryPoint,const void *arg);

/**
 * Destroys the current thread. If it's the only thread in the process, the complete process will
 * destroyed.
 * Note that the actual deletion will be done later!
 *
 * @param exitCode the exit-code
 */
void proc_destroyThread(int exitCode);

/**
 * Removes all regions from the given process
 *
 * @param p the process
 * @param remStack wether the stack should be removed too
 */
void proc_removeRegions(sProc *p,bool remStack);

/**
 * Stores the exit-state of the first terminated child-process of <ppid> into <state>
 *
 * @param ppid the parent-pid
 * @param state the pointer to the state (may be NULL!)
 * @return the pid on success
 */
int proc_getExitState(pid_t ppid,sExitState *state);

/**
 * Sends a SIG_SEGFAULT signal to the given process and performs a thread-switch if the process
 * has been terminated.
 *
 * @param p the process
 */
void proc_segFault(sProc *p);

/**
 * Marks the given process as zombie and notifies the waiting parent thread. As soon as the parent
 * thread fetches the exit-state we'll kill the process
 *
 * @param p the process
 * @param exitCode the exit-code to store
 * @param signal the signal with which it was killed (SIG_COUNT if none)
 */
void proc_terminate(sProc *p,int exitCode,sig_t signal);

/**
 * Kills the given process, that means all structures will be destroyed and the memory free'd.
 * This is not possible with the current process!
 *
 * @param p the process
 */
void proc_kill(sProc *p);

/**
 * Copies the arguments (for exec) in <args> to <*argBuffer> and takes care that everything
 * is ok. <*argBuffer> will be allocated on the heap, so that is has to be free'd when you're done.
 *
 * @param args the arguments to copy
 * @param argBuffer will point to the location where it has been copied (to be used by
 * 	proc_setupUserStack())
 * @param size will point to the size the arguments take in <argBuffer>
 * @param fromUser whether the arguments are from user-space
 * @return the number of arguments on success or < 0
 */
int proc_buildArgs(const char *const *args,char **argBuffer,size_t *size,bool fromUser);

/**
 * Prints all existing processes
 */
void proc_printAll(void);

/**
 * Prints the regions of all existing processes
 */
void proc_printAllRegions(void);

/**
 * Prints the given parts of the page-directory for all existing processes
 *
 * @param parts the parts (see paging_printPageDirOf)
 * @param regions wether to print the regions too
 */
void proc_printAllPDs(uint parts,bool regions);

/**
 * Prints the given process
 *
 * @param p the pointer to the process
 */
void proc_print(const sProc *p);


#if DEBUGGING

/**
 * Starts profiling all processes
 */
void proc_dbg_startProf(void);

/**
 * Stops profiling all processes and prints the result
 */
void proc_dbg_stopProf(void);

#endif

#endif /*SYS_PROC_H_*/