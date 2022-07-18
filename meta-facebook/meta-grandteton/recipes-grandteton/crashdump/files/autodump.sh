#!/bin/bash

PID=$$
# File format autodump<fru>.pid (See pal_is_crashdump_ongoing()
# function definition)
PID_FILE='/var/run/autodump1.pid'
DUMP_DIR='/tmp/crashdump/output'
MAP_DIR='/var/bafi'
PARSE_FILE="$DUMP_DIR/bafi.log"
DUMP_FILE="$DUMP_DIR/sensordump.log"
DUMP_UTIL='/usr/bin/crashdump'
PARSE_UTIL='/usr/bin/bafi'
LOG_ARCHIVE='/mnt/data/autodump.tar.gz'
ACD=true

if [ ! -f $DUMP_UTIL ] ; then
  # shellcheck disable=SC1091
  . /usr/local/bin/gpio-utils.sh

  ACD=false
  DUMP_FILE="$DUMP_DIR/autodump.log"
  DUMP_UTIL='/usr/local/bin/dump.sh'
  mkdir -p $DUMP_DIR
fi

# check if running auto dump
[ -r $PID_FILE ] && OLDPID=$(cat $PID_FILE) || OLDPID=''

# Set current pid
echo $PID > $PID_FILE

# Set crashdump timestamp
sys_runtime=$(awk '{print $1}' /proc/uptime)
sys_runtime=$(printf "%0.f" "$sys_runtime")
kv set fru1_crashdump $((sys_runtime+630))

# kill previous autodump if exist
if [ -n "$OLDPID" ] && (grep "autodump" /proc/$OLDPID/cmdline &> /dev/null) ; then
  echo "kill pid $OLDPID..."
  kill -s 9 $OLDPID
  killall -s 9 dump.sh >/dev/null 2>&1
  killall -s 9 peci-util >/dev/null 2>&1
  killall -s 9 crashdump >/dev/null 2>&1
  killall -s 9 bafi >/dev/null 2>&1

  if [ "$ACD" != true ] ; then
    { echo -n "(uncompleted) Auto Dump End at "; date; } >> $DUMP_FILE
    tar zcf '/mnt/data/autodump_uncompleted.tar.gz' -C $DUMP_DIR "$(basename $DUMP_FILE)"
  fi
fi
unset OLDPID

DELAY_SEC=30
SENSOR_HISTORY=180

while test $# -gt 0
do
  case "$1" in
  --now)
    DELAY_SEC=0
    ;;
  *)
    echo "unknown argument $1"
    ;;
  esac
  shift
done

if [ "$DELAY_SEC" != "0" ]; then
  echo "Auto Dump will start after ${DELAY_SEC}s..."
  sleep ${DELAY_SEC}
fi

echo "Auto Dump Started"
LOGS=$(basename $DUMP_FILE)
if [ "$ACD" == true ] ; then
  $DUMP_UTIL

  # shellcheck disable=SC2012
  LOG_FILE=$(ls $DUMP_DIR/crashdump_* -t1 |head -n 1)
  ${PARSE_UTIL} "$LOG_FILE" > $PARSE_FILE 2>&1

  LOGS="$LOGS $(basename "$LOG_FILE") $(basename $PARSE_FILE)"
  { echo -n "Sensor Dump Start at "; date; } > $DUMP_FILE

else

  { echo -n "Auto Dump Start at "; date; } > $DUMP_FILE

  # CPU0
  { $DUMP_UTIL 48 coreid; $DUMP_UTIL 48 msr; } >> $DUMP_FILE 2>&1

  # CPU1
  if [ "$(gpio_get_value FM_CPU1_SKTOCC_LVT3_PLD_N)" -eq "0" ]; then
    { $DUMP_UTIL 49 coreid; $DUMP_UTIL 49 msr; } >> $DUMP_FILE 2>&1
  else
    echo "<<---------- CPU1 is not present, skip it. ---------->>" >> $DUMP_FILE
  fi

  # PCIe
  #$DUMP_UTIL pcie >> $DUMP_FILE 2>&1
fi

# Sensors
{
  echo ""
  echo "Sensor history of last ${SENSOR_HISTORY}s at dump:"
  /usr/local/bin/sensor-util all --history $SENSOR_HISTORY

  echo "Sensor threshold at dump:"
  /usr/local/bin/sensor-util all --threshold

  echo -n "Dump End at "; date
} >> $DUMP_FILE 2>&1

# shellcheck disable=SC2086
tar zcf $LOG_ARCHIVE -C $DUMP_DIR $LOGS
logger -t "ipmid" -p daemon.crit "Crashdump for FRU: 1 is generated at $LOG_ARCHIVE"

# Remove current pid file
rm $PID_FILE

echo "Auto Dump Stored in $LOG_ARCHIVE"
exit 0
