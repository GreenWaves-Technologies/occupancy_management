#!/usr/bin/env /bash

### Copyright (C) 2020 GreenWaves Technologies
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.

CURRENT_PATH=`pwd`


echo "Select if you want to burn boot from HYPERFLASH or SPIFLASH: "
echo -e "\t1 - HYPERFLASH"
echo -e "\t2 - SPIFLASH"

read board

read -p "Are you sure? This operation cannot be undone [Yy]: " -n 1 -r
echo    # (optional) move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    if [ "$board" == "1" ]
	then
		cd $GAP_SDK_HOME//utils/openocd_tools/src/fuser
		make clean all run revc_hyper=1
		cd $CURRENT_PATH
	elif [ "$board" == "2" ]
	then
		cd $GAP_SDK_HOME//utils/openocd_tools/src/fuser
		make clean all run revc_qspi=1
		cd $CURRENT_PATH
	else
		echo "Selection Error"
	fi
fi


