/*
 * Unarchive.cpp - Extract files from a gzipped TAR file
 * adapted from some 'libpng' sample code
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef UNIX
# include <unistd.h>
# include <utime.h>
#endif

#include "config_vtdata.h"
#include "FilePath.h"
#include "Unarchive.h"

#if SUPPORT_UNZIP
#include "ZipArchive.h"
#endif

#include "zlib.h"

#ifdef WIN32
#  ifdef _MSC_VER
#	define unlink(fn)		  _unlink(fn)
#  endif
#endif


/* Values used in typeflag field.  */
#define REGTYPE		'0'		/* regular file */
#define AREGTYPE	'\0'	/* regular file */
#define DIRTYPE		'5'		/* directory */

#define BLOCKSIZE 512

struct tar_header
{						/* byte offset */
	char name[100];		/*	0 */
	char mode[8];		/* 100 */
	char uid[8];		/* 108 */
	char gid[8];		/* 116 */
	char size[12];		/* 124 */
	char mtime[12];		/* 136 */
	char chksum[8];		/* 148 */
	char typeflag;		/* 156 */
	char linkname[100];	/* 157 */
	char magic[6];		/* 257 */
	char version[2];	/* 263 */
	char uname[32];		/* 265 */
	char gname[32];		/* 297 */
	char devmajor[8];	/* 329 */
	char devminor[8];	/* 337 */
	char prefix[155];	/* 345 */
						/* 500 */
};

union tar_buffer {
	char				buffer[BLOCKSIZE];
	struct tar_header	header;
};

/* helper functions */
int getoct(char *p,int width)
{
	int result = 0;
	char c;

	while (width --)
	{
		c = *p++;
		if (c == ' ')
			continue;
		if (c == 0)
			break;
		result = result * 8 + (c - '0');
	}
	return result;
}

/**
 * Unarchives the indicated tarred, gzipped, or gzipped tar file.
 * Each directory and file in the archive is created.
 *
 * \param prepend_path A string to be prepended to all output filenames.
 *
 * \return -1 on error, otherwise the number of files the archive contained.
 */
int ExpandTGZ(const char *archive_fname, const char *prepend_path)
{
	gzFile	in;
	union	tar_buffer buffer;
	int		len;
	int		getheader = 1;
	int		remaining = 0;
	FILE	*outfile = NULL;
	char	fname[BLOCKSIZE];
	char	fullname[1024];
	time_t	tartime;
	int		files_encountered = 0;

	/*
	 *  Process the TGZ file
	 */
	in = gzopen(archive_fname, "rb");
	if (in == NULL)
	{
//		fprintf(stderr,"%s: Couldn't gzopen %s\n", prog, TGZfile);
		return -1;
	}

	while (1)
	{
		len = gzread(in, &buffer, BLOCKSIZE);
		if (len < 0)
		{
			// error (gzerror(in, &err));
			return -1;
		}
		/*
		* Always expect complete blocks to process
		* the tar information.
		*/
		if (len != BLOCKSIZE)
		{
			// error("gzread: incomplete block read");
			gzclose(in);
			return -1;
		}

		/*
		* If we have to get a tar header
		*/
		if (getheader == 1)
		{
			/*
			 * if we met the end of the tar
			 * or the end-of-tar block,
			 * we are done
			 */
			if ((len == 0) || (buffer.header.name[0]== 0)) break;

			tartime = (time_t)getoct(buffer.header.mtime, 12);
			strncpy(fname, buffer.header.name, BLOCKSIZE);

			strcpy(fullname, prepend_path);
			strcat(fullname, fname);

			switch (buffer.header.typeflag)
			{
			case DIRTYPE:
				vtCreateDir(fullname);
				break;
			case REGTYPE:
			case AREGTYPE:
				remaining = getoct(buffer.header.size, 12);
				if (remaining)
				{
					outfile = fopen(fullname,"wb");
					if (outfile == NULL)
					{
						/* try creating directory */
						char *p = strrchr(fullname, '/');
						if (p != NULL)
						{
							*p = '\0';
							vtCreateDir(fullname);
							*p = '/';
							outfile = fopen(fullname,"wb");
						}
					}
//					fprintf(stderr, "%s %s\n", (outfile) ? "Extracting" : "Couldn't create", fname);
					files_encountered++;
				}
				else
					outfile = NULL;
				/*
				 * could have no contents
				 */
				getheader = (remaining) ? 0 : 1;
				break;
			default:
				break;
			}
		}
		else
		{
			unsigned int bytes = (remaining > BLOCKSIZE) ? BLOCKSIZE : remaining;

			if (outfile != NULL)
			{
				if (fwrite(&buffer,sizeof(char),bytes,outfile) != bytes)
				{
//					fprintf(stderr,"%s : error writing %s skipping...\n",prog,fname);
					fclose(outfile);
					unlink(fname);
				}
			}
			remaining -= bytes;
			if (remaining == 0)
			{
				getheader = 1;
				if (outfile != NULL)
				{
#ifdef WIN32
					fclose(outfile);

					outfile = NULL;
#else
					struct utimbuf settime;

					settime.actime = settime.modtime = tartime;

					fclose(outfile);
					outfile = NULL;
					utime(fname,&settime);
#endif
				}
			}
		}
	}

	if (gzclose(in) != Z_OK)
	{
		// error("failed gzclose");
		return -1;
	}

	return files_encountered;
}

