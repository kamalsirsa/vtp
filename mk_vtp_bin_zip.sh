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

cp VTP/TerrainApps/BExtractor/Release/BExtractor.exe VTP/TerrainApps/BExtractor
zip $DIST_FILE3 VTP/TerrainApps/BExtractor/BE.ini
zip $DIST_FILE3 VTP/TerrainApps/BExtractor/BExtractor.exe
zip $DIST_FILE3 VTP/TerrainApps/BExtractor/license.txt
zip $DIST_FILE3 VTP/TerrainApps/BExtractor/Docs/*

cp VTP/TerrainApps/CManager/Release_Unicode/CManager_u.exe VTP/TerrainApps/CManager
zip $DIST_FILE3 VTP/TerrainApps/CManager/CManager_u.exe
zip $DIST_FILE3 VTP/TerrainApps/CManager/itemtypes.txt
zip $DIST_FILE3 VTP/TerrainApps/CManager/Docs/*

cp VTP/TerrainApps/Enviro/Release_Unicode/wxEnviro_u.exe VTP/TerrainApps/Enviro
zip $DIST_FILE3 VTP/TerrainApps/Enviro/wxEnviro_u.exe
zip $DIST_FILE3 VTP/TerrainApps/Enviro/license.txt
zip $DIST_FILE3 VTP/TerrainApps/Enviro/Docs/*
zip $DIST_FILE3 VTP/TerrainApps/Enviro/Docs/Navigation/*

# deal specially with Enviro.ini
mv VTP/TerrainApps/Enviro/Enviro.ini VTP/TerrainApps/Enviro/Enviro_dev.ini
cp VTP/TerrainApps/Enviro/Ship/Enviro.ini VTP/TerrainApps/Enviro
zip $DIST_FILE3 VTP/TerrainApps/Enviro/Enviro.ini
rm VTP/TerrainApps/Enviro/Enviro.ini
mv VTP/TerrainApps/Enviro/Enviro_dev.ini VTP/TerrainApps/Enviro/Enviro.ini

# Enviro locale files
zip $DIST_FILE3 VTP/TerrainApps/Enviro/af/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/de/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/hu/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/pt/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/ro/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/sv/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/zh/Enviro.mo

cp VTP/TerrainApps/VTBuilder/Release_Unicode/VTBuilder_u.exe VTP/TerrainApps/VTBuilder
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/license.txt
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/VTBuilder_u.exe
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/VTBuilder.ini
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/Docs/*
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/Docs/images/*

# VTBuilder locale files
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/zh/VTBuilder.mo

# VTConvert
cp VTP/TerrainApps/VTConvert/Release/VTConvert.exe VTP/TerrainApps/VTConvert
zip $DIST_FILE3 VTP/TerrainApps/VTConvert/VTConvert.exe

#deal specially with docs
mv VTP/Docs/*.html VTP/TerrainApps
zip $DIST_FILE3 VTP/TerrainApps/*.html
mv VTP/TerrainApps/*.html VTP/Docs

echo $DIST_FILE3 ready.
