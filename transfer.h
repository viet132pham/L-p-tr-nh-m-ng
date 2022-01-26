#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include "colorCode.h"

#define BUFF_SIZE 100
#define REQUEST_SIZE 1024
#define BUFF_DATA 4096
#define MAX_CLIENTS 10

void printRequest(char *request);
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length);
int readWithCheck(int sock, char buff[BUFF_SIZE], int length);
void *SendFileToServer(int new_socket, char fname[50]);
int receiveUploadedFile(int sock, char filePath[100]);
void str_trim_lf(char *arr, int length);
void sendCode(int sock, int code);
void clearBuff();

// ====================================================================

// In ra thông điệp request - OK
void printRequest(char *request){
	printf(FG_GREEN "[+]REQUEST: %s\n" NORMAL, request);
}

// Gửi thông điệp không gửi được báo lỗi - OK
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length) {
	int sendByte = 0;
	sendByte = send(sock, buff, length, 0);
	if (sendByte > 0) {
	}else {
		printf("[-]Connection is interrupted\n");
		exit(0);
	}
}

// Nhận thông điệp không gửi được báo lỗi - OK
int readWithCheck(int sock, char buff[BUFF_SIZE], int length) {
	int recvByte = 0;
	recvByte = recv(sock, buff, length, 0);
	if (recvByte > 0) {
		return recvByte;
	}
	return 0;
}

// Hàm gửi file- OK
void *SendFile(int new_socket, char *fname) {
	FILE *fp = fopen(fname, "rb");
	if (fp == NULL) {
		printf("[-]File open error");
	}
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	int n, total = 0;
	char sendline[BUFF_DATA] = {0};
	send(new_socket, &size, sizeof(size), 0);
	while ((n = fread(sendline, 1, BUFF_DATA, fp)) > 0) {
		if (n != BUFF_DATA && ferror(fp)) {
			perror("[-]Read File Error");
			exit(1);
		}
		if (send(new_socket, sendline, n, 0) == -1) {
			perror("[-]Can't send file");
			exit(1);
		}
		total += n;
		memset(sendline, '\0', BUFF_DATA);
		if(total >= size) {
			fclose(fp);
			break;
		}
	}
	printf(FG_GREEN "[+]File OK....Completed" NORMAL "\n");
	printf(FG_GREEN "[+]TOTAL SEND: %d\n" NORMAL, total);
}

// Hàm nhận file vào lưu vào thư mục chứa - OK
int receiveUploadedFile(int sock, char filePath[200]) {
	FILE *fp;
	printf(FG_GREEN "[+]Receiving file..." NORMAL "\n");
	fp = fopen(filePath, "wb");
	if (NULL == fp) {
		printf("[-]Error opening file\n");
		return -1;
	}
	int sizeFileRecv = 0;
	recv(sock, &sizeFileRecv, sizeof(sizeFileRecv), 0);
	printf("[+]SIZE IMG: %d\n", sizeFileRecv);
	ssize_t n;
	int total = 0;
	char buff[BUFF_DATA] = {0};
	while ((n = recv(sock, buff, BUFF_DATA, 0)) > 0) {
		if (n == -1) {
			perror("[-]Receive File Error");
			exit(1);
		}
		// if (total + n >= sizeFileRecv) {
		// 	n = sizeFileRecv - total;
		// }
		if (fwrite(buff, 1, n, fp) != n) {
			perror("[-]Write File Error");
			exit(1);
		}
		total += n;
		memset(buff, '\0', BUFF_DATA);
		if(total >= sizeFileRecv) {
			break;
		}
	}
	printf(FG_GREEN "[+]File OK....Completed" NORMAL "\n");
	printf(FG_GREEN "[+]TOTAL RECV: %d\n" NORMAL, total);
	fclose(fp);
	return 1;
}

// Hàm gửi tín hiệu code tương ứng - OK
void sendCode(int sock, int code) {
	char codeStr[10];
	sprintf(codeStr, "%d", code);
	sendWithCheck(sock, codeStr, strlen(codeStr) + 1);
}

// Xứ lí dấu enter - OK
void str_trim_lf(char *arr, int length) {
	int i;
	for (i = 0; i < length; i++) {
		if (arr[i] == '\n') {
			arr[i] = '\0';
			break;
		}
	}
}

void clearBuff() {
	char c;
	while ((c = getchar()) != '\n' && c != EOF) {
	}
}
