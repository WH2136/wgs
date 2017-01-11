#!/bin/sh
set -e
CWD=$(pwd)
$CWD/00-download.sh
$CWD/01-binutils-pass-1.sh
$CWD/02-gcc-pass-1.sh
$CWD/03-linux-headers.sh
$CWD/04-glibc.sh
$CWD/05-libstdc++.sh
$CWD/06-binutils-pass-2.sh
$CWD/07-gcc-pass-2.sh
$CWD/08-tcl.sh
$CWD/09-expect.sh
$CWD/10-dejagnu.sh
$CWD/11-check.sh
$CWD/12-ncurses.sh
$CWD/13-bash.sh
$CWD/14-bzip2.sh
$CWD/15-coreutils.sh
$CWD/16-diffutils.sh
$CWD/17-file.sh
$CWD/18-findutils.sh
$CWD/19-gawk.sh
$CWD/20-gettext.sh
$CWD/21-grep.sh
$CWD/22-gzip.sh
$CWD/23-m4.sh
$CWD/24-make.sh
$CWD/25-patch.sh
$CWD/26-perl.sh
$CWD/27-sed.sh
$CWD/28-tar.sh
$CWD/29-texinfo.sh
$CWD/30-util-linux.sh
$CWD/31-xz.sh
$CWD/32-strip.sh
$CWD/33-virtual-filesystems.sh
$CWD/34-chroot.sh