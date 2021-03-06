#include <stdio.h>
#include "md5.h"
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include "zlib.h"
#include <sys/types.h>
#include <utime.h>

using namespace std;


#define READ_DATA_SIZE	1024
#define MD5_SIZE		16
#define MD5_STR_LEN		(MD5_SIZE * 2)
#define MAX_FILE_SIZE 2048 

int Compute_file_md5(const char *file_path, char *value);
void checkFile(char*, char*, char*);
void copyFile(char*, char*);
void creatDir(char *);
void readVersion();
void writeVersion();
void compressVersion();

map<string, string> vermap;

char in_file[MAX_FILE_SIZE];
char out_file[MAX_FILE_SIZE];

char *incr_path;
char *in_path;
char *out_path;
char *ver_id;
int main(int argc, char* argv[])
{
	in_path = argv[1];
	out_path = argv[2];
	incr_path = argv[3];
	ver_id = argv[4];

	printf("in_path:%s out_path:%s incr_path:%s\n", in_path, out_path, incr_path);

	printf("Start...%s.\n", incr_path);
	readVersion();
	printf("readVersion Finish...\n");

	char tmp1[512];
	char tmp2[512];
	char tmp3[512];
	memset(tmp1, 0, sizeof(tmp1));

	memset(tmp2, 0, sizeof(tmp2));
	memset(tmp3, 0, sizeof(tmp3));
	checkFile(tmp1, tmp2, tmp3);
	printf("checkFile Finish...\n");
	writeVersion();
	printf("writeVersion Finish...\n");
	compressVersion();
}


unsigned char com_buf[100 * 1024 * 1024];
unsigned char com_buf1[100 * 1024 * 1024];

void compressVersion()
{
	int fd;
	char ver_path[512];
	memset(ver_path, 0, sizeof(ver_path));
	sprintf(ver_path, "%s/%s", incr_path, "version.swf");

//	unsigned char buf[100 * 1024 * 1024];
	memset(com_buf, 0, sizeof(com_buf));
//	unsigned char dest_buf[100 * 1024 * 1024];
	memset(com_buf1, 0, sizeof(com_buf1));

	fd = open(ver_path, O_RDONLY);
	int read_len = read(fd, com_buf, sizeof(com_buf));
	close(fd);

	uLong dest_len = 100 * 1024 * 1024;
	compress(com_buf1, &dest_len, com_buf, read_len);

	printf("file_len:%ld %d\n", dest_len, read_len);
	fd = open(ver_path, O_WRONLY|O_TRUNC);
	write(fd, com_buf1, dest_len);
	close(fd);
}

const int tmp_buf_size = 10;
void writeVersion()
{
        char ver_path[512];
        memset(ver_path, 0, sizeof(ver_path));
        sprintf(ver_path, "%s/%s", incr_path, "version.swf");

        char new_file_path[1024];
        //char new_file_md5[MD5_STR_LEN];
        memset(new_file_path, 0, sizeof(new_file_path));

        char tmp_buf[1024 * tmp_buf_size]; 
        memset(tmp_buf, 0, sizeof(tmp_buf));
        int tmp_buf_idx = 0;

        short str_len;
        const char *str;
        int fd = open(ver_path, O_WRONLY|O_CREAT|O_TRUNC);
        unsigned short count = vermap.size();
        write(fd, &count, 2); 
        map <string, string>::iterator ver_iter;

	char tmp_file_path[1024];
	memset(tmp_file_path, 0, sizeof(tmp_file_path));
	int file_size;
	struct stat file_info;

        char ver_str[256];
        int insize = strlen(in_path);
        printf("ver_size---------->%d\n", count);
        for (ver_iter = vermap.begin(); ver_iter != vermap.end(); ver_iter++)
        {   
                str = ver_iter->first.c_str();
                str_len = strlen(str);
                memcpy(tmp_buf + tmp_buf_idx, &str_len, 2); 
                tmp_buf_idx += 2;
                memcpy(tmp_buf + tmp_buf_idx, str, str_len);
                tmp_buf_idx += str_len;
				
		memset(tmp_file_path, 0, sizeof(tmp_file_path));
		strcat(tmp_file_path, in_path);
		strcat(tmp_file_path, "/");
		strcat(tmp_file_path, str);

                memset(new_file_path, 0, sizeof(new_file_path));
                sprintf(new_file_path, "%s/%s", in_path, str);
//                memset(new_file_md5, 0, sizeof(new_file_md5));

                str = ver_iter->second.c_str();
                str_len = strlen(str);

                memcpy(tmp_buf + tmp_buf_idx, &str_len, 2);
                tmp_buf_idx += 2;
                memcpy(tmp_buf + tmp_buf_idx, str, str_len);
                tmp_buf_idx += str_len;
				
//		sprintf(tmp_file_path, "%s%s", in_path, tmp_file_path);
		stat(tmp_file_path, &file_info);
		file_size = file_info.st_size / 128;

		//printf("file--->len:%s %d\n", tmp_file_path, file_size);
                memcpy(tmp_buf + tmp_buf_idx, &file_size, 2);
                tmp_buf_idx += 2;

                if (tmp_buf_idx >= 1024 * 9)
                {
                        write(fd, tmp_buf, tmp_buf_idx);
                        memset(tmp_buf, 0, sizeof(tmp_buf));
                        tmp_buf_idx = 0;
                }

                //Compute_file_md5(new_file_path, new_file_md5);
        }
        write(fd, tmp_buf, tmp_buf_idx);
 
        close(fd);
}

