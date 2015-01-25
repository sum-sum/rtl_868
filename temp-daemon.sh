#!/bin/bash

#
#    rtl_868
#    Copyright (C) 2015  Clemens Helfmeier
#    e-mail: clemenshelfmeier@gmx.de
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


# temperature daemon.
#
# Send SIGUSR1 to this process to force a restart of the
# receiver and decoder and to select the next output filename
#
# output file is named w.r.t. current time.
#


# trap the SIGUSR1:
function hnd_SIGUSR1() {
  echo "Killing receiver on user request."
  kill $RX_PID
  wait $RX_PID
}
trap hnd_SIGUSR1 SIGUSR1

cd `dirname $0`

while true; do
  # determine output filename
  FN="temp-`date +%Y%m%d-%H%M.csv`"
  echo "Using output filename \"${FN}\"."
  # start the daemon in background
  rtl_fm -f 868.26e6 -M fm -s 500k -r 75k -g 42 -A fast | ./rtl_868 -vvv >>${FN} &
  RX_PID=$!
  # wait for signal or termination
  wait ${RX_PID}
  ERET=$?
  if (( $ERET <= 128 )); then
    echo ""
    echo "Process exited with $ERET.";
    break;
  fi
  echo "Done. Will restart receiver on next loop."
done;

