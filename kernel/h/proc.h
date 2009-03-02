/**
 * @version		$Id: proc.h 81 2008-11-23 12:43:27Z nasmussen $
 * @author		Nils Asmussen <nils@script-solution.de>
 * @copyright	2008 Nils Asmussen
 */

#ifndef PROC_H_
#define PROC_H_

#include "common.h"
#include "intrpt.h"

/* max number of processes */
#define PROC_COUNT		1024
#define MAX_FD_COUNT	16

/* the signals
#define SIG_TERM		1
#define SIG_KILL		2
#define SIG_SEGFAULT	3 */

/* the process-state which will be saved for context-switching */
typedef struct {
	u32 esp;
	u32 edi;
	u32 esi;
	u32 ebp;
	u32 eflags;
} sProcSave;

/* the process states */
typedef enum {ST_UNUSED = 0,ST_RUNNING = 1,ST_READY = 2,ST_BLOCKED = 3,ST_ZOMBIE = 4} eProcState;

/* represents a process */
/* TODO move stuff for existing processes to the kernel-stack-page */
typedef struct {
	/* process state. see eProcState */
	u8 state;
	/* process id (2^16 processes should be enough :)) */
	tPid pid;
	/* parent process id */
	tPid parentPid;
	/* the physical address for the page-directory of this process */
	u32 physPDirAddr;
	/* the number of pages per segment */
	u32 textPages;
	u32 dataPages;
	u32 stackPages;
	sProcSave save;
	/* file descriptors: indices of the global file table */
	tFile fileDescs[MAX_FD_COUNT];
	/* a bitfield with signals the process should get
	u16 signals; */
} sProc;

/* the area for proc_changeSize() */
typedef enum {CHG_DATA,CHG_STACK} eChgArea;

/**
 * Saves the state of the current process in the given area
 *
 * @param saveArea the area where to save the state
 * @return false for the caller-process, true for the resumed process
 */
extern bool proc_save(sProcSave *saveArea);

/**
 * Resumes the given state
 *
 * @param pageDir the physical address of the page-dir
 * @param saveArea the area to load the state from
 * @return always true
 */
extern bool proc_resume(u32 pageDir,sProcSave *saveArea);

/**
 * Initializes the process-management
 */
void proc_init(void);

/**
 * Searches for a free pid and returns it or 0 if there is no free process-slot
 *
 * @return the pid or 0
 */
tPid proc_getFreePid(void);

/**
 * @return the running process
 */
sProc *proc_getRunning(void);

/**
 * @param pid the pid of the process
 * @return the process with given pid
 */
sProc *proc_getByPid(tPid pid);

/**
 * Switches to another process
 */
void proc_switch(void);

/**
 * Returns the file-number for the given file-descriptor
 *
 * @param fd the file-descriptor
 * @return the file-number or < 0 if the fd is invalid
 */
tFile proc_fdToFile(tFD fd);

/**
 * Opens the given file
 *
 * @param fileNo the file-number
 * @return the file-descriptor if successfull or the error-code (< 0)
 */
s32 proc_openFile(tFile fileNo);

/**
 * Closes the given file-descriptor. That means it releases the fd-slot with given index.
 *
 * @param fd the file-descriptor
 * @return the file-number that was associated with the fd (or ERR_INVALID_FD)
 */
tFile proc_closeFile(tFD fd);

/**
 * Clones the current process into the given one, saves the new process in proc_clone() so that
 * it will start there on proc_resume(). The function returns -1 if there is
 * not enough memory.
 *
 * @param newPid the target-pid
 * @return -1 if an error occurred, 0 for parent, 1 for child
 */
s32 proc_clone(tPid newPid);

/**
 * Commit suicide. Marks the current process as destroyable. After the next context-switch
 * the process will be removed.
 */
void proc_suicide(void);

/**
 * Destroyes the given process. That means the process-slot will be marked as "unused" and the
 * paging-structure will be freed.
 *
 * @param p the process
 */
void proc_destroy(sProc *p);

/**
 * Setups the given interrupt-stack for the current process
 *
 * @param frame the interrupt-stack-frame
 */
void proc_setupIntrptStack(sIntrptStackFrame *frame);

/**
 * Checks wether the given segment-sizes are valid
 *
 * @param textPages the number of text-pages
 * @param dataPages the number of data-pages
 * @param stackPages the number of stack-pages
 * @return true if so
 */
bool proc_segSizesValid(u32 textPages,u32 dataPages,u32 stackPages);

/**
 * Changes the size of either the data-segment of the process or the stack-segment
 * If <change> is positive pages will be added and otherwise removed. Added pages
 * will always be cleared.
 * If there is not enough memory the function returns false.
 * Note that the size of the current process (dataPages / stackPages) will be adjusted!
 *
 * @param change the number of pages to add or remove
 * @param area the data or stack? (CHG_*)
 * @return true if successfull
 */
bool proc_changeSize(s32 change,eChgArea area);

#if DEBUGGING

/**
 * Prints the given process
 *
 * @param p the pointer to the process
 */
void proc_dbg_print(sProc *p);

/**
 * Prints the given process-state
 *
 * @param state the pointer to the state-struct
 */
void proc_dbg_printState(sProcSave *state);

#endif

#endif /*PROC_H_*/
