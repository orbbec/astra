/*
Copyright (c) 2013-2014, Cong Xu, Baudouin Feildel
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef TINYDIR_H
#define TINYDIR_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>


/* types */

#define _TINYDIR_PATH_MAX 4096
#define _TINYDIR_PATH_EXTRA 0
#define _TINYDIR_FILENAME_MAX 256

#define _TINYDIR_FUNC static __inline__

/* Allow user to use a custom allocator by defining _TINYDIR_MALLOC and _TINYDIR_FREE. */
#if    defined(_TINYDIR_MALLOC) &&  defined(_TINYDIR_FREE)
#elif !defined(_TINYDIR_MALLOC) && !defined(_TINYDIR_FREE)
#else
#error "Either define both alloc and free or none of them!"
#endif

#if !defined(_TINYDIR_MALLOC)
	#define _TINYDIR_MALLOC(_size) malloc(_size)
	#define _TINYDIR_FREE(_ptr)    free(_ptr)
#endif //!defined(_TINYDIR_MALLOC)

typedef struct
{
	char path[_TINYDIR_PATH_MAX];
	char name[_TINYDIR_FILENAME_MAX];
	char *extension;
	int is_dir;
	int is_reg;

	struct stat _s;
} tinydir_file;

typedef struct
{
	char path[_TINYDIR_PATH_MAX];
	int has_next;
	size_t n_files;

	tinydir_file *_files;
	DIR *_d;
	struct dirent *_e;
} tinydir_dir;


/* declarations */

_TINYDIR_FUNC
int tinydir_open(tinydir_dir *dir, const char *path);
_TINYDIR_FUNC
int tinydir_open_sorted(tinydir_dir *dir, const char *path);
_TINYDIR_FUNC
void tinydir_close(tinydir_dir *dir);

_TINYDIR_FUNC
int tinydir_next(tinydir_dir *dir);
_TINYDIR_FUNC
int tinydir_readfile(const tinydir_dir *dir, tinydir_file *file);
_TINYDIR_FUNC
int tinydir_readfile_n(const tinydir_dir *dir, tinydir_file *file, size_t i);
_TINYDIR_FUNC
int tinydir_open_subdir_n(tinydir_dir *dir, size_t i);

_TINYDIR_FUNC
void _tinydir_get_ext(tinydir_file *file);
_TINYDIR_FUNC
int _tinydir_file_cmp(const void *a, const void *b);


/* definitions*/

_TINYDIR_FUNC
int tinydir_open(tinydir_dir *dir, const char *path)
{
	if (dir == NULL || path == NULL || strlen(path) == 0)
	{
		errno = EINVAL;
		return -1;
	}
	if (strlen(path) + _TINYDIR_PATH_EXTRA >= _TINYDIR_PATH_MAX)
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	/* initialise dir */
	dir->_files = NULL;
	dir->_d = NULL;
	tinydir_close(dir);

	strcpy(dir->path, path);
	dir->_d = opendir(path);
	if (dir->_d == NULL)
	{
		errno = ENOENT;
		goto bail;
	}

	/* read first file */
	dir->has_next = 1;
	dir->_e = readdir(dir->_d);
	if (dir->_e == NULL)
	{
		dir->has_next = 0;
	}
	return 0;

bail:
	tinydir_close(dir);
	return -1;
}

_TINYDIR_FUNC
int tinydir_open_sorted(tinydir_dir *dir, const char *path)
{
	/* Count the number of files first, to pre-allocate the files array */
	size_t n_files = 0;
	if (tinydir_open(dir, path) == -1)
	{
		return -1;
	}
	while (dir->has_next)
	{
		n_files++;
		if (tinydir_next(dir) == -1)
		{
			goto bail;
		}
	}
	tinydir_close(dir);

	if (tinydir_open(dir, path) == -1)
	{
		return -1;
	}

	dir->n_files = 0;
	dir->_files = (tinydir_file *)_TINYDIR_MALLOC(sizeof *dir->_files * n_files);
	if (dir->_files == NULL)
	{
		errno = ENOMEM;
		goto bail;
	}
	while (dir->has_next)
	{
		tinydir_file *p_file;
		dir->n_files++;

		p_file = &dir->_files[dir->n_files - 1];
		if (tinydir_readfile(dir, p_file) == -1)
		{
			goto bail;
		}

		if (tinydir_next(dir) == -1)
		{
			goto bail;
		}

		/* Just in case the number of files has changed between the first and
		second reads, terminate without writing into unallocated memory */
		if (dir->n_files == n_files)
		{
			break;
		}
	}

	qsort(dir->_files, dir->n_files, sizeof(tinydir_file), _tinydir_file_cmp);

	return 0;

bail:
	tinydir_close(dir);
	return -1;
}

