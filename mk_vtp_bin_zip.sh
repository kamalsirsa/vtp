#!/bin/sh

if [ $# -lt 1 ] ; then
  echo "Usage: mk_vtp_bin_zip.sh date"
  echo
  echo "Example: mk_vtp_bin_zip.sh 021029"
  exit
fi

TARGETDIR=C:/Distrib
DATE=$1
DIST_FILE3=${TARGETDIR}/vtp-apps-bin-${DATE}.zip

# Create the archive containing the App Binaries
rm -f $DIST_FILE3

zip $DIST_FILE3 VTP/TerrainApps/BExtractor/BE.ini
zip $DIST_FILE3 VTP/TerrainApps/BExtractor/BExtractor.exe
zip $DIST_FILE3 VTP/TerrainApps/BExtractor/license.txt
zip $DIST_FILE3 VTP/TerrainApps/BExtractor/Docs/*

zip $DIST_FILE3 VTP/TerrainApps/CManager/CManager_u.exe
zip $DIST_FILE3 VTP/TerrainApps/CManager/CManager.ini
zip $DIST_FILE3 VTP/TerrainApps/CManager/itemtypes.txt
zip $DIST_FILE3 VTP/TerrainApps/CManager/Docs/*

zip $DIST_FILE3 VTP/TerrainApps/Enviro/Enviro.ini
zip $DIST_FILE3 VTP/TerrainApps/Enviro/wxEnviro_u.exe
zip $DIST_FILE3 VTP/TerrainApps/Enviro/license.txt
zip $DIST_FILE3 VTP/TerrainApps/Enviro/Docs/*
zip $DIST_FILE3 VTP/TerrainApps/Enviro/Docs/Navigation/*

zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/license.txt
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/VTBuilder_u.exe
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/VTBuilder.ini
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/Docs/*
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/Docs/images/*

echo $DIST_FILE3 ready.