/**
 * Unarchives the indicated zipped file.
 * Each directory and file in the archive is created.
 *
 * \param prepend_path A string to be prepended to all output filenames.
 *
 * \return -1 on error, otherwise the number of files the archive contained.
 */
int ExpandZip(const char *archive_fname, const char *prepend_path)
{
#if 0
	// This is how to call Info-Zip's unzip32.dll, but it's horribly non-portable.
	DCL dcl;

	dcl.C_flag = true;	// case insensitive
	dcl.ExtractOnlyNewer = false;
	dcl.fPrivilege = 0;	// restore ACL's if > 0, use privileges if 2
	dcl.fQuiet = 0;	// We want all messages.
	dcl.lpszExtractDir = NULL;
	dcl.lpszZipFN = archive_fname;	// The ZIP-file to extract
	dcl.naflag = 0;	// Do not convert CR to CRLF
	dcl.ncflag = 0;	// Write to stdout if true
	dcl.ndflag = 1;	// Recreate directories if true
	dcl.nfflag = 0;	// "freshen" (replace existing files by newer versions)
	dcl.noflag = 1; // Over-write all files if true
	dcl.ntflag = 0;	// test zip file if true
	dcl.nvflag = 0;	// give a verbose listing if true
	dcl.nzflag = 0;	// display a zip file comment if true
	dcl.nZIflag = 0;	// get zip info if true
	dcl.PromptToOverwrite = 0; // true if prompt to overwrite is wanted
	dcl.SpaceToUnderscore = 0; // true if convert space to underscore

	int nExtract = 1;	// I think this is the number of files to extract from
	int nDontExtract = 0;

	char ** ppExFilter = NULL;
	char ** ppDontExFilter = NULL;

	int nRet = Wiz_SingleEntryUnzip(nExtract, ppExFilter, nDontExtract, ppDontExFilter, &dcl, NULL);

	// This is how to call Info-Zip's UnZip without all of the DLL weirdness.
	UzpVer *pVersion = UzpVersion();
	int argc = 1;
	char *argv[1] = { "C:/TEMP/test2.zip" };

	UzpInit init;
	init.inputfn = NULL;
	init.msgfn = NULL;
	init.pausefn = NULL;
	init.userfn = NULL;
	init.structlen = sizeof(UzpInit);

	retcode = UzpMain(argc, argv);

	// Here is an attempt to call Info-Zip's UnZip really directly.
	CONSTRUCTGLOBALS();
	int r = unzip(__G__ argc, argv);
	DESTROYGLOBALS();
#endif

#if SUPPORT_UNZIP
	int iVolumeSize = 0;
	int iCount = 0;
	int i;

	CZipArchive zip;
	try
	{
		zip.Open(archive_fname, CZipArchive::zipOpenReadOnly, iVolumeSize);
		iCount = zip.GetCount();

		CZipFileHeader fh;
		for (i = 0; i < iCount; i++)
		{
			if (zip.GetFileInfo(fh, i))
				printf("%d: %s\n", i, fh.GetFileName());
			zip.ExtractFile(i, prepend_path);
		}
	}
	catch(...)
	{
		// all failures go here
		return 0;
	}
	zip.Close();
	return iCount;
#else
	return 0;
#endif	// SUPPORT_UNZIP
}