_TINYDIR_FUNC
void tinydir_close(tinydir_dir *dir)
{
	if (dir == NULL)
	{
		return;
	}

	memset(dir->path, 0, sizeof(dir->path));
	dir->has_next = 0;
	dir->n_files = 0;
	if (dir->_files != NULL)
	{
		_TINYDIR_FREE(dir->_files);
	}
	dir->_files = NULL;
	if (dir->_d)
	{
		closedir(dir->_d);
	}
	dir->_d = NULL;
	dir->_e = NULL;
}

_TINYDIR_FUNC
int tinydir_next(tinydir_dir *dir)
{
	if (dir == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	if (!dir->has_next)
	{
		errno = ENOENT;
		return -1;
	}

	dir->_e = readdir(dir->_d);
	if (dir->_e == NULL)
	{
		dir->has_next = 0;
	}

	return 0;
}

_TINYDIR_FUNC
int tinydir_readfile(const tinydir_dir *dir, tinydir_file *file)
{
	if (dir == NULL || file == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	if (dir->_e == NULL)
	{
		errno = ENOENT;
		return -1;
	}
	if (strlen(dir->path) +
		strlen(
			dir->_e->d_name
		) + 1 + _TINYDIR_PATH_EXTRA >=
		_TINYDIR_PATH_MAX)
	{
		/* the path for the file will be too long */
		errno = ENAMETOOLONG;
		return -1;
	}
	if (strlen(
			dir->_e->d_name
		) >= _TINYDIR_FILENAME_MAX)
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	strcpy(file->path, dir->path);
	strcat(file->path, "/");
	strcpy(file->name,
		dir->_e->d_name
	);
	strcat(file->path, file->name);
	if (stat(file->path, &file->_s) == -1)	
	{	
		return -1;	
	}
	_tinydir_get_ext(file);

	file->is_dir =
		S_ISDIR(file->_s.st_mode);
	file->is_reg =
	S_ISREG(file->_s.st_mode);

	return 0;
}

_TINYDIR_FUNC
int tinydir_readfile_n(const tinydir_dir *dir, tinydir_file *file, size_t i)
{
	if (dir == NULL || file == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	if (i >= dir->n_files)
	{
		errno = ENOENT;
		return -1;
	}

	memcpy(file, &dir->_files[i], sizeof(tinydir_file));
	_tinydir_get_ext(file);

	return 0;
}

_TINYDIR_FUNC
int tinydir_open_subdir_n(tinydir_dir *dir, size_t i)
{
	char path[_TINYDIR_PATH_MAX];
	if (dir == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	if (i >= dir->n_files || !dir->_files[i].is_dir)
	{
		errno = ENOENT;
		return -1;
	}

	strcpy(path, dir->_files[i].path);
	tinydir_close(dir);
	if (tinydir_open_sorted(dir, path) == -1)
	{
		return -1;
	}

	return 0;
}

/* Open a single file given its path */
_TINYDIR_FUNC
int tinydir_file_open(tinydir_file *file, const char *path)
{
	tinydir_dir dir;
	int result = 0;
	int found = 0;
	char dir_name_buf[_TINYDIR_PATH_MAX];
	char file_name_buf[_TINYDIR_FILENAME_MAX];
	char *dir_name;
	char *base_name;
	
	if (file == NULL || path == NULL || strlen(path) == 0)
	{
		errno = EINVAL;
		return -1;
	}
	if (strlen(path) + _TINYDIR_PATH_EXTRA >= _TINYDIR_PATH_MAX)
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	/* Get the parent path */
	strcpy(dir_name_buf, path);
	dir_name = dirname(dir_name_buf);
	strcpy(file_name_buf, path);
	base_name = basename(file_name_buf);
	
	/* Open the parent directory */
	if (tinydir_open(&dir, dir_name) == -1)
	{
		return -1;
	}

	/* Read through the parent directory and look for the file */
	while (dir.has_next)
	{
		if (tinydir_readfile(&dir, file) == -1)
		{
			result = -1;
			goto bail;
		}
		if (strcmp(file->name, base_name) == 0)
		{
			/* File found */
			found = 1;
			goto bail;
		}
		tinydir_next(&dir);
	}
	if (!found)
	{
		result = -1;
		errno = ENOENT;
	}
	
bail:
	tinydir_close(&dir);
	return result;
}

_TINYDIR_FUNC
void _tinydir_get_ext(tinydir_file *file)
{
	char *period = strrchr(file->name, '.');
	if (period == NULL)
	{
		file->extension = &(file->name[strlen(file->name)]);
	}
	else
	{
		file->extension = period + 1;
	}
}

_TINYDIR_FUNC
int _tinydir_file_cmp(const void *a, const void *b)
{
	const tinydir_file *fa = (const tinydir_file *)a;
	const tinydir_file *fb = (const tinydir_file *)b;
	if (fa->is_dir != fb->is_dir)
	{
		return -(fa->is_dir - fb->is_dir);
	}
	return strncmp(fa->name, fb->name, _TINYDIR_FILENAME_MAX);
}

#endif
