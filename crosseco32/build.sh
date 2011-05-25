#!/bin/bash
ROOT=`dirname $(readlink -f $0)`
BUILD=$ROOT/../build/eco32-cross
DIST=$ROOT/../build/eco32-dist
SRC=$ROOT/src
HEADER=$ROOT/../include
LIBC=$ROOT/../lib/c
MAKE_ARGS=-j8

GCCCORE_ARCH=gcc-core-4.4.3.tar.bz2
GCCGPP_ARCH=gcc-g++-4.4.3.tar.bz2
BINUTILS_ARCH=binutils-2.20.tar.bz2
NEWLIB_ARCH=newlib-1.15.0.tar.gz

if [ "$1" == "--rebuild" ] || [ ! -d $DIST ]; then
	REBUILD=1
else
	REBUILD=0
fi

# setup
export PREFIX=$DIST
export TARGET=eco32-elf-escape
mkdir -p $BUILD/gcc $BUILD/binutils $BUILD/newlib
if [ $REBUILD -eq 1 ]; then
	if [ -d $SRC ]; then
		if [ -d $BUILD/binutils ]; then
			cd $BUILD/binutils
			make distclean
		fi
		if [ -d $BUILD/gcc ]; then
			cd $BUILD/gcc
			make distclean
		fi
		cd $ROOT
	fi
	rm -Rf $SRC $DIST
	mkdir -p $SRC
fi

# binutils
if [ $REBUILD -eq 1 ]; then
	cat $BINUTILS_ARCH | bunzip2 | tar -C $SRC -xf -
	mv $SRC/binutils-2.20 $SRC/binutils
	patch -p0 < binutils.diff
fi
cd $BUILD/binutils
if [ $REBUILD -eq 1 ] || [ ! -f $BUILD/binutils/Makefile ]; then
	$SRC/binutils/configure --target=$TARGET --prefix=$PREFIX --disable-nls
	if [ $? -ne 0 ]; then
		exit 1
	fi
fi
make $MAKE_ARGS all && make install
if [ $? -ne 0 ]; then
	exit 1
fi

# gcc
cd $ROOT
if [ $REBUILD -eq 1 ]; then
	cat $GCCCORE_ARCH | bunzip2 | tar -C $SRC -xf -
	cat $GCCGPP_ARCH | bunzip2 | tar -C $SRC -xf -
	mv $SRC/gcc-4.4.3 $SRC/gcc
	patch -p0 < gcc.diff
fi
export PATH=$PATH:$PREFIX/bin
cd $BUILD/gcc
if [ $REBUILD -eq 1 ] || [ ! -f $BUILD/gcc/Makefile ]; then
	$SRC/gcc/configure --target=$TARGET --prefix=$PREFIX --disable-nls \
		  --enable-languages=c,c++ --with-headers=$HEADER \
		  --disable-linker-build-id --with-gxx-include-dir=$HEADER/cpp
	if [ $? -ne 0 ]; then
		exit 1
	fi
fi
make $MAKE_ARGS all-gcc && make install-gcc
if [ $? -ne 0 ]; then
	exit 1
fi
ln -sf $DIST/bin/$TARGET-gcc $DIST/bin/$TARGET-cc

# libgcc
make $MAKE_ARGS all-target-libgcc && make install-target-libgcc
if [ $? -ne 0 ]; then
	exit 1
fi

