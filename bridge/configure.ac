#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(bridge, 0.1, http://www.ti.com)
AC_CONFIG_SRCDIR([api/inc/DSPManager.h])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.
AC_SEARCH_LIBS([pthread_create], [pthread])
AC_PROG_LIBTOOL
LT_INIT

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h malloc.h stdint.h stdlib.h string.h sys/file.h sys/ioctl.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_C_VOLATILE

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([getpagesize gettimeofday memset munmap strchr])

# Currently Build for 3430 Proc Family. Please change this
# to build it for other Proc Family
AC_SUBST(PROCFAMILY, OMAP_3430)
AM_CONDITIONAL(PROCFAMILY_IS_4430, [test "x$PROCFAMILY" = xOMAP_4430])
AM_CONDITIONAL(PROCFAMILY_IS_3430, [test "x$PROCFAMILY" = xOMAP_3430])
AM_CONDITIONAL(PROCFAMILY_IS_2430, [test "x$PROCFAMILY" = xOMAP_2430])


#Compiling with Debug enabled by default. Change it to false to disable.
default_debug=true
AC_ARG_ENABLE([debug],
     [  --enable-debug          Turn on debugging,[default=true]],
     [case "${enableval}" in
       yes) debug=true
	    CFLAGS="$CFLAGS -g";;
       no)  debug=false
	    CFLAGS="$CFLAGS  -fomit-frame-pointer";;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
     esac],[debug=$default_debug])
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])

#Warning Flags
CFLAGS="$CFLAGS  -Wall  -Wno-trigraphs -Werror-implicit-function-declaration "

#Optimization Flags
CFLAGS="$CFLAGS -fno-strict-aliasing"

if test x$PROCFAMILY = xOMAP_3430; then
CFLAGS="$CFLAGS -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -march=armv7-a -msoft-float -Uarm -D__LINUX_ARM_ARCH__=7"
fi

if test x$PROCFAMILY = xOMAP_4430; then
CFLAGS="$CFLAGS  -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -march=armv7-a -msoft-float -Uarm -D__LINUX_ARM_ARCH__=7 "
fi

# Code generation flags
CFLAGS="$CFLAGS -fno-common"
#Macros
CFLAGS="$CFLAGS -DLINUX"

#if __KERNEL__
#CFLAGS="$CFLAGS -D__KERNEL__  -fno-builtin"
#endif


AC_CONFIG_FILES([Makefile
                 api/Makefile
                 api/inc/Makefile
                 api/make/Makefile
                 api/src/Makefile
                 api/src/bridge/Makefile
                 api/src/qos/Makefile
                 samples/Makefile
                 samples/inc/Makefile
                 samples/make/Makefile
                 samples/src/Makefile
                 samples/src/cexec/Makefile
                 samples/src/crashmon/Makefile
                 samples/src/dmmcopy/Makefile
                 samples/src/dynreg/Makefile
                 samples/src/ping/Makefile
                 samples/src/qostest/Makefile
                 samples/src/scale/Makefile
                 samples/src/strmcopy/Makefile
                 samples/src/zerocopymsg/Makefile])
AC_OUTPUT

echo "
  Bridge Database Library ($PACKAGE_NAME) version $PACKAGE_VERSION
  Prefix.........: $prefix
  DEBUG..........: $debug
  Procfamily.....: $PROCFAMILY
  C Compiler.....: $CC
  Linker.........: $LD $LDFLAGS $LIBS
"
