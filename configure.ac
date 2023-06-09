#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT(qball, alsos, xavier@llnl.gov)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/ui/FoldInWsCmd.h])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX(mpic++)
AX_CXX_COMPILE_STDCXX([11])
AC_PROG_CC(mpicc)
AC_PROG_CC_C99
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_C_BIGENDIAN([acx_bigendian=yes], [acx_bigendian=no])

if test x$acx_bigendian == xyes; then
  AC_DEFINE(WORDS_BIGENDIAN, 1, [the architecture is bigendian])
fi

AC_SYS_LARGEFILE

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FSEEKO
AC_FUNC_STRTOD
AC_CHECK_FUNCS([gettimeofday memset mkdir pow sqrt strspn uname])

AC_MSG_NOTICE([
================================================================================
  CHECKING FOR FORTRAN LIBRARIES
================================================================================])
dnl We need to check for fortran, to link with blas and other libraries
AC_PROG_FC(mpif90)
AC_FC_LIBRARY_LDFLAGS
AC_FC_WRAPPERS

AC_MSG_NOTICE([
================================================================================
  CHECKING FOR OPENMP
================================================================================])

# Checks for OpenMP
AC_LANG_PUSH(C)
AX_OPENMP
CXXFLAGS="$CXXFLAGS $OPENMP_CXXFLAGS"
AC_LANG_POP(C)

# Checks for OpenMP
AC_LANG_PUSH(C++)
AX_OPENMP
CXXFLAGS="$CXXFLAGS $OPENMP_CXXFLAGS"
AC_LANG_POP(C++)

# Checks for OpenMP
AC_LANG_PUSH(Fortran)
AX_OPENMP
CXXFLAGS="$CXXFLAGS $OPENMP_CXXFLAGS"
AC_LANG_POP(Fortran)


#LIBRARIES
AC_MSG_NOTICE([
================================================================================
  CHECKING FOR MPI
================================================================================])

acx_mpi_ok=yes
AC_LANG_PUSH(C++)
AC_CHECK_HEADERS([mpi.h], [], [acx_mpi_ok=no])
AC_CHECK_FUNCS([MPI_Init], [], [acx_mpi_ok=no])
AC_LANG_POP(C++)
AC_MSG_CHECKING(whether we can use MPI)
AC_MSG_RESULT($acx_mpi_ok)

if test $acx_mpi_ok == yes; then
   AC_DEFINE(USE_MPI, 1, [enable MPI support])
else
   AC_MSG_ERROR([
   
   Cannot find the required MPI library. If you have an MPI Implementation
   installed you might need to set CC=mpicc and CXX=mpic++.
   ])
fi

AC_DEFINE(ARCH, "autotools", [architecture])

AC_MSG_NOTICE([
================================================================================
  CHECKING FOR FFT LIBRARIES
================================================================================])

found_fft=no

#FFTW3
if test x$found_fft != xyes ; then
  ACX_ESSL_FFT
fi

if test x$acx_essl_fft_ok == xyes ; then
  found_fft=yes
  fft_lib=essl
  CFLAGS="$CFLAGS $CFLAGS_ESSL_FFT"
  CXXFLAGS="$CXXFLAGS $CFLAGS_ESSL_FFT"
  LIBS="$LIBS $LIBS_FFTW3"
fi

#FFTW3
if test x$found_fft != xyes ; then
  ACX_FFTW3
fi

if test x$acx_fftw3_ok == xyes ; then
  found_fft=yes
  fft_lib=fftw3
  CFLAGS="$CFLAGS $CFLAGS_FFTW3"
  CXXFLAGS="$CXXFLAGS $CFLAGS_FFTW3"
  LIBS="$LIBS $LIBS_FFTW3"
fi

#FFTW2
if test x$found_fft != xyes ; then
   ACX_FFTW2
fi

if test x$acx_fftw2_ok == xyes ; then
  found_fft=yes
  fft_lib=fftw2
  CFLAGS="$CFLAGS $CFLAGS_FFTW2"
  CXXFLAGS="$CXXFLAGS $CFLAGS_FFTW2"
  LIBS="$LIBS $LIBS_FFTW2"
fi

#INTERNAL LIBRARY
if test x$found_fft != xyes ; then
  AC_MSG_WARN([

   Cannot find a suitable FFT library, using _slower_ internal FFT routines.

  ])
  fft_lib=internal  
  AC_DEFINE(FFT_NOLIB, 1, [no FFT library])
fi

AC_MSG_NOTICE([
================================================================================
  CHECKING FOR REQUIRED LINEAR ALGEBRA LIBRARIES (BLAS AND LAPACK)
================================================================================])
dnl check for BLAS
ACX_BLAS([], AC_MSG_ERROR([could not find required BLAS library]))

