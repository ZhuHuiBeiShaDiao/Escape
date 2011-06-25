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

#ifndef STDIO_H_
#define STDIO_H_

/* Note that most of the comments here are from doc/iso_c99.pdf */

#include <esc/common.h>
#include <esc/io.h>

#define _IOFBF			0	/* fully buffered */
#define _IOLBF			1	/* line buffered */
#define _IONBF			2	/* unbuffered */

/* the size of the buffer used by the setbuf function */
#define BUFSIZ			512
/* the minimum number of files that the implementation guarantees can be open simultaneously */
#define FOPEN_MAX		64
/* the size needed for an array of char large enough to hold the longest file name string that the
 * implementation guarantees can be opened */
#define FILENAME_MAX	64
/* the maximum number of unique file names that can be generated by the tmpnam function */
#define TMP_MAX			16
/* the size needed for an array of char large enough to hold a temporary file name string generated
 * by the tmpnam function */
#define L_tmpnam		0

#ifndef EOF
#	define EOF			-1
#endif

typedef off_t fpos_t;

#ifdef __cplusplus
extern "C" {
using namespace std;
#endif

/* hack to provide FILE without including ../lib/c/stdio/iobuf.h */
#ifndef IOBUF_H_
typedef void FILE;
#endif

/* std-streams */
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;


/* --- Operations on files --- */

/**
 * The remove function causes the file whose name is the string pointed to by filename
 * to be no longer accessible by that name. A subsequent attempt to open that file using that
 * name will fail, unless it is created anew. If the file is open, the behavior of the remove
 * function is implementation-defined.
 *
 * @param filename the filename
 * @return 0 on success
 */
int remove(const char *filename);

/**
 * The rename function causes the file whose name is the string pointed to by old to be
 * henceforth known by the name given by the string pointed to by new. The file named
 * old is no longer accessible by that name. If a file named by the string pointed to by new
 * exists prior to the call to the rename function, the behavior is implementation-defined.
 *
 * @param old the old name
 * @param newn the new name
 * @return 0 on success
 */
int rename(const char *old,const char *newn);

/**
 * The tmpfile function creates a temporary binary file that is different from any other
 * existing file and that will automatically be removed when it is closed or at program
 * termination. If the program terminates abnormally, whether an open temporary file is
 * removed is implementation-defined. The file is opened for update with "wb+" mode.
 *
 * @return a pointer to the stream of the file that it created. If the file cannot be created,
 * the tmpfile function returns a null pointer.
 */
FILE *tmpfile(void);

/**
 * The tmpnam function generates a string that is a valid file name and that is not the same
 * as the name of an existing file. The function is potentially capable of generating TMP_MAX
 * different strings, but any or all of them may already be in use by existing files and
 * thus not be suitable return values.
 * The tmpnam function generates a different string each time it is called.
 * The implementation shall behave as if no library function calls the tmpnam function.
 *
 * @param s optionally the buffer to write to
 * @return If no suitable string can be generated, the tmpnam function returns a null pointer.
 * 	Otherwise, if the argument is a null pointer, the tmpnam function leaves its result in an
 * 	internal static object and returns a pointer to that object (subsequent calls to the tmpnam
 * 	function may modify the same object). If the argument is not a null pointer, it is assumed
 * 	to point to an array of at least L_tmpnam chars; the tmpnam function writes its result
 * 	in that array and returns the argument as its value.
 */
char *tmpnam(char *s);

/* --- File access functions --- */

/**
 * Opens the file whose name is specified in the parameter filename and associates it with a
 * stream that can be identified in future operations by the FILE object whose pointer is returned.
 * The operations that are allowed on the stream and how these are performed are defined by the
 * mode parameter.
 *
 * @param filename the file to open
 * @param mode string containing a file access modes. It can be:
 * 	"r"		Open a file for reading. The file must exist.
 * 	"w"		Create an empty file for writing. If a file with the same name already exists its content
 * 			is erased and the file is treated as a new empty file.
 * 	"a"		Append to a file. Writing operations append data at the end of the file. The file is
 * 			created if it does not exist.
 * 	"r+"	Open a file for update both reading and writing. The file must exist.
 * 	"w+"	Create an empty file for both reading and writing. If a file with the same name
 * 			already exists its content is erased and the file is treated as a new empty file.
 * 	"a+"	Open a file for reading and appending. All writing operations are performed at the
 * 			end of the file, protecting the previous content to be overwritten. You can
 * 			reposition (fseek, rewind) the internal pointer to anywhere in the file for
 * 			reading, but writing operations will move it back to the end of file.
 * 			The file is created if it does not exist.
 * @return a FILE object if successfull or NULL if failed
 */
FILE *fopen(const char *filename,const char *mode);

/**
 * The freopen function opens the file whose name is the string pointed to by filename
 * and associates the stream pointed to by stream with it. The mode argument is used just
 * as in the fopen function.
 * If filename is a null pointer, the freopen function attempts to change the mode of
 * the stream to that specified by mode, as if the name of the file currently associated with
 * the stream had been used. It is implementation-defined which changes of mode are
 * permitted (if any), and under what circumstances.
 * The freopen function first attempts to close any file that is associated with the specified
 * stream. Failure to close the file is ignored. The error and end-of-file indicators for the
 * stream are cleared.
 *
 * @param filename the file to open (may be NULL)
 * @param mode the mode, like in fopen()
 * @param stream the stream to reopen
 * @return the new stream on success or NULL if failed
 */
FILE *freopen(const char *filename,const char *mode,FILE *stream);

/**
 * Except that it returns no value, the setbuf function is equivalent to the setvbuf
 * function invoked with the values _IOFBF for mode and BUFSIZ for size, or (if buf
 * is a null pointer), with the value _IONBF for mode.
 *
 * @param stream the stream
 * @param buf the buffer to set
 */
void setbuf(FILE *stream,char *buf);

/**
 * The setvbuf function may be used only after the stream pointed to by stream has
 * been associated with an open file and before any other operation (other than an
 * unsuccessful call to setvbuf) is performed on the stream. The argument mode
 * determines how stream will be buffered, as follows: _IOFBF causes input/output to be
 * fully buffered; _IOLBF causes input/output to be line buffered; _IONBF causes
 * input/output to be unbuffered. If buf is not a null pointer, the array it points to may be
 * used instead of a buffer allocated by the setvbuf function and the argument size
 * specifies the size of the array; otherwise, size may determine the size of a buffer
 * allocated by the setvbuf function. The contents of the array at any time are
 * indeterminate.
 *
 * @param stream the stream
 * @param buf the buffer to set
 * @param mode the type of buffering
 * @param size the size of the buffer
 * @return 0 on success
 */
int setvbuf(FILE *stream,char *buf,int mode,size_t size);

/**
 * The  function  fflush  forces a write of all buffered data for the given output
 *
 * @param stream the stream
 * @return 0 on success or EOF
 */
int fflush(FILE *stream);

/**
 * Closes the given stream
 *
 * @param stream the stream
 * @return 0 on success or EOF
 */
int fclose(FILE *stream);

/* --- Direct input/output --- */

/**
 * Reads an array of count elements, each one with a size of size bytes, from the stream and
 * stores them in the block of memory specified by ptr.
 * The postion indicator of the stream is advanced by the total amount of bytes read.
 * The total amount of bytes read if successful is (size * count).
 *
 * @param ptr Pointer to a block of memory with a minimum size of (size*count) bytes.
 * @param size Size in bytes of each element to be read.
 * @param count Number of elements, each one with a size of size bytes.
 * @param file Pointer to a tFile object that specifies an input stream.
 * @return total number of read elements (maybe less than <count>, if EOF is reached or an error
 * 	occurred)
 */
size_t fread(void *ptr,size_t size,size_t count,FILE *file);

/**
 * Writes an array of count elements, each one with a size of size bytes, from the block of memory
 * pointed by ptr to the current position in the stream.
 * The postion indicator of the stream is advanced by the total number of bytes written.
 *
 * @param ptr Pointer to the array of elements to be written.
 * @param size Size in bytes of each element to be written.
 * @param count Number of elements, each one with a size of size bytes.
 * @param file Pointer to a tFile object that specifies an output stream.
 * @return total number of written elements (if it differs from <count> an error is occurred)
 */
size_t fwrite(const void *ptr,size_t size,size_t count,FILE *file);

/* --- Character input/output --- */

/**
 * Reads one char from <file>
 *
 * @param file the file
 * @return the character or IO_EOF
 */
int fgetc(FILE *stream);

/**
 * Reads max. <max> from <file> (or till EOF) into the given buffer
 *
 * @param str the buffer
 * @param max the maximum number of chars to read
 * @param file the file
 * @return str on success or NULL
 */
char *fgets(char *str,size_t max,FILE *file);

/**
 * The same as fgets(), except that it don't includes the newline into the string
 *
 * @param str the buffer
 * @param max the maximum number of chars to read
 * @param file the file
 * @return str on success or NULL
 */
char *fgetl(char *str,size_t max,FILE *f);

/**
 * Prints the given character to <file>
 *
 * @param c the character
 * @param file the file
 * @return the character or IO_EOF if failed
 */
int fputc(int c,FILE *file);

/**
 * Prints the given string to <file>
 *
 * @param str the string
 * @param file the file
 * @return the number of written chars
 */
int fputs(const char *str,FILE *file);

/**
 * Reads one char from <file>
 *
 * @param file the file
 * @return the character or IO_EOF
 */
int getc(FILE *file);

/**
 * Reads one char from STDIN
 *
 * @return the character or IO_EOF
 */
int getchar(void);

/**
 * Reads from STDIN (or till EOF) into the given buffer
 *
 * @param buffer the buffer
 * @return buffer on success or NULL
 */
char *gets(char *buffer);

/**
 * Prints the given character to <file>
 *
 * @param c the character
 * @param file the file
 * @return the character or IO_EOF if failed
 */
int putc(int c,FILE *file);

/**
 * Prints the given character to STDOUT
 *
 * @param c the character
 * @return the character or IO_EOF if failed
 */
int putchar(int c);

/**
 * Prints the given string to STDOUT
 *
 * @param str the string
 * @return the number of written chars
 */
int puts(const char *str);

/**
 * Puts the given character back to the buffer for <file>. If the buffer is full, the character
 * will be ignored.
 *
 * @param c the character
 * @param file the file
 * @return 0 on success or IO_EOF
 */
int ungetc(int c,FILE *file);

/* --- File positioning --- */

/**
 * The fgetpos function stores the current values of the parse state (if any) and file
 * position indicator for the stream pointed to by stream in the object pointed to by pos.
 * The values stored contain unspecified information usable by the fsetpos function for
 * repositioning the stream to its position at the time of the call to the fgetpos function
 *
 * @param stream the stream
 * @param pos where to store the position
 * @return 0 on success
 */
int fgetpos(FILE *stream,fpos_t *pos);

/**
 * The fseek function sets the file position indicator for the stream pointed to by stream.
 * If a read or write error occurs, the error indicator for the stream is set and fseek fails.
 * For a binary stream, the new position, measured in characters from the beginning of the
 * file, is obtained by adding offset to the position specified by whence. The specified
 * position is the beginning of the file if whence is SEEK_SET, the current value of the file
 * position indicator if SEEK_CUR, or end-of-file if SEEK_END. A binary stream need not
 * meaningfully support fseek calls with a whence value of SEEK_END.
 * For a text stream, either offset shall be zero, or offset shall be a value returned by
 * an earlier successful call to the ftell function on a stream associated with the same file
 * and whence shall be SEEK_SET.
 * After determining the new position, a successful call to the fseek function undoes any
 * effects of the ungetc function on the stream, clears the end-of-file indicator for the
 * stream, and then establishes the new position. After a successful fseek call, the next
 * operation on an update stream may be either input or output.
 *
 * @param fd the file-descriptor
 * @param offset the offset
 * @param whence the type of seek (SEEK_*)
 * @return 0 on success
 */
int fseek(FILE *stream,int offset,uint whence);

/**
 * The fsetpos function sets the mbstate_t object (if any) and file position indicator
 * for the stream pointed to by stream according to the value of the object pointed to by
 * pos, which shall be a value obtained from an earlier successful call to the fgetpos
 * function on a stream associated with the same file. If a read or write error occurs, the
 * error indicator for the stream is set and fsetpos fails.
 * A successful call to the fsetpos function undoes any effects of the ungetc function
 * on the stream, clears the end-of-file indicator for the stream, and then establishes the new
 * parse state and position. After a successful fsetpos call, the next operation on an
 * update stream may be either input or output.
 *
 * @param stream the stream
 * @param pos the new position
 * @return 0 on success
 */
int fsetpos(FILE *stream,const fpos_t *pos);

/**
 * The ftell function obtains the current value of the file position indicator for the stream
 * pointed to by stream. For a binary stream, the value is the number of characters from
 * the beginning of the file. For a text stream, its file position indicator contains unspecified
 * information, usable by the fseek function for returning the file position indicator for the
 * stream to its position at the time of the ftell call; the difference between two such
 * return values is not necessarily a meaningful measure of the number of characters written
 * or read.
 *
 * @param stream the stream
 * @return the current position
 */
long ftell(FILE *stream);

/**
 * The rewind function sets the file position indicator for the stream pointed to by
 * stream to the beginning of the file. It is equivalent to
 * 	(void)fseek(stream, 0L, SEEK_SET)
 * except that the error indicator for the stream is also cleared.
 *
 * @param stream the stream
 */
void rewind(FILE *stream);

/* --- Error handling --- */

/**
 * The clearerr function clears the end-of-file and error indicators for the stream pointed
 * to by stream.
 *
 * @param stream the stream
 */
void clearerr(FILE *stream);

/**
 * The feof function tests the end-of-file indicator for the stream pointed to by stream.
 *
 * @param stream the stream
 * @return true if at EOF
 */
int feof(FILE *stream);

/**
 * The ferror function tests the error indicator for the stream pointed to by stream.
 *
 * @param stream the stream
 * @return nonzero if and only if the error indicator is set for stream.
 */
int ferror(FILE *stream);

/**
 * Prints "<msg>: <lastError>"
 *
 * @param msg the prefix of the message
 */
void perror(const char *msg);

/* --- Formated input/output --- */

/**
 * Formated output to STDOUT. Supports:
 * You can format values with: %[flags][width][.precision][length]specifier
 *
 * Flags:
 *  "-"		: Left-justify within the given field width; Right justification is the default
 *  "+"		: Forces to precede the result with a plus or minus sign (+ or -) even for
 *  		  positive numbers
 *  " "		: If no sign is going to be written, a blank space is inserted before the value.
 *  "#"		: Used with o, x or X specifiers the value is preceeded with 0, 0x or 0X respectively
 *  "0"		: Left-pads the number with zeroes (0) instead of spaces, where padding is specified
 *
 * Width:
 *  [0-9]+	: Minimum number of characters to be printed. If the value to be printed is shorter
 *  		  than this number, the result is padded with blank spaces.
 *  "*"		: The width is not specified in the format string, but as an additional integer value
 *  		  argument preceding the argument that has to be formatted.
 *
 * Precision:
 * 	[0-9]+	: For integer specifiers (d, i, o, u, x, X): precision specifies the minimum number of
 * 			  digits to be written. If the value to be written is shorter than this number, the
 * 			  result is padded with leading zeros.
 * 	"*"		: The precision is not specified in the format string, but as an additional integer
 * 			  value argument preceding the argument that has to be formatted.
 *
 * Length:
 * 	"h"		: The argument is interpreted as a short int or unsigned short int (only applies to
 * 			  integer specifiers: i, d, o, u, x and X).
 * 	"l"		: The argument is interpreted as a long int, unsigned long int or double
 * 	"L"		: The argument is interpreted as a long long int or unsigned long long int
 *
 * Specifier:
 * 	"c"		: a character
 * 	"d"		: signed decimal integer
 * 	"i"		: alias of "d"
 * 	"f"		: decimal floating point
 * 	"o"		: unsigned octal integer
 * 	"u"		: unsigned decimal integer
 * 	"x"		: unsigned hexadecimal integer
 * 	"X"		: unsigned hexadecimal integer (capical letters)
 * 	"b"		: unsigned binary integer
 * 	"p"		: pointer address
 * 	"s"		: string of characters
 * 	"n"		: prints nothing. The argument must be a pointer to a signed int, where the number of
 * 			  characters written so far is stored.
 *
 * @param fmt the format
 * @return the number of written chars
 */
int printf(const char *fmt,...);

/**
 * Like printf(), but prints to <file>
 *
 * @param file the file
 * @param fmt the format
 * @return the number of written chars
 */
int fprintf(FILE *file,const char *fmt,...);

/**
 * Like printf(), but prints to the given string
 *
 * @param str the string to print to
 * @param fmt the format
 * @return the number of written chars
 */
int sprintf(char *str,const char *fmt,...);

/**
 * Like sprintf(), but with max length
 *
 * @param str the string to print to
 * @param n the size of <str>
 * @param fmt the format
 * @return the number chars that have been written or would have been written if <n> were
 * 	large enough
 */
int snprintf(char *str,size_t n,const char *fmt,...);

/**
 * Like snprintf() but with given arguments
 *
 * @param str the string to print to
 * @param n the size of <str>
 * @param fmt the format
 * @param ap the argument-list
 * @return the number chars that have been written or would have been written if <n> were
 * 	large enough
 */
int vsnprintf(char *str,size_t n,const char *fmt,va_list ap);

/**
 * Like printf(), but lets you specify the argument-list
 *
 * @param fmt the format
 * @param ap the argument-list
 * @return the number of written chars
 */
int vprintf(const char *fmt,va_list ap);

/**
 * Like vprintf(), but prints to <file>
 *
 * @param file the file
 * @param fmt the format
 * @param ap the argument-list
 * @return the number of written chars
 */
int vfprintf(FILE *file,const char *fmt,va_list ap);

/**
 * Like vprintf(), but prints to <str>
 *
 * @param str the string to print to
 * @param fmt the format
 * @param ap the argument-list
 * @return the number of written chars
 */
int vsprintf(char *str,const char *fmt,va_list ap);

/**
 * Reads data in the specified format from STDIN. Supports:
 * 	%d: signed integer
 * 	%u: unsigned integer, base 10
 * 	%o: unsigned integer, base 8
 * 	%x: unsigned integer, base 16
 * 	%b: unsigned integer, base 2
 * 	%s: string
 * 	%c: character
 *
 * Additionally you can specify the max. length:
 * 	%2d
 * 	%10s
 *  ...
 *
 * @param fmt the format
 * @return the number of matched variables
 */
int scanf(const char *fmt,...);

/**
 * Like scanf, but with specified arguments
 *
 * @param fmt the format
 * @param ap the argument-list
 * @return the number of matched variables
 */
int vscanf(const char *fmt,va_list ap);

/**
 * Reads data in the specified format from <file>. See scanf().
 *
 * @param file the file
 * @param fmt the format
 * @return the number of matched variables
 */
int fscanf(FILE *file,const char *fmt,...);

/**
 * Like fscanf, but with specified arguments
 *
 * @param file the file
 * @param fmt the format
 * @param ap the argument-list
 * @return the number of matched variables
 */
int vfscanf(FILE *file,const char *fmt,va_list ap);

/**
 * Reads data in the specified format from <str>. See scanf().
 *
 * @param str the string
 * @param fmt the format
 * @return the number of matched variables
 */
int sscanf(const char *str,const char *fmt,...);

/**
 * Like sscanf(), but with specified arguments
 *
 * @param str the string
 * @param fmt the format
 * @param ap the argument-list
 * @return the number of matched variables
 */
int vsscanf(const char *str,const char *fmt,va_list ap);

/* --- Other --- */

/**
 * The function fileno() examines the  argument  stream  and  returns  its
 * integer descriptor.
 *
 * @param stream the stream
 * @return the file-descriptor
 */
int fileno(FILE *stream);

/**
 * The same as escc_get() but reads the escape-code from <f>. Assumes that the last read char
 * was '\033'.
 *
 * @param f the file
 * @param n1 will be set to the first argument (ESCC_ARG_UNUSED if unused)
 * @param n2 will be set to the second argument (ESCC_ARG_UNUSED if unused)
 * @param n3 will be set to the third argument (ESCC_ARG_UNUSED if unused)
 * @return the scanned escape-code (ESCC_*)
 */
int freadesc(FILE *f,int *n1,int *n2,int *n3);

/**
 * Prints "<prefix>: <lastError>" to stderr
 *
 * @param prefix the prefix of the message
 * @return the number of written chars
 */
int printe(const char *prefix,...);

/**
 * The same as printe(), but with argument-pointer specified
 *
 * @param prefix the prefix of the message
 * @param ap the argument-pointer
 * @return the number of written chars
 */
int vprinte(const char *prefix,va_list ap);

/**
 * The  isatty()  function  tests  whether  fd  is an open file descriptor
 * referring to a terminal.
 *
 * @param fd the file-descriptor
 * @return true if it referrs to a terminal
 */
int isatty(int fd);

/**
 * The lseek() function repositions the offset of the open file associated with the file descriptor
 * fd to the argument offset according to the directive whence as follows:
 * 	SEEK_SET: The offset is set to offset bytes.
 * 	SEEK_CUR: The offset is set to its current location plus offset bytes.
 * 	SEEK_END: The offset is set to the size of the file plus offset bytes.
 *
 * @param fd the file-descriptor
 * @param offset the offset
 * @param whence the type of seek (SEEK_*)
 * @return the new position
 */
off_t lseek(int fd,off_t offset,uint whence);

#ifdef __cplusplus
}
#endif

#endif /* STDIO_H_ */
