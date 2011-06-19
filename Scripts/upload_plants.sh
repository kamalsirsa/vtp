#!/bin/sh

if [ $# -lt 1 ] ; then
  echo "Usage: upload_plants.sh date"
  exit
fi

DATE=$1
PLANTZIP=/cygdrive/c/Distrib/vtp-plantlib-${DATE}.zip
PLANTXML=/cygdrive/c/VTP/Scripts/plant_list.html

lftp -c "open sftp://vterrain@vterrain.org; \
		 put -O public_html/Implementation/Plants ${PLANTXML}; \
		 put -O public_html/dist ${PLANTZIP}"
