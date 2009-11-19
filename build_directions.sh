#!/bin/sh

DIR=`dirname $0`
HOST=arm-none-linux-gnueabi
# Gather requirements
# path to target filesystem
echo "Enter PREFIX (currently '$PREFIX'):\c"
read VALUE
export PREFIX=${VALUE:=$PREFIX}

# path to tiler-userspace git root
echo "Enter path to tiler-userspace (currently '$TILER_USERSPACE'):\c"
read VALUE
export TILER_USERSPACE=${VALUE:=$TILER_USERSPACE}

# path to kernel-syslink git root
echo "Enter path to kernel-syslink (currently '$KRNLSRC'):\c"
read VALUE
export KRNLSRC=${VALUE:=$KRNLSRC}

# path to userspace-space git root
export USERSPACE_SYSLINK=`readlink -f $DIR`

echo "Enter tool path (currently '$TOOL'):\c"
read VALUE
TOOLBIN=${VALUE:=$TOOLBIN}

echo TOOLBIN           is ${TOOLBIN}
echo PREFIX            is ${PREFIX}
echo TILER_USERSPACE   is ${TILER_USERSPACE}
echo USERSPACE_SYSLINK is ${USERSPACE_SYSLINK}
echo KRNLSRC	       is ${KRNLSRC}

export PATH=${TOOLBIN}:$PATH
export PKG_CONFIG_PATH=$PREFIX/target/lib/pkgconfig

#.. find libgcc
TOOL=`which ${HOST}-gcc`
TOOLDIR=`dirname $TOOL`
LIBGCC=$TOOLDIR/../lib/gcc/$HOST/*/libgcc.a
if [ ! -e $LIBGCC ]
then
	echo "Could not find libgcc.a"
exit
fi
echo Found libgcc.a in $LIBGCC
LIBRT=$TOOLDIR/../$HOST/libc/lib/librt*.so
if [ ! -e $LIBRT ]
then
	echo "Could not find librt.so"
exit
fi
echo Found librt.so in $LIBRT
LIBPTHREAD=$TOOLDIR/../$HOST/libc/lib/libpthread*.so
if [ ! -e $LIBPTHREAD ]
then
	echo "Could not find libpthread.so"
exit
fi
echo Found libpthread.so in $LIBPTHREAD

#... Uncomment below if you want to enable DEBUG option.
# ENABLE_DEBUG=--enable-debug

build_syslink()
{
	# Building memmgr
	echo "							   "
	echo "*****************************************************"
	echo "        Building tiler memmgr APIs and Samples	   "
	echo "*****************************************************"
	echo "							   "
	cd ${TILER_USERSPACE}/memmgr
	./bootstrap.sh
	./configure --prefix ${PREFIX}/target --bindir ${PREFIX}/target/syslink \
	--host ${HOST} --build i686-pc-linux-gnu
	make clean > /dev/null 2>&1
	make
	if [[ $? -ne 0 ]] ; then
	    exit 1
	fi
	make install
	if [[ $? -ne 0 ]] ; then
	    exit 1
	fi
	# Building syslink
	#.. need libgcc.a, librt.so and libpthread.so
	mkdir -p ${PREFIX}/target/lib
	cp $LIBGCC ${PREFIX}/target/lib
	cp `dirname $LIBRT`/librt*.so* ${PREFIX}/target/lib
	cp `dirname $LIBPTHREAD`/libpthread*.so* ${PREFIX}/target/lib
	#.. syslink prefix needs a target subdirectory,
	#so we will create link to the parent
	cd ${USERSPACE_SYSLINK}/syslink
	echo "							  "
	echo "****************************************************"
	echo "      Building Syslink APIs and Samples		  "
	echo "****************************************************"
	echo "							  "
	./bootstrap.sh
	./configure --prefix ${PREFIX}/target --bindir ${PREFIX}/target/syslink \
	--host ${HOST} ${ENABLE_DEBUG}  --build i686-pc-linux-gnu
	export TILER_INC_PATH=${TILER_USERSPACE}/memmgr
	make clean > /dev/null 2>&1
	make
	if [[ $? -ne 0 ]] ; then
	    exit 1
	fi
	make install
	if [[ $? -ne 0 ]] ; then
	    exit 1
	fi
}

build_bridge()
{
	# Building tesla bridge
	echo "							  "
	echo "****************************************************"
	echo "	    Building Bridge APIs and Samples		  "
	echo "****************************************************"
	echo "							  "
	cd ${USERSPACE_SYSLINK}/bridge
	./bootstrap.sh

	./configure --prefix ${PREFIX}/target --bindir ${PREFIX}/target/dspbridge \
	--host ${HOST} ${ENABLE_DEBUG}  --build i686-pc-linux-gnu
	make clean > /dev/null 2>&1
	make
	if [[ $? -ne 0 ]] ; then
	    exit 1
	fi
	make install
	if [[ $? -ne 0 ]] ; then
	    exit 1
	fi
	cd -
}


echo "	"
echo "Following are the 2 Build options available:"
echo "--------------------------------------------"
echo "1--------------> Build Syslink Only"
echo "2--------------> Build Bridge Only"
echo "3--------------> Build Syslink & Bridge"
echo "Any other Option to exit from Build system"
echo "	"
echo "Enter your option:"
read VALUE
case $VALUE in
        1)
		build_syslink ;; # End of case 1
	2)
		build_bridge ;; # End of case 2
	3)
		build_syslink
		build_bridge ;; # End of case 3
	*)	echo " Exiting from the build system....... "
		exit 1
		;;
	esac
