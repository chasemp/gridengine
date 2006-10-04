#!/bin/sh

Usage()
{
   echo "monitor.sh [options]"
   echo "options:"
   echo "   -cell       <cell>     use \$SGE_CELL other than \"default\""
   echo "   -interval   <time>     use statistics interval other than \"15sec\""
   echo "   -spooling              show qmaster spooling probes"
   echo "   -requests              show incoming qmaster request probes"
}

# monitor.sh defaults
cell=default
interval=15sec
spooling_probes=0
request_probes=0

while [ $# -gt 0 ]; do
   case "$1" in
      -spooling)
         spooling_probes=1
         shift
         ;;
      -request)
         request_probes=1
         shift
         ;;
      -interval)
         shift
         interval="$1"
         shift
         ;;
      -cell)
         shift
         cell="$1"
         shift
         ;;
      -help)
         Usage
         exit 0
         ;;
      *)
         Usage
         exit 1
         ;;
   esac
done

if [ $SGE_ROOT = "" ]; then
   echo "Please run with \$SGE_ROOT set on master machine"
   exit 1
fi

master=`cat $SGE_ROOT/$cell/spool/qmaster/qmaster.pid`
if [ $? -ne 0 ]; then
   echo "Couldn't read sge_qmaster pid from \$SGE_ROOT/$cell/spool/qmaster/qmaster.pid"
   exit 1
fi
schedd=`cat $SGE_ROOT/$cell/spool/qmaster/schedd/schedd.pid`
if [ $? -ne 0 ]; then
   echo "Couldn't read sge_schedd pid from \$SGE_ROOT/$cell/spool/qmaster/schedd/schedd.pid"
   exit 1
fi

/usr/sbin/dtrace -s $SGE_ROOT/util/dtrace/monitor.d $master $schedd $interval $spooling_probes $request_probes
