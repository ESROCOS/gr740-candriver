#!/bin/sh

# Have to set include path for the compilers to find the correct header files oO
ORCHESTRATOR_OPTIONS+=" -e gr740_partition:/opt/rtems-5.1-2018.03.08/sparc-rtems5/gr740/lib/include"
CFLAGS+="-DCONFIGURE_DRIVER_AMBAPP_GAISLER_GRCAN"

echo "options: $ORCHESTRATOR_OPTIONS"
