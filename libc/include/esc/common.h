/**
 * @version		$Id$
 * @author		Nils Asmussen <nils@script-solution.de>
 * @copyright	2008 Nils Asmussen
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <types.h>
#include <stddef.h>

#ifndef DEBUGGING
#define DEBUGGING 1
#endif

/**
 * Calculates the stacktrace
 *
 * @return the trace (null-terminated)
 */
u32 *getStackTrace(void);

/**
 * Prints the stack-trace
 */
void printStackTrace(void);

/**
 * Prints the last error-message
 */
void printLastError(void);

#endif /*COMMON_H_*/
