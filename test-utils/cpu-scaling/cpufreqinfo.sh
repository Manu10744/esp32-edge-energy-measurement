#!/bin/bash

# ---       INFO        --- #
# This is a script for Linux-based devices that offers to check the CPU frequency settings for each core.
#
# For this purpose, it detects the amount of cores and outputs the scaling_* and cpuinfo_* settings
# for each core.

AMOUNT_OF_CORES=$(ls -d /sys/devices/system/cpu/cpu[0-9]* | wc -l)
echo "Detected $AMOUNT_OF_CORES CPU cores."

COLUMNS="-"
for core in $(seq 1 $AMOUNT_OF_CORES)
do
        COLUMNS+=" -"
done
echo "$COLUMNS"

for i in /sys/devices/system/cpu/cpu0/cpufreq/{cpuinfo,scaling}_*; do
  pname=$(basename $i)
  [[ "$pname" == *available* ]] || [[ "$pname" == *transition* ]] || \
  [[ "$pname" == *driver* ]]    || [[ "$pname" == *setspeed* ]] && continue
  echo "$pname: "

  ITERATIONS=$(expr $AMOUNT_OF_CORES - 1)
  for j in `seq 0 $ITERATIONS`;do
    kparam=$(echo $i | sed "s/cpu0/cpu$j/")
    sudo cat $kparam
  done
done | paste $COLUMNS | column -t