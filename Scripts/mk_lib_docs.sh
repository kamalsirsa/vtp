#!/bin/sh

TERRAINSDK=/cygdrive/c/VTP/TerrainSDK

echo "Making vtdata docs with doxygen.."
cd $TERRAINSDK/vtdata
doxygen vtdata.cfg

echo "Making vtlib docs with doxygen.."
cd $TERRAINSDK/vtlib
doxygen vtlib.cfg