dnl check for LAPACK
ACX_LAPACK([], AC_MSG_ERROR([could not find required LAPACK library]))

AC_MSG_NOTICE([
================================================================================
  CHECKING FOR REQUIRED PARALLEL LINEAR ALGEBRA LIBRARIES (BLACS AND SCALAPACK)
================================================================================])

ACX_BLACS
ACX_SCALAPACK
ACX_SCALAPACK2

LIBS="$LIBS_SCALAPACK $LIBS_BLACS $LIBS_LAPACK $LIBS_BLAS $LIBS $FCLIBS"

AC_MSG_NOTICE([
================================================================================
  IBM MASS LIBRARY
================================================================================])

dnl For the moment we do not use MASS because it conflicts with xlfmath
dnl AC_CHECK_LIB(mass, sincos)

ACX_MASSV

if test x$acx_massv_ok == xyes; then
  LIBS="$LIBS_MASSV $LIBS"
fi

AC_MSG_NOTICE([
================================================================================
  LIBXC LIBRARY
================================================================================])

ACX_LIBXC

if test x$acx_libxc_ok == xyes; then
  LIBS="$LIBS_LIBXC $LIBS"
  CFLAGS="$CFLAGS $CFLAGS_LIBXC"
  CXXFLAGS="$CXXFLAGS $CFLAGS_LIBXC"
fi

AC_MSG_NOTICE([
================================================================================
  CHECKING FOR BLUEGENE/Q LIBRARIES
================================================================================])

AC_CHECK_HEADERS([spi/include/kernel/location.h bgpm/include/bgpm.h], [acx_bgq=yes], [acx_bgq=no])

if test x$acx_bgq == xyes; then
  AC_DEFINE(HAVE_BGQLIBS, 1, [whether BlueGene/Q libraries are available])
fi

AC_MSG_NOTICE([
================================================================================
  CONFIGURATION DONE, GENERATING MAKEFILES
================================================================================])
AC_CONFIG_FILES([Makefile
                 external_libs/Makefile
		 external_libs/dftd3/Makefile
 		 external_libs/rapidxml/Makefile
		 share/Makefile
		 share/pseudopotentials/Makefile
 		 share/pseudopotentials/pseudo-dojo.org/Makefile
 		 share/pseudopotentials/pseudo-dojo.org/nc-sr-04_pbe_standard/Makefile
 		 share/pseudopotentials/pseudo-dojo.org/nc-sr-04_pbe_stringent/Makefile
 		 share/pseudopotentials/pseudo-dojo.org/nc-sr-04_pbesol_standard/Makefile
 		 share/pseudopotentials/pseudo-dojo.org/nc-sr-04_pbesol_stringent/Makefile
  		 share/pseudopotentials/pseudo-dojo.org/nc-sr-04_pw_standard/Makefile
 		 share/pseudopotentials/pseudo-dojo.org/nc-sr-04_pw_stringent/Makefile
 		 share/pseudopotentials/quantum-simulation.org/Makefile
  		 share/pseudopotentials/quantum-simulation.org/hscv/Makefile
   		 share/pseudopotentials/quantum-simulation.org/hscv/lda/Makefile
   		 share/pseudopotentials/quantum-simulation.org/hscv/pbe/Makefile
 		 share/pseudopotentials/quantum-simulation.org/sg15/Makefile
 		 share/pseudopotentials/quantum-simulation.org/sg15/convergence/Makefile
                 src/Makefile
		 src/math/Makefile
                 src/pseudo/Makefile
                 src/functionals/Makefile
		 src/ui/Makefile
		 src/vars/Makefile
		 src/qball/Makefile
 		 src/main/Makefile
		 src/tests/Makefile
                 testsuite/Makefile])

AC_OUTPUT

AC_MSG_NOTICE([

================================================================================
  CONFIGURATION SUMMARY
================================================================================

C compiler       :  $CC $CFLAGS
C++ compiler     :  $CXX $CXXFLAGS
Fortran compiler :  $FC $FCFLAGS
Big endian       :  $acx_bigendian
MPI              :  $acx_mpi_ok
FFT library      :  $fft_lib
BLAS             :  $LIBS_BLAS
LAPACK           :  $LIBS_LAPACK
BLACS            :  $LIBS_BLACS
SCALAPACK        :  $acx_scalapack_ok ($LIBS_SCALAPACK)
SCALAPACK 2      :  $acx_scalapack2_ok
MASSV            :  $acx_massv_ok
BlueGene/Q       :  $acx_bgq
LIBXC            :  $acx_libxc_ok
])



