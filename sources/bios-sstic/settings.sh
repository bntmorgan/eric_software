#!/bin/bash
# Xilinx Webpack 14.1
XILINX_BASE="/home/csauvana/bin/Xilinx"
XILINX_VERSION="14.4"
export LD_PRELOAD=/home/csauvana/bin/usb-driver/libusb-driver.so 

export XILINXD_LICENSE_FILE=2100@flexlml.laas.fr
export XIL_IMPACT_USE_LIBUSB=1
source ${XILINX_BASE}/${XILINX_VERSION}/ISE_DS/settings64.sh

# rtems
RTEMS_BASE="/opt/rtems-4.11"
export PATH=$RTEMS_BASE/bin:$PATH
export CROSS_COMPILER=lm32-rtems4.11-

# Milkymist
export BOARD=pico-e17
#Source issues
make 