unsigned char file_buf[100 * 1024 * 1024];
unsigned char file_buf1[100 * 1024 * 1024];
void readVersion()
{
	memset(file_buf1, 0, sizeof(file_buf1));
	memset(file_buf, 0, sizeof(file_buf));

	char ver_path[512];
	memset(ver_path, 0, sizeof(ver_path));
	sprintf(ver_path, "%s/%s", out_path, "version.swf");

	int fd = open(ver_path, O_RDONLY|O_CREAT);

	uLong read_len = read(fd, file_buf, sizeof(file_buf));	

	read_len = sizeof(file_buf);
	uncompress(file_buf1, &read_len, file_buf, sizeof(file_buf));

	short count = 0;
	unsigned char *file_ptr;
	//read_len = read(fd, &count, 4);
	file_ptr = file_buf1;
	memcpy(&count, file_ptr, 2);
	file_ptr += 2;
	short file_path_len = 0;
	short file_ver_len = 0;
	char file_path[512];
	char file_ver[256];
	printf("file:%s file_len:%ld  count--->%d\n", ver_path, read_len, count);
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

		if (strlen(file_path) != 0)
		{
			vermap[file_path] = file_ver;	
		}
	}
	close(fd);
}

void checkFile(char *in_path_tt, char* out_path_tt, char* incr_path_tt)
{
	char in_path_t[1024];
	char out_path_t[1024];
	char incr_path_t[1024];

	char map_path_t[1024];

	memset(in_path_t, 0, sizeof(in_path_t));
	memset(out_path_t, 0, sizeof(out_path_t));
	memset(incr_path_t, 0, sizeof(incr_path_t));

	strcat(in_path_t, in_path);
	strcat(out_path_t, out_path);
	strcat(incr_path_t, incr_path);
	if (strlen(in_path_tt) != 0)
	{
		strcat(in_path_t, "/");
		strcat(in_path_t, in_path_tt);
	}
	if (strlen(out_path_tt) != 0)
	{
		strcat(out_path_t, "/");
		strcat(out_path_t, out_path_tt);
	}
	if (strlen(incr_path_tt) != 0)
	{
		strcat(incr_path_t, "/");
		strcat(incr_path_t, incr_path_tt);
	}

	int fd1, fd2;
	int fdLen1, fdLen2;

	DIR* p_dir1;
	struct dirent *ent;
	char child_path[1024];
	memset(child_path, 0, sizeof(child_path));

	char out_child_path[1024];
	memset(out_child_path, 0, sizeof(child_path));

	char incr_child_path[1024];

	char file_path[1024];
	memset(file_path, 0, sizeof(child_path));

	char out_file_path[1024];
	char incr_file_path[1024];

	char in_md5_str[MD5_STR_LEN + 1];
	char out_md5_str[MD5_STR_LEN + 1];

	p_dir1 = opendir(in_path_t);

	while ((ent = readdir(p_dir1)) != NULL)
	{
		if (ent->d_type & DT_DIR)
		{
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
			{
				continue;
			}

			if (strlen(in_path_tt) != 0)
			{

				sprintf(child_path, "%s/%s", in_path_tt, ent->d_name);
				sprintf(out_child_path, "%s/%s", out_path_tt, ent->d_name);
				sprintf(incr_child_path, "%s/%s", incr_path_tt, ent->d_name);
			}
			else
			{
				sprintf(child_path, "%s", ent->d_name);
				sprintf(out_child_path, "%s", ent->d_name);
				sprintf(incr_child_path, "%s", ent->d_name);
			}


			checkFile(child_path, out_child_path, incr_child_path);
		}
		else
		{
			memset(file_path, 0, sizeof(file_path));
			memset(out_file_path, 0, sizeof(out_file_path));
			memset(incr_file_path, 0, sizeof(incr_file_path));

			memset(map_path_t, 0, sizeof(map_path_t));
			if (strlen(in_path_tt) == 0)
			{
				sprintf(map_path_t, "%s", ent->d_name);
			}
			else
			{
				sprintf(map_path_t, "%s/%s", in_path_tt, ent->d_name);
			}

			sprintf(file_path, "%s/%s", in_path_t, ent->d_name);
			sprintf(out_file_path, "%s/%s", out_path_t, ent->d_name);
			sprintf(incr_file_path, "%s/%s", incr_path_t, ent->d_name);

			struct stat filebuf1;
			struct stat filebuf2;
			stat(file_path, &filebuf1);
			stat(out_file_path, &filebuf2);

			if (filebuf1.st_mtime != filebuf2.st_mtime)
			{
				//printf("in_file:%s  out_file:%s\n", file_path, out_file_path);
				fd1 = open(file_path, O_RDONLY);
				fd2 = open(out_file_path, O_RDONLY);

				while (1)
				{
					memset(in_file, 0, sizeof(in_file));
					memset(out_file, 0, sizeof(out_file));
					fdLen1 = read(fd1, in_file, MAX_FILE_SIZE);
					fdLen2 = read(fd2, out_file, MAX_FILE_SIZE);

					if (fdLen1 == 0 && fdLen2 == 0)
					{
						//					printf("name:%s file1_len:%d  file2_len:%d\n", file_path, fdLen1, fdLen2);
						break;
					}
					if (fdLen1 != fdLen2)
					{
						printf("len1 != len2 --> %d %d %s --> %s\n", fdLen1, fdLen2, file_path, incr_file_path);
						vermap[map_path_t] = ver_id;
						copyFile(file_path, incr_file_path);
						break;
					}
					//Compute_file_md5(file_path, in_md5_str);
					//Compute_file_md5(out_file_path, out_md5_str);
					if (memcmp(in_file, out_file, fdLen1) != 0)//strcmp(in_md5_str, out_md5_str) != 0)
					{
						printf("字节不同 --> %s --> %s\n", file_path, incr_file_path);
						vermap[map_path_t] = ver_id;
						copyFile(file_path, incr_file_path);
						break;
					}
					//	break;

				}

				struct stat filebuf1;
				stat(file_path, &filebuf1);  
				struct utimbuf timbuf;
				timbuf.actime = filebuf1.st_atime;
				timbuf.modtime = filebuf1.st_mtime;
				utime(incr_file_path, &timbuf);

				close(fd1);
				close(fd2);
			}
		}
	}
	closedir(p_dir1);
}

