# Make x86-64 random byte generators.

# Copyright 2015, 2020 Paul Eggert

# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

# Optimization level.  Change this -O2 to -Og or -O0 or whatever.
OPTIMIZE = -Og

# The C compiler and its options.
CC = gcc
CFLAGS = $(OPTIMIZE) -g3 -Wall -Wextra -fsanitize=undefined \
  -march=native -mtune=native -mrdrnd

# The archiver command, its options and filename extension.
TAR = tar
TARFLAGS = --xz --transform 's,^,randall/,'
TAREXT = txz

default: randall

randall: *.c
	$(CC) $(CFLAGS) *.c -o $@

assignment: randall-assignment.$(TAREXT)
assignment-files = COPYING Makefile randall.c
randall-assignment.$(TAREXT): $(assignment-files)
	$(TAR) $(TARFLAGS) -cf $@ $(assignment-files)

submission: randall-submission.$(TAREXT)
submission-files = $(assignment-files) \
  notes.txt # More files should be listed here, as needed.
randall-submission.$(TAREXT): $(submission-files)
	$(TAR) $(TARFLAGS) -cf $@ $(submission-files)

.PHONY: default clean assignment submission

clean:
	rm -f *.o *.$(TAREXT) randall

check:
	@if [ `./randall 150 | wc -c` -eq 150 ]; \
  then echo "test passed : ./randall 150" ; \
  else echo "test failed : ./randall 150" ; \
  fi ;

check2:
	@if [ `./randall -i rdrand 150 | wc -c` -eq 150 ]; \
  then echo "test passed : ./randall -i rdrand 150" ; \
  else echo "test failed : ./randall -i rdrand 150" ; \
  fi ;

check3:
	@if [ `./randall -i mrand48_r 150 | wc -c` -eq 150 ]; \
  then echo "test passed : ./randall -i mrand48_r 150" ; \
  else echo "test failed : ./randall -i mrand48_r 150" ; \
  fi ;

check4:
	@if [ `./randall -o stdio 150 | wc -c` -eq 150 ]; \
  then echo "test passed : ./randall -o stdio 150" ; \
  else echo "test failed : ./randall -o stdio 150" ; \
  fi ;

check5:
	@if [ `./randall -o 1 2000 | wc -c` -eq 2000 ]; \
  then echo "test passed : ./randall -o 1 2000" ; \
  else echo "test failed : ./randall -o 1 2000" ; \
  fi ;




