#----------------------------------------------------------------------------------
# File: Makefile
#
# Note: This Makefile needs gcc compiler.
#
# Description: This file should be used to compile src files.  
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# (c) 2017 University of Arizona & University of Michigan
#
#-------------------------------------------------------------------------------------
IDIR= ../include
CC= gcc
CFLAGS= -I $(IDIR) -Wall -std=gnu99 -g -funsigned-char -m32

OSTYPE = $(shell uname)

ifeq ($(OSTYPE),CYGWIN_NT-5.1)
ARCH = -D_CYGWIN_
endif

ifeq ($(OSTYPE),Linux)
ARCH = -D_LINUX_
SOCK = -lnsl -lresolv
endif

ifeq ($(OSTYPE),SunOS)
ARCH =  -D_SOLARIS_
SOCK = -lnsl -lsocket -lresolv
endif

ifeq ($(OSTYPE),Darwin)
ARCH = -D_DARWIN_
SOCK = -lresolv
endif

# -- [TODO] OSTYPE routines -- 
ODIR= obj
LDIR= ../lib
_DEPS= name_stat.h
DEPS= $(patsubst %,$(IDIR)/%,$(_DEPS))

SRC= main.c name_stat.c parsing.c
OBJ= $(patsubst %.c,$(ODIR)/%.o,$(SRC))

ng: $(OBJ) 
	$(CC) -o $@ $^ $(CFLAGS)

# -- out of src directory --
ng_: $(OBJ)
	$(CC) -o ../$(patsubst %_,%,$@) $^ $(CFLAGS)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

$(info ************ [-i URL file path] [-n number of input names] [-m number of new names to be generated][-o  optional] **********)
