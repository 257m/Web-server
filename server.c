#include "common.h"

bool string_compare(const char* str1,const char* str2)
{
    while(*str1 == *str2) {
        if(!*str1 || !*str2)
            return !(*str1 || *str2);
        str1++;
		str2++;
    }
	return false;
}

char* string_null_char(char* str, char* output, char c)
{
	if (!*str)
		return;
	while (*str && *str != c)
		*output++ = *str++;
	*output = '\0';
	return output;
}

char* stratt(const char *s1, const char *s2, char* result)
{
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

typedef struct {
	unsigned long int filelength;
	int status;
	char* status_str;
	char* textbuffer;
	char* content_type;
	bool dynamic;
} Page;

Page* page_create() {
	Page* page = malloc(sizeof(Page));
	if (page) {
		page->textbuffer = NULL;
		page->content_type = NULL;
		page->status_str = NULL;
		page->filelength = 0;
		page->dynamic = true;
	}
	else {
		fprintf(stderr, "Error allocating memory for page\n");
		exit(-1);
	}
	return page;
}

void page_free(Page* page)
{
	if (page->dynamic)
		free(page->textbuffer);
	free(page);
}

Page* page_create_from_file(char* filename) {
	Page* page = page_create();
	FILE * f = fopen (filename, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		page->filelength = ftell(f);
		fseek(f, 0, SEEK_SET);
		page->textbuffer = malloc((page->filelength * sizeof(char)) + 1);
		bzero(page->textbuffer,page->filelength+1);
		if (page->textbuffer)
			fread (page->textbuffer, 1, page->filelength, f);
		fclose (f);
	}
	else {
		fprintf(stderr, "Failed to open file: %s\n", filename);
		page->status = 404;
		page->status_str = "Not Found";
		page->textbuffer = "Not Found";
		page->content_type = "text/txt";
		page->filelength = 9;
		page->dynamic = false;
		return page;
	}
	char* extension = filename;
	while (*filename)
		if (*(filename++) == '.')
			extension = filename;
	if (string_compare(extension,"html"))
		page->content_type = "text/html";
	else if (string_compare(extension,"js"))
		page->content_type = "text/javascript";
	else if (string_compare(extension,"wasm"))
		page->content_type = "application/wasm";
	else if (string_compare(extension, "png"))
		page->content_type = "image/png";
	else
		page->content_type = "text/txt";
	page->status = 200;
	page->status_str = "OK";
	return page;
}

Page* page_load_from_file(Page* page, char* file)
{
	if (page->dynamic)
		free(page->textbuffer);
	FILE * f = fopen (file, "rb");
	if (f) {
		fseek (f, 0, SEEK_END);
		page->filelength = ftell(f);
		fseek (f, 0, SEEK_SET);
		page->textbuffer = malloc((page->filelength * sizeof(char)) + 1);
		bzero(page->textbuffer,page->filelength+1);
		if (page->textbuffer)
			fread (page->textbuffer, 1, page->filelength, f);
		fclose (f);
	}
	else {
		fprintf(stderr, "Failed to open file %s\n", file);
		page->status = 404;
		page->status_str = "Not Found";
		page->textbuffer = "Not Found";
		page->content_type = "text/txt";
		page->filelength = 9;
		page->dynamic = false;
		return page;
	}
	char extension [32] = "\0";
	int i = 0;
	while (file[i++] != '.') {
		if (file[i-1] == '\0') {
			fprintf(stderr, "file name %s for page does not have file extension\nExiting...\n", file);
			exit(-1);
		}
	}
	int j = 0;
	while (file[i] != '\0')
		extension[j++] = file[i++];
	if (string_compare(extension,"html"))
		page->content_type = "text/html";
	else if (string_compare(extension,"js"))
		page->content_type = "text/javascript";
	else if (string_compare(extension,"png") || string_compare(extension,"ico"))
		page->content_type = "image/png";
	else if (string_compare(extension,"wasm"))
		page->content_type = "application/wasm";
	else if (string_compare(extension, "png"))
		page->content_type = "image/png";
	else
		page->content_type = "text/html";
	page->status = 200;
	page->status_str = "OK";
	return page;
}

#define MAX_URL 2048

typedef struct {
	char path [MAX_URL]; 
	uint8_t method;
} Request;

char request_meth_strs [][7] = {
	"GET",
	"HEAD",
	"POST",
	"DELETE",
	"PUT",
	"CONNECT",
	"OPTIONS",
	"TRACE",
	"PATCH"
};

enum {
	GET,
	HEAD,
	POST,
	DELETE,
	PUT,
	CONNECT,
	OPTIONS,
	TRACE,
	PATCH,
	MAX_METHOD,
};

void request_load(Request* request,char* buffer)
{
	char tempbuffer [32] = "";
	char* ptemp = tempbuffer;
	while (*buffer && (*buffer != ' ') && (ptemp < (tempbuffer + sizeof(tempbuffer))))
		*ptemp++ = *buffer++;
	
	request->method = 0;
	while (!string_compare(tempbuffer, request_meth_strs[request->method]))
		if (++request->method >= MAX_METHOD)
			break;

	ptemp = request->path;
	while(*buffer && *buffer != ' ')
		*ptemp++ = *buffer++;
	*ptemp = '\0';
	printf("Request Load: %s\n", ptemp);
}

/*
stratt simply returns str1 + str2 without changing str1
*/

/* 
num = number of args ;
next pass in name of variable as string that you want to query
then pass in textbuffer to write it to
*/
void request_query_load(Request* request,int num, ...) {
	va_list args;
	char **varnames = malloc(sizeof(char*) * num);
	char **outputbuffers = malloc(sizeof(char*) * num);
	
	char tempvarname [64] = {0};
	char tempvalue [64] = {0};
	
	int j = 0;
	
	va_start(args, num);

	for (int i = 0;i < num*2; i++) {
		if (!(i % 2)) {
			varnames[i/2] = va_arg(args,char*);
		} else {
			outputbuffers[i/2] = va_arg(args,char*);
		}
	}

	unsigned int charnum = 0;
	unsigned int tempnum = 0;

	while (1) {
		if (request->path[charnum] == '\0') goto end;
		if (request->path[charnum++] == '?') break;
		}

	for (int i = 0;i < num;i++) {
		bzero(tempvarname,sizeof(tempvarname));
		bzero(tempvalue,sizeof(tempvalue));
		tempnum = 0;
		while (1) {
			tempvarname[tempnum++] = request->path[charnum++];
			if (request->path[charnum] == '=') {
				charnum++;
				break;
					}
			else if (request->path[charnum] == '\0') {
				goto end;
					}
				}
		
		tempnum = 0;
		while (1) {
			if (request->path[charnum] == '&') {
				charnum++;
				break;
					}
			if (request->path[charnum] == '\0') {
				for (j = 0;j < num;j++) {
				if (string_compare(tempvarname,varnames[j])) {
					strcpy(outputbuffers[j],tempvalue);
					break;
				}
			}
				goto end;
					}
			tempvalue[tempnum++] = request->path[charnum++];
				}

			for (j = 0;j < num;j++) {
				if (string_compare(tempvarname,varnames[j])) {
					strcpy(outputbuffers[j],tempvalue);
					break;
				}
			}
			}
	end:
		va_end(args);
		free(varnames);
		free(outputbuffers);
}

void string_save_to_file(char* file, char* string)
{
	FILE* f = fopen(file,"w");
	if (f) {
		fprintf(f,"%s",string);
		fclose (f);
	}
	else {
		printf("Failed to open file\n");
		exit(-1);
	}
}

char* file_load_into_buffer(char* filestring)
{
	char* file = 0;
	long filelength;
	FILE * f = fopen (filestring, "rb");
	if (f) {
		fseek (f, 0, SEEK_END);
		filelength = ftell (f);
		fseek (f, 0, SEEK_SET);
		file = malloc (filelength);
		if (file) {
			fread (file, 1, filelength, f);
		}
		fclose (f);
	}
	else {
		printf("Failed to open file\n");
		exit(-1);
	}
	return file;
}

#define public_dir "public"
#define public_dir_size (sizeof(public_dir) - 1)

int main(int argc, char **argv)
{
	if (argc > 1)
		chdir(argv[0]);
	Request client_request = {"", 0};
	Page* current_page = page_create_from_file(public_dir "/index.html");
	char path_wo_query [64] = public_dir;
	int listenfd, connfd, n;
	struct sockaddr_in servaddr;
	uint8_t buff[MAXLINE+1];
	uint8_t recvline[MAXLINE+1];

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err_n_die("Error while creating socket!");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERVER_PORT);

	if (bind(listenfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
		err_n_die("bind error.");

	if (listen(listenfd, 10) < 0)
		err_n_die("listen error.");

	for ( ; ; ) {
		struct sockaddr_in addr;
		socklen_t addr_len;
		char client_address[2048];

		printf("waiting for a connection on port %d\n",SERVER_PORT);
		fflush(stdout);
		connfd = accept(listenfd, (SA *) &addr, &addr_len);

		inet_ntop(AF_INET, &addr, client_address, 2047);
		printf("Client connection: %s\n",client_address);

		memset(recvline, 0, MAXLINE);

		while ((n = read(connfd, recvline, MAXLINE))) {
			fprintf(stderr, "\nPacket Size: %d\n%s\n\n%s", n, bin2hex(recvline, n), recvline);

			if (recvline[n-1] == '\n')
				break;
			memset(recvline, 0, MAXLINE);
		}

		if (n < 0)
			err_n_die("read error");

		request_load(&client_request, recvline);
		bzero(path_wo_query + public_dir_size,sizeof(path_wo_query) - public_dir_size);
		string_null_char(client_request.path, path_wo_query + public_dir_size, '?');
		bzero(buff, sizeof(buff));
		unsigned int Http_Header_Size = 0;

		if (!*path_wo_query + public_dir_size) {
			current_page->status = 404;
			current_page->status_str = "Not Found";
			current_page->textbuffer = "Not Found";
			current_page->content_type = "text/txt";
			current_page->filelength = 9;
			current_page->dynamic = false;
		}
		else if (string_compare(path_wo_query + public_dir_size, "/"))
			page_load_from_file(current_page, public_dir "/index.html");
		else
			page_load_from_file(current_page, path_wo_query);

		snprintf((char*)buff, sizeof(buff), "HTTP/1.1 %d %s\r\nContent-Type: %s; charset=UTF-8\r\nConnection: close\r\nContent-Length: %d\r\n\r\n",
			current_page->status, current_page->status_str, current_page->content_type, current_page->filelength);
		while (buff[Http_Header_Size])
			Http_Header_Size++;
		memcpy(buff+Http_Header_Size, current_page->textbuffer, current_page->filelength);
		fprintf(stderr, "%s\n", buff);
		write(connfd, (char*)buff, Http_Header_Size + current_page->filelength);
		close(connfd);
	}
}

void err_n_die(const char *fmt, ...) {
	int errno_save;
	va_list ap;

	errno_save = errno;

	va_start(ap, fmt);
	vfprintf(stdout,fmt,ap);
	fprintf(stdout,"\n");
	fflush(stdout);

	if (errno_save != 0) {
    	fprintf(stdout, "(errno = %d) : %s\n",errno_save,
    	strerror(errno_save));
    	fprintf(stdout,"\n");
    	fflush(stdout);
	}
	va_end(ap);

	exit(1);
}

char *bin2hex(const unsigned char *input, size_t len) {
	char *result;
	char *hexits = "0123456789ABCDEF";

	if (input == NULL || len <= 0)
		return NULL;

	int resultlength = (len*3)+1;

	result = malloc(resultlength);
	bzero(result,resultlength);

	for (int i = 0;i < len; i++) {
		result[i*3] = hexits[input[i] >> 4];
		result[(i*3)+1] = hexits[input[i] & 0x0F];
		result[(i*3)+2] = ' ';
	}

	return result;
}
