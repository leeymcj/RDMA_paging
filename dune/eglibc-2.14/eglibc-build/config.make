# config.make.  Generated from config.make.in by configure.
# Don't edit this file.  Put configuration parameters in configparms instead.

version = 2.14.1
release = stable

# Installation prefixes.
install_root =
prefix = /usr/local
exec_prefix = ${prefix}
datadir = ${datarootdir}
libdir = ${exec_prefix}/lib
slibdir = 
localedir = 
sysconfdir = ${prefix}/etc
libexecdir = ${exec_prefix}/libexec
rootsbindir = 
infodir = ${datarootdir}/info
includedir = ${prefix}/include
datarootdir = ${prefix}/share

# Should we use and build ldconfig?
use-ldconfig = yes

# Maybe the `ldd' script must be rewritten.
ldd-rewrite-script = ../sysdeps/unix/sysv/linux/x86_64/ldd-rewrite.sed

# System configuration.
config-machine = x86_64
base-machine = x86_64
config-vendor = unknown
config-os = linux-gnu
config-sysdirs =  sysdeps/x86_64/elf nptl/sysdeps/unix/sysv/linux/x86_64 sysdeps/unix/sysv/linux/x86_64 sysdeps/unix/sysv/linux/wordsize-64 nptl/sysdeps/unix/sysv/linux nptl/sysdeps/pthread sysdeps/pthread sysdeps/unix/sysv/linux sysdeps/gnu sysdeps/unix/common sysdeps/unix/mman sysdeps/unix/inet nptl/sysdeps/unix/sysv sysdeps/unix/sysv sysdeps/unix/x86_64 nptl/sysdeps/unix sysdeps/unix sysdeps/posix sysdeps/x86_64/fpu sysdeps/x86_64/multiarch nptl/sysdeps/x86_64 sysdeps/x86_64 sysdeps/wordsize-64 sysdeps/ieee754/ldbl-96 sysdeps/ieee754/dbl-64/wordsize-64 sysdeps/ieee754/dbl-64 sysdeps/ieee754/flt-32 sysdeps/ieee754 sysdeps/generic/elf sysdeps/generic
cflags-cpu = 
asflags-cpu = 

config-cflags-sse4 = yes
config-cflags-avx = yes
config-cflags-novzeroupper = yes
config-asflags-i686 = yes

defines =  -D_LIBC_REENTRANT
sysincludes = 
c++-sysincludes = 
all-warnings = 

elf = yes
have-z-combreloc = yes
have-z-execstack = yes
have-initfini = 
have-Bgroup = yes
have-as-needed = yes
libgcc_s_suffix = 
need-nopic-initfini = 
with-fp = yes
with-cvs = yes
old-glibc-headers = no
unwind-find-fde = no
have-cpp-asm-debuginfo = yes
enable-check-abi = no
have-forced-unwind = yes
have-fpie = yes
gnu89-inline-CFLAGS = -fgnu89-inline
have-ssp = yes
have-selinux = yes
have-libaudit = no
have-libcap = no
have-cc-with-libunwind = no
fno-unit-at-a-time = -fno-toplevel-reorder -fno-section-anchors
bind-now = no
have-hash-style = yes

static-libgcc = -static-libgcc

versioning = yes
oldest-abi = default
no-whole-archive = -Wl,--no-whole-archive
exceptions = -fexceptions
multi-arch = default

mach-interface-list = 

have-bash2 = yes
have-ksh = yes

sizeof-long-double = 16

experimental-malloc = 

nss-crypt = no

# Configuration options.
build-static = yes
build-shared = yes
build-pic-default= no
build-profile = no
build-omitfp = no
build-bounded = no
build-static-nss = no
add-ons = libidn nptl
add-on-subdirs =  libidn
sysdeps-add-ons =  nptl
cross-compiling = no
force-install = yes

# Build tools.
CC = gcc
CXX = g++
BUILD_CC = 
CFLAGS = -g -O2
ASFLAGS-config =  -Wa,--noexecstack
AR = ar
NM = nm
MAKEINFO = :
AS = $(CC) -c
MIG = 
PWD_P = /usr/bin/pwd
BISON = /usr/bin/bison
AUTOCONF = autoconf
OBJDUMP = objdump

# Installation tools.
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_SCRIPT = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_INFO = /usr/sbin/install-info
LN_S = ln -s
MSGFMT = msgfmt

# Script execution tools.
BASH = /usr/bin/bash
KSH = /usr/bin/bash
AWK = gawk
PERL = /usr/bin/perl

# Additional libraries.
LIBGD = no

# Package versions and bug reporting configuration.
PKGVERSION = (EGLIBC) 
REPORT_BUGS_TO = <http://www.eglibc.org/issues/>

# More variables may be inserted below by configure.

override stddef.h = # The installed <stddef.h> seems to be libc-friendly.