# newlib
cd $ROOT
#if [ $REBUILD -eq 1 ] || [ ! -d $SRC/newlib ]; then
#	rm -Rf $SRC/newlib
#	cat $NEWLIB_ARCH | gunzip | tar -C $SRC -xf -
#	mv $SRC/newlib-1.15.0 $SRC/newlib
#	patch -p0 < newlib1.diff
#	cd $SRC/newlib/newlib/libc/sys
#	autoconf
#	cd $ROOT
#	patch -p0 < newlib2.diff
#	SYSCALLS_FILE=$SRC/newlib/newlib/libc/sys/escape/syscalls.S
#	cat $LIBC/syscalls.s > $SYSCALLS_FILE
#	echo 'SYSC_VOID_1ARGS _exit,$SYSCALL_EXIT' >> $SYSCALLS_FILE
#	echo 'SYSC_VOID_1ARGS close,$SYSCALL_CLOSE' >> $SYSCALLS_FILE
#	echo 'SYSC_RET_2ARGS_ERR exec,$SYSCALL_EXEC' >> $SYSCALLS_FILE
#	echo 'SYSC_RET_0ARGS_ERR fork,$SYSCALL_FORK' >> $SYSCALLS_FILE
#	echo 'SYSC_RET_2ARGS_ERR fstat,$SYSCALL_FSTAT' >> $SYSCALLS_FILE
#	echo 'SYSC_RET_0ARGS getpid,$SYSCALL_PID' >> $SYSCALLS_FILE
#	echo 'SYSC_RET_1ARGS_ERR isatty,$SYSCALL_ISTERM' >> $SYSCALLS_FILE
#	echo 'SYSC_RET_3ARGS_ERR sendSignalTo,$SYSCALL_SENDSIG' >> $SYSCALLS_FILE
#	echo 'SYSC_RET_2ARGS_ERR _stat,$SYSCALL_STAT' >> $SYSCALLS_FILE
#	echo 'SYSC_RET_2ARGS_ERR link,$SYSCALL_LINK' >> $SYSCALLS_FILE
#	echo 'SYSC_RET_3ARGS_ERR seek,$SYSCALL_SEEK' >> $SYSCALLS_FILE
#	echo 'SYSC_RET_2ARGS_ERR open,$SYSCALL_OPEN' >> $SYSCALLS_FILE
#	echo 'SYSC_RET_3ARGS_ERR read,$SYSCALL_READ' >> $SYSCALLS_FILE
#	echo 'SYSC_RET_1ARGS_ERR _changeSize,$SYSCALL_CHGSIZE' >> $SYSCALLS_FILE
#	echo 'SYSC_RET_1ARGS_ERR unlink,$SYSCALL_UNLINK' >> $SYSCALLS_FILE
#	echo 'SYSC_RET_1ARGS_ERR waitChild,$SYSCALL_WAITCHILD' >> $SYSCALLS_FILE
#	cd $SRC/newlib/newlib/libc/sys
#	autoconf
#	cd escape
#	autoreconf
#fi

# newlib will create this dir again, so remove it, let newlib put its stuff in there and remove it
# again afterwards & replace it with a symlink to our include
#rm -f $DIST/$TARGET/include
#cd $BUILD/newlib
#if [ $REBUILD -eq 1 ] || [ ! -f Makefile ]; then
#	rm -Rf $BUILD/newlib/*
#	$SRC/newlib/configure --target=$TARGET --prefix=$PREFIX
#	if [ $? -ne 0 ]; then
#		exit 1
#	fi
#	# to prevent problems with makeinfo
#	echo "MAKEINFO = :" >> $BUILD/newlib/Makefile
#fi
#make $MAKE_ARGS && make install
#if [ $? -ne 0 ]; then
#	exit 1
#fi

# libstdc++
#export PATH=$PATH:$PREFIX/bin
#mkdir -p $BUILD/gcc/libstdc++-v3
#cd $BUILD/gcc/libstdc++-v3
#if [ $REBUILD -eq 1 ] || [ ! -f Makefile ]; then
#	# pretend that we're using newlib. without it I can't find a way to build libsupc++
#	CPP=$TARGET-cpp $SRC/gcc/libstdc++-v3/configure --host=$TARGET --prefix=$PREFIX \
#		--disable-hosted-libstdcxx --disable-nls --with-newlib
#	if [ $? -ne 0 ]; then
#		exit 1
#	fi
#fi
#cd include
#make $MAKE_ARGS && make install
#if [ $? -ne 0 ]; then
#	exit 1
#fi
#cd ../libsupc++
#make $MAKE_ARGS && make install
#if [ $? -ne 0 ]; then
#	exit 1
#fi


# create basic symlinks
rm -Rf $DIST/$TARGET/sys-include $DIST/$TARGET/include
ln -sf $ROOT/../include $DIST/$TARGET/sys-include
ln -sf $ROOT/../include $DIST/$TARGET/include

# copy crt* to basic gcc-stuff
cp -f $BUILD/gcc/$TARGET/libgcc/crt*.o $DIST/lib/gcc/$TARGET/4.4.3
