//
// VTConcert.cpp
//
// A very simply command-line tool to convert from any VTP-supported elevation
// format to a BT file.
//
// Copyright (c) 2003-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtdata/ElevationGrid.h"
#include "vtdata/FilePath.h"

void print_help()
{
	printf("VTConvert, a command-line tool for converting geodata.\n");
	printf("Currently, it just converts elevation data, from any format, to the BT format.\n");
	printf(" Build: ");
#if DEBUG
	printf("Debug");
#else
	printf("Release");
#endif
	printf(", date: %s\n\n", __DATE__);

	printf("Command-line options:\n");
	printf("  -in infile       Indicates the input file.\n");
	printf("  -out outfile     Indicates the output file.\n");
	printf("  -gzip            Write output directly to a .gz file\n");
	printf("\n");
	printf("If outfile is not specified, it is derived from infile.\n");
	printf("If outfile has a trailing slash, it is assumed to be a\n"
		" directory, the output has it's name derived from infile,\n"
		" and is written into that directory.\n");
	printf("\n");
}

void main(int argc, char **argv)
{
	vtString str, fname_in, fname_out;
	bool bGZip = false;

	for (int i = 0; i < argc; i++)
	{
		str = argv[i];
		if (str == "-in")
		{
			fname_in = argv[i+1];
			i++;
		}
		else if (str == "-out")
		{
			fname_out = argv[i+1];
			i++;
		}
		else if (str.Left(2) == "-h")
		{
			print_help();
			return;
		}
		else if (str == "-gzip")
		{
			bGZip = true;
		}
	}
	if (fname_in == "")
	{
		printf("Didn't get an input name.  Try -h for help.\n");
		return;
	}

	// Check if output is a directory
	vtString last = fname_out.Right(1);
	if (last == "/" || last == "\\")
	{
		// Use it as the base of the output path
		fname_out += fname_in;
		RemoveFileExtensions(fname_out);
	}
	else if (fname_out == "")
	{
		// Derive output name, if not given, from input.
		fname_out = fname_in;
		RemoveFileExtensions(fname_out);
	}

	// Add extension, if not present
	if (bGZip)
	{
		if (fname_out.Right(6).CompareNoCase(".bt.gz"))
			fname_out += ".bt.gz";
	}
	else
	{
		if (fname_out.Right(3).CompareNoCase(".bt"))
			fname_out += ".bt";
	}

	vtElevationGrid grid;
	if (grid.LoadFromFile(fname_in))
	{
		if (grid.SaveToBT(fname_out, NULL, bGZip))
		{
			int col, row;
			grid.GetDimensions(col, row);
			printf("Successfully wrote elevation, grid size %d x %d.\n", col, row);
		}
		else
			printf("Failed to write output file.\n");
	}
	else
	{
		printf("Failed to read elevation data from %s\n", (const char *) fname_in);
	}
}
