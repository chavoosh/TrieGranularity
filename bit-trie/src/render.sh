#!/bin/bash

#----------------------------------------------------------------------------------
# File: render.sh 
#
# This script creates a pdf file showing the generated bit-trie from the last run
# of the program with option -R enabled. So, if the pdf file of the trie is desired
# you MUST run the program with -R option, otherwise, this script might does not represent
# the trie from a correct run.
# 
# Thus, by running the two following commands, the pdf file of the trie will be ready under
# `./bit-trie/src/dot` directory:
#     $ ./bt -t -R   ///< [-R] options MUST be enabled
#     $ bash ./render.sh
#
# This script runs based on `dot` (to draw the trie) and `xpdf` (to open the pdf) tools.
# If these tools are not installed on the current system, then the script will try to install
# them.
# If the script failed to install these tools (e.g., due to lack of sudo access),
# please manually install the tool and then run the script.
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
#----------------------------------------------------------------------------------

if [ ! -f ./dot/graph.dot ]; then
    echo "File not found!"
    exit 1
fi
if hash dot 2>/dev/null; then
    dot -Tpdf -o ./dot/graph.pdf ./dot/graph.dot
    if hash xpdf 2>/dev/null; then
        xpdf ./dot/graph.pdf
    else
        sudo apt-get install xpdf 
        xpdf ./dot/graph.pdf
    fi
else
    sudo apt-get install dot
    dot -Tpdf -o ./dot/graph.pdf ./dot/graph.dot
    if hash xpdf 2>/dev/null; then
        xpdf ./dot/graph.pdf
    else
        sudo apt-get install xpdf
        xpdf ./dot/graph.pdf
    fi
fi
