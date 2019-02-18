#----------------------------------------------------------------------------------
# File: Makefile 
#
# gcc compiler is required.
# 
# Description: This file shold be used to compile src files of character-trie structure.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# (c) 2018 - 2019 University of Arizona & University of Michigan
#-------------------------------------------------------------------------------------

IDIR= ../include
CC= gcc
CFLAGS= -I $(IDIR) -Wall -std=gnu99 -g -funsigned-char

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
_DEPS= ht_hashtable.h Bt_trie.h db_debug.h db_debug_struct.h main.h
DEPS= $(patsubst %,$(IDIR)/%,$(_DEPS))

SRC= main.c Bt_trie.c db_debug.c ht_hashtable.c
OBJ= $(patsubst %.c,$(ODIR)/%.o,$(SRC))

Bt: $(OBJ) 
	$(CC) -o $@ $^ $(CFLAGS)

# -- out of src directory --
Bt_: $(OBJ)
	$(CC) -o ../$(patsubst %_,%,$@) $^ $(CFLAGS)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

