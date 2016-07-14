#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <map>
#include "zlib.h"
#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <fcntl.h>

using namespace std;

#define MAX_FILE_PATH	1024

int readVersion();
int copyFile(char *in_file_path, char *out_file_path);
int creatDir(char *p_dir);

char *incr_path;
char *target_path;

map<string, string> vermap;

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("[Failed][argc=%d less than 2.]\n",argc);
		return -1;
	}

	incr_path = argv[1];
	target_path = argv[2];

	printf("incr_path: %s, target_path\n", incr_path, target_path);

	if (readVersion() != 0)
	{
		printf("[Failed][read readVersion failed.]\n");
		return -1;
	}

	int len = strlen(incr_path);
	while ( len -- )
	{
		if (incr_path[len] != ' ' && incr_path[len] != '/')
		{
			break;
		}
	}
	while( -- len > 0)				// cd .. 
	{
		if (incr_path[len] == '/' ) 
		{
			incr_path[len] = '\0';
			break;
		}
	}

	map<string, string>::iterator ver_iter;
	char input_file_path[MAX_FILE_PATH];
	char output_file_path[MAX_FILE_PATH];
	for (ver_iter = vermap.begin(); ver_iter != vermap.end(); ver_iter ++)
	{
		memset(input_file_path, 0, sizeof input_file_path);
		memset(output_file_path, 0, sizeof output_file_path);

		sprintf(input_file_path, "%s/%s/%s", incr_path, ver_iter->second.c_str(), ver_iter->first.c_str());
		sprintf(output_file_path, "%s/%s", target_path, ver_iter->first.c_str());

		if (copyFile(input_file_path, output_file_path) != 0)
		{
			printf("[Failed][copyFile failed.\n");
			return -1;
		}
	}
	printf("[make_fullversion Success.]\n");

	return 0;
}



unsigned char file_buf[100 * 1024 * 1024];
unsigned char file_buf1[100 * 1024 * 1024];
int readVersion()
{
	char swf_file_path[MAX_FILE_PATH];
	memset(swf_file_path, 0, sizeof(swf_file_path));

	sprintf(swf_file_path, "%s/%s", incr_path, "version.swf");

	int fd = open(swf_file_path, O_RDONLY|O_CREAT);
	if(fd == -1)
	{
		printf("[Failed][swf_file_path=%s open failed.]\n",swf_file_path);
		return -1;
	}

	memset(file_buf1, 0, sizeof(file_buf1));
	memset(file_buf, 0, sizeof(file_buf));
	uLong read_len = read(fd, file_buf, sizeof(file_buf));
	if(read_len <= 0)
	{
		printf("[Failed][swf_file_path=%s have no content.]\n",swf_file_path);
		return -1;
	}

	read_len = sizeof(file_buf);
	if(uncompress(file_buf1, &read_len, file_buf, sizeof(file_buf)) != Z_OK)
	{
		printf("[Failed][uncompress failed.]\n");
		return -1;
	}

	short count = 0;
	unsigned char *file_ptr;
	//read_len = read(fd, &count, 4);
	file_ptr = file_buf1;
	memcpy(&count, file_ptr, 2);		// get count 
	file_ptr += 2;
	short file_path_len = 0;
	short file_ver_len = 0;
	char file_path[512];
	char file_ver[256];
	printf("file:%s file_len:%ld  count--->%d\n", swf_file_path, read_len, count);
	for (int i = 0; i < count; i++)
	{
		memset(file_path, 0, sizeof(file_path));
		memset(file_ver, 0, sizeof(file_ver));
		file_path_len = 0;
		file_ver_len = 0;

		memcpy(&file_path_len, file_ptr, 2);
		file_ptr += 2;
		memcpy(file_path, file_ptr, file_path_len);
		file_ptr += file_path_len;

		memcpy(&file_ver_len, file_ptr, 2);
		file_ptr += 2;
		memcpy(file_ver, file_ptr, file_ver_len);
		file_ptr += file_ver_len;

		file_ptr += 2;
		//	printf("file_path:%s ver:%s\n", file_path, file_ver);

		if(strlen(file_path) == 0 || strlen(file_ver) == 0)
		{
			printf("[Failed][file_path or file_ver length equal zero.]\n");
			return -1;
		}
		else
		{
			vermap[file_path] = file_ver;
		}
	}
	close(fd);
	return 0;
}

int copyFile(char *in_file_path, char *out_file_path)
{
	if(in_file_path == NULL || out_file_path == NULL || creatDir(out_file_path) != 0)
	{
		printf("[Failed][copyFile creatDir failed. in_file_path = %s --> out_file_path=%s]\n",in_file_path, out_file_path);
		return -1;
	}
	printf("copyFile in=%s --> out=%s\n",in_file_path, out_file_path);
	
	int in_fd;
	int out_fd;

	int read_len;
	char buf[1024];

	in_fd = open(in_file_path, O_RDONLY);
	out_fd = open(out_file_path, O_WRONLY | O_CREAT | O_TRUNC);
	if(in_fd == -1 || out_fd == -1)
	{
		printf("[Failed][open file failed. in_file_path=%s, out_file_path=%s.]\n",in_file_path, out_file_path);
		return -1;
	}

	while (1)
	{
		read_len = read(in_fd, buf, 1024);
		if (read_len > 0)
		{
			if( write(out_fd, buf, read_len) == -1)
			{
				printf("[Failed][copyFile write failed, errno = %d.]\n",errno);
				return -1;
			}
		}
		else if(read_len < 0)
		{
			printf("[Failed][copyFile read failed, errno=%d.]\n",errno);
			return -1;
		}
		else 
		{
			break;
		}
	}

	struct stat filebuf1;
    stat(in_file_path, &filebuf1);	
	struct utimbuf timbuf;
	timbuf.actime = filebuf1.st_atime;
	timbuf.modtime = filebuf1.st_mtime;
	utime(out_file_path, &timbuf);
	
	close(in_fd);
	close(out_fd);

	return 0;
}

int creatDir(char *p_dir)
{
	int i = 0;
	int iRet;
	int iLen;
	char *pszDir;

	if (p_dir == NULL)
	{
		return -1;
	}

	pszDir = strdup(p_dir);
	iLen = strlen(pszDir);

	int pos = -1, len = iLen-1;
	while( len >= 0)
	{
		if(pszDir[len] == '/')
		{
			pos = len;
			break;
		}
		len --;
	}
	if(pos != -1 )
	{
		pszDir[pos] = '\0';
		if(access(pszDir, F_OK) == 0)
		{
			free(pszDir);
			pszDir = NULL;
			return 0;
		}
		pszDir[pos] = '/';
	}

	for (i = 0; i < iLen; i++)
	{
		if (pszDir[i] == '/')
		{
			pszDir[i] = '\0';

			if (access(pszDir, F_OK) == -1 && strlen(pszDir) > 0)
			{
				if(mkdir(pszDir, 0755) == -1)
				{
					printf("[Failed][mkdir failed. pszDir=%s error=%d]\n",pszDir,errno);
					return -1;
				}
			}

			pszDir[i] = '/';
		}
	}

	free(pszDir);
	pszDir = NULL;
	return 0;
}