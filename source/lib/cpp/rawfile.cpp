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

#include <esc/dir.h>
#include <rawfile.h>
#include <stdexcept>

namespace std {
	void rawfile::open(const string& filename,open_type mode) {
		close();
		uint flags = 0;
		if(mode & READ)
			flags |= IO_READ;
		if(mode & WRITE)
			flags |= IO_WRITE | IO_CREATE;
		if(mode & APPEND)
			flags |= IO_APPEND;
		if(mode & TRUNCATE)
			flags |= IO_TRUNCATE;
		_fd = ::open(filename.c_str(),flags);
		if(_fd < 0)
			throw io_exception("Unable to open",_fd);
		_mode = mode;
	}
	void rawfile::use(int fd) {
		close();
		_fd = fd;
		_mode = READ | WRITE;
	}
	void rawfile::seek(off_type offset,int whence) {
		if(_fd < 0)
			throw io_exception("File not opened",0);
		int res;
		if((res = ::seek(_fd,offset,whence)) < 0)
			throw io_exception("Unable to seek",res);
	}
	rawfile::size_type rawfile::read(void *data,size_type size,size_type count) {
		if(_fd < 0 || !(_mode & READ))
			throw io_exception("File not opened for reading",0);
		int res = RETRY(::read(_fd,data,size * count));
		if(res < 0)
			throw io_exception("Unable to read",res);
		return (size_type)res / size;
	}
	rawfile::size_type rawfile::write(const void *data,size_type size,size_type count) {
		if(_fd < 0 || !(_mode & WRITE))
			throw io_exception("File not opened for writing",0);
		int res = ::write(_fd,data,size * count);
		if(res < 0)
			throw io_exception("Unable to write",res);
		return (size_type)res / size;
	}
	void rawfile::close() {
		if(_fd >= 0) {
			::close(_fd);
			_fd = -1;
			_mode = 0;
		}
	}
}