void copyFile(char *in_file_path, char *out_file_path)
{
	creatDir(out_file_path);

	int in_fd;
	int out_fd;

	int read_len;
	char buf[1024];

	in_fd = open(in_file_path, O_RDONLY);
	out_fd = open(out_file_path, O_WRONLY | O_CREAT | O_TRUNC);
	while (1)
	{
		read_len = read(in_fd, buf, 1024);
		if (read_len > 0)
		{
			write(out_fd, buf, read_len);
		}
		else
		{
			break;
		}
	}
	//
//	struct stat filebuf1;
  //      stat(in_file_path, &filebuf1);	
//	struct utimbuf timbuf;
//	timbuf.actime = filebuf1.st_atime;
//	timbuf.modtime = filebuf1.st_mtime;
//	utime(out_file_path, &timbuf);
	
	close(in_fd);
	close(out_fd);
}

void creatDir(char *p_dir)
{
	int i = 0;
	int iRet;
	int iLen;
	char *pszDir;

	if (p_dir == NULL)
	{
		return;
	}

	pszDir = strdup(p_dir);
	iLen = strlen(pszDir);

	for (i = 0; i < iLen; i++)
	{
		if (pszDir[i] == '/')
		{
			pszDir[i] = '\0';

			if (access(pszDir, F_OK) == -1)
			{
				mkdir(pszDir, 0755);
			}

			pszDir[i] = '/';
		}
	}
}

int Compute_file_md5(const char *file_path, char *md5_str)
{
	int i;
	int fd;
	int ret;
	unsigned char data[READ_DATA_SIZE];
	unsigned char md5_value[MD5_SIZE];
	MD5_CTX md5;

	fd = open(file_path, O_RDONLY);
	if (-1 == fd)
	{
		perror("open");
		return -1;
	}

	// init md5
	MD5Init(&md5);

	while (1)
	{
		ret = read(fd, data, READ_DATA_SIZE);
		if (-1 == ret)
		{
			perror("read");
			return -1;
		}

		MD5Update(&md5, data, ret);

		if (0 == ret || ret < READ_DATA_SIZE)
		{
			break;
		}
	}

	close(fd);

	MD5Final(&md5, md5_value);

	for(i = 0; i < MD5_SIZE; i++)
	{
		snprintf(md5_str + i*2, 2+1, "%02x", md5_value[i]);
	}
	md5_str[MD5_STR_LEN] = '\0'; // add end

	return 0;
}
