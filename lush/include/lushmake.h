/***********************************************************************
 * 
 *  LUSH Lisp Universal Shell
 *    Copyright (C) 2002 Leon Bottou, Yann Le Cun, AT&T Corp, NECI.
 *  Includes parts of TL3:
 *    Copyright (C) 1987-1999 Leon Bottou and Neuristique.
 *  Includes selected parts of SN3.2:
 *    Copyright (C) 1991-2001 AT&T Corp.
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA
 * 
 ***********************************************************************/

/***********************************************************************
 * $Id: lushmake.h.in,v 1.2 2003/01/28 18:00:15 leonb Exp $
 **********************************************************************/

#define LUSH_MAKE_MACROS \
  { "SHELL", "/bin/bash" }, \
  { "OPTS", "-DNO_DEBUG -Wall -O3 -march=nocona" }, \
  { "DEFS", "-DHAVE_CONFIG_H" }, \
  { "LIBS", "-lreadline -lcurses -lbfd -liberty -lutil -ldl -lm " }, \
  { "host", "x86_64-unknown-linux-gnu" }, \
  { "CPP", "gcc -E" }, \
  { "CPPFLAGS", "" }, \
  { "CC", "gcc" }, \
  { "GCC", "yes" }, \
  { "CFLAGS", " -Wno-write-strings" }, \
  { "CXX", "g++" }, \
  { "CXXFLAGS", "-g -O2" }, \
  { "F77", "" }, \
  { "FFLAGS", "" }, \
  { "LDCC", "g++" }, \
  { "LDFLAGS", "" }, \
  { "PTHREAD_FLAGS", "-pthread" }, \
  { "PTHREAD_LIBS", "" }, \
  { "AR", "/usr/bin/ar" }, \
  { "MV", "/bin/mv" }, \
  { "CP", "/bin/cp" }, \
  { "INDENT", "/usr/bin/indent" }, \
  { "LN_S", "ln -s" }, \
  { "TOUCH", "/usr/bin/touch" }, \
  { "RANLIB", "ranlib" }, \
  { "X_LIBS", "  -lSM -lICE -lX11 " }, \
  { "X_CFLAGS", "" }, \
  { "CC_PIC_FLAG", "-fPIC" }, \
  { "CC_EXP_FLAG", "-Wl,-export-dynamic" }, \
  { "MAKESO", "g++ -shared -o" }, \
  { "EXEEXT", "" }, \
  { "OBJEXT", "o" }, \
  { "SOEXT", "so" }
