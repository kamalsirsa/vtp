#!/bin/sh

TERRAINAPPS=/cygdrive/c/VTP/TerrainApps

lftp -c "open vterrain.org; \
		 user vterrain; \
		 cd public_html/Doc; \
		 lcd $TERRAINAPPS; \
		 mirror -R --verbose --continue --no-perms --exclude CVS BExtractor/Docs BExtractor; \
		 mirror -R --verbose --continue --no-perms --exclude CVS Enviro/Docs Enviro; \
		 mirror -R --verbose --continue --no-perms --exclude CVS VTBuilder/Docs VTBuilder"
