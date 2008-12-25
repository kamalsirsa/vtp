#!/bin/sh

if [ $# -lt 1 ] ; then
  echo "Usage: mk_vtp_bin_zip.sh date"
  echo
  echo "Example: mk_vtp_bin_zip.sh 041029"
  exit
fi

SOURCEDIR=/cygdrive/c
TARGETDIR=/cygdrive/c/Distrib
DATE=$1
DIST_FILE3=${TARGETDIR}/vtp-apps-bin-${DATE}.zip

cd $SOURCEDIR

# Create the archive containing the App Binaries
rm -f $DIST_FILE3

#cp VTP/TerrainApps/BExtractor/Release-vc9/BExtractor.exe VTP/TerrainApps/BExtractor
#zip $DIST_FILE3 VTP/TerrainApps/BExtractor/BE.ini
#zip $DIST_FILE3 VTP/TerrainApps/BExtractor/BExtractor.exe
#zip $DIST_FILE3 VTP/TerrainApps/BExtractor/license.txt
#zip $DIST_FILE3 VTP/TerrainApps/BExtractor/Docs/*

cp VTP/TerrainApps/CManager/Release-Unicode-vc9/CManager.exe VTP/TerrainApps/CManager
zip $DIST_FILE3 VTP/TerrainApps/CManager/CManager.exe
zip $DIST_FILE3 VTP/TerrainApps/CManager/itemtypes.txt
zip $DIST_FILE3 VTP/TerrainApps/CManager/tags.txt
zip $DIST_FILE3 VTP/TerrainApps/CManager/Docs/*

cp VTP/TerrainApps/Enviro/Release-Unicode-vc9/wxEnviro.exe VTP/TerrainApps/Enviro
zip $DIST_FILE3 VTP/TerrainApps/Enviro/wxEnviro.exe
zip -r $DIST_FILE3 VTP/TerrainApps/Enviro/Docs -x *.svn

# deal specially with Enviro.xml
mv VTP/TerrainApps/Enviro/Enviro.xml VTP/TerrainApps/Enviro/Enviro_dev.xml
cp VTP/TerrainApps/Enviro/Ship/Enviro.xml VTP/TerrainApps/Enviro
zip $DIST_FILE3 VTP/TerrainApps/Enviro/Enviro.xml
rm VTP/TerrainApps/Enviro/Enviro.xml
mv VTP/TerrainApps/Enviro/Enviro_dev.xml VTP/TerrainApps/Enviro/Enviro.xml

# Enviro locale files
zip $DIST_FILE3 VTP/TerrainApps/Enviro/af/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/ar/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/de/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/en/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/en_GB/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/es/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/fr/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/hu/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/it/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/pt/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/ro/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/sv/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/tr/Enviro.mo
zip $DIST_FILE3 VTP/TerrainApps/Enviro/zh/Enviro.mo

cp VTP/TerrainApps/VTBuilder/Release-Unicode-vc9/VTBuilder.exe VTP/TerrainApps/VTBuilder
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/VTBuilder.exe
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/VTBuilder.xml
zip -r $DIST_FILE3 VTP/TerrainApps/VTBuilder/Docs -x *.svn

# VTBuilder locale files
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/ar/VTBuilder.mo
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/en/VTBuilder.mo
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/fr/VTBuilder.mo
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/ro/VTBuilder.mo
zip $DIST_FILE3 VTP/TerrainApps/VTBuilder/zh/VTBuilder.mo

# VTConvert
cp VTP/TerrainApps/VTConvert/Release-vc9/VTConvert.exe VTP/TerrainApps/VTConvert
zip $DIST_FILE3 VTP/TerrainApps/VTConvert/VTConvert.exe

#deal specially with docs
mv VTP/Docs/*.html VTP/TerrainApps
zip $DIST_FILE3 VTP/TerrainApps/*.html
mv VTP/TerrainApps/*.html VTP/Docs

echo $DIST_FILE3 ready.
