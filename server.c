#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/stat.h>
#include "communication_code.h"
#include "linked_list.h"
#include "transfer.h"
#include <time.h>
#include <pthread.h>

int count_send = 0;
int count_write = 0;
char main_name[BUFF_SIZE] = "";
int num_client = 0;

singleList users;
client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// =============== Function ==================== 

int isLogged(char *user_name);
void queue_add(client_t *cl);
void queue_delete(char *username);
void print_queue();
void readUserFile(singleList *users);
int checkExistence(int type, singleList list, char *string);
void *findByName(int type, singleList list, char string[50]);
void signUp(int sock, singleList *users, char *name, char *pass);
int signIn(int sock, singleList users, user_struct **loginUser, char *name, char *pass);
void send_message(char name[100], char *nameFile);
void send_code_img_not_found();
void *SendFileToClient(int new_socket, char *fname);
void send_message_to_sender(char *file_path, char *username);
void receiveUploadedFileServer(int sock, char filePath[200]);
void *handleThread(void *my_sock);

// ====================================

//==============MAIN==============
int main(int argc, char *argv[]) {
	if (argc == 1) {
		printf("Please input port number\n");
		return 0;
	}
	char *port_number = argv[1];
	int port = atoi(port_number);
	int opt = 1;
	int server_fd, new_socket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("[-]Socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("[-]Setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	// Forcefully attaching socket to the port
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("[-]Bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0) {
		perror("[-]Listen");
		exit(EXIT_FAILURE);
	}

	char buff[100];
	createSingleList(&users);
	readUserFile(&users);
	while (1) {
		pthread_t tid;

		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
			perror("[-]Accept");
			exit(EXIT_FAILURE);
		}
		printf("New request from sockfd = %d.\n", new_socket);
		pthread_create(&tid, NULL, &handleThread, &new_socket);
	}
	close(server_fd);
	for(int k = 0; k < MAX_CLIENTS; k++) {
		free(clients[k]);
	}
	node *pnode = users.root;
	for(; pnode; pnode = pnode->next) {
		pnode = NULL;
	}
	free(&users);
	memset(main_name, '\0', strlen(main_name) + 1);
	count_send = count_write = 0;
	num_client = 0;
	return 0;
}

// =======================================

// check if username is logged in
int isLogged(char *user_name) {
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i]) {
			if(strcmp(clients[i]->name, user_name) == 0) {
				return 1;
			}
		}
	}
	return 0;
}

// Thêm các client đã đăng nhập thành công vào mảng kết nối - OK
void queue_add(client_t *cl) {
	pthread_mutex_lock(&clients_mutex);
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		if (!clients[i]) {
			clients[i] = cl;
			num_client++;
			break;
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

// Xóa client đã đăng xuất ra khỏi mảng đang kết nối - OK
void queue_delete(char *username) {
	pthread_mutex_lock(&clients_mutex);
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		if (clients[i] && strcmp(clients[i]->name, username) == 0) {
			clients[i] = NULL;
			num_client--;
			break;
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

// In ra danh sách client đang kết nối - OK
void print_queue() {
	printf("[+]List clients: \n");
	for (int i = 0; i < num_client; i++) {
		printf("%s\n", clients[i]->name);
	}
}

// Đọc file chứa thông tin user rồi lưu vào danh sách liên kết userList - OK
void readUserFile(singleList *users) {
	char username[50], password[50];
	int status;
	FILE *f = fopen("./storage/user.txt", "r");

	if (f == NULL) {
		perror("[-]Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		char c = fgetc(f);
		if (c != EOF) {
			int res = fseek(f, -1, SEEK_CUR);
		}else {
			break;
		}

		fgets(username, 50, f);
		username[strlen(username) - 1] = '\0';

		fgets(password, 50, f);
		password[strlen(password) - 1] = '\0';

		fscanf(f, "%d\n", &status);

		user_struct *user = (user_struct *)malloc(sizeof(user_struct));
		strcpy(user->user_name, username);
		strcpy(user->password, password);
		user->status = status;
		insertEnd(users, user);
	}
	fclose(f);
}

// Hàm kiểm tra username đã tồn tại chưa - OK
int checkExistence(int type, singleList list, char *string) {
	switch (type) {
	// Check user
	case 1: {
		int i = 0;
		list.cur = list.root;
		while (list.cur != NULL) {
			i++;
			if (strcmp(((user_struct *)list.cur->element)->user_name, string) != 0) {
				list.cur = list.cur->next;
			}else {
				return 1;
			}
		}
		return 0;
	}
	break;

	default:
		printf("[-]Type chua hop le !! (1,2 or 3)\n");
		break;
	}
}

// Tìm kiếm user theo username - OK
void *findByName(int type, singleList list, char string[50]) {
	switch (type) {
	case 1: {
			int i = 0;
			list.cur = list.root;
			while (list.cur != NULL) {
				i++;
				if (strcmp(((user_struct *)list.cur->element)->user_name, string) != 0) {
					list.cur = list.cur->next;
				}else {
					return list.cur->element;
				}
			}
			return NULL;
		}
		break;
	default:
		printf("[-]Type chua hop le !! (1,2 or 3)\n");
		break;
	}
}

// Hàm kiểm tra đăng ký - OK
void signUp(int sock, singleList *users, char *name, char *pass) {
	char buff[BUFF_SIZE];
	int size;
	printf("USERNAME: \'%s\'\n", name);
	if (checkExistence(1, *users, name) == 1) {
		sendCode(sock, EXISTENCE_USERNAME);
	}else {
		user_struct *user = (user_struct *)malloc(sizeof(user_struct));
		strcpy(user->user_name, name);
		strcpy(user->password, pass);
		user->status = 1;
		insertEnd(users, user);
		sendCode(sock, REGISTER_SUCCESS);
		printf("REGISTER SUCCESS\n");
	}
}

// Hàm kiểm tra đăng nhập - OK
int signIn(int sock, singleList users, user_struct **loginUser, char *name, char *pass) {
	if(isLogged(name) == 1) {
		sendCode(sock, IS_CURRENTLY_LOGGED);
		printf("[-] LOGIN FAILED\n");
		return 0;
	}
	if (checkExistence(1, users, name) == 1) {
		*loginUser = (user_struct *)(findByName(1, users, name));
		if (strcmp((*loginUser)->password, pass) == 0) {
			sendCode(sock, LOGIN_SUCCESS);
			client_t *cli = (client_t *)malloc(sizeof(client_t));
			strcpy(cli->name, name);
			cli->sockfd = sock;
			cli->uid = num_client;
			queue_add(cli);
			printf("[+] LOGIN SUCCESS\n");
			return 1;
		}else {
			sendCode(sock, LOGIN_FAILED);
			printf("[-] LOGIN FAILED\n");
			return 0;
		}
	}else {
		sendCode(sock, LOGIN_FAILED);
		printf("[-] LOGIN FAILED\n");
		return 0;
	}
}

// Hàm gửi thông điệp tìm kiếm cho các client khác trừ người gửi - OK
void send_message(char name[100], char *nameFile) {
	char send_request[REQUEST_SIZE];
	for (int i = 0; i < num_client; i++) {
		if (strcmp(name, clients[i]->name) != 0) {
			sprintf(send_request, "%d*%s", FIND_IMG_IN_USERS, nameFile);
			printf("->SEND TO %s - RECV FROM %s - %s - %s \n", clients[i]->name, name, nameFile, send_request);
			send(clients[i]->sockfd, send_request, sizeof(send_request), 0);
			memset(send_request, '\0', strlen(send_request) + 1);
		}
	}
}

// Hàm gửi code không tìm thấy ảnh - OK
void send_code_img_not_found(){ 
	for (int i = 0; i < num_client; i++) {
		if (strcmp(main_name, clients[i]->name) == 0) {
			sendCode(clients[i]->sockfd, NO_IMG_FOUND);
			break;
		}
	}
}

// Hàm gửi file cho client - OK
void *SendFileToClient(int new_socket, char *fname) {
	SendFile(new_socket, fname);
}

// Gửi danh sách tìm thấy cho người dùng tìm kiếm - OK
void send_message_to_sender(char *file_path, char *username) {
	char send_request[REQUEST_SIZE];
	for (int i = 0; i < num_client; i++) {
		if (strcmp(main_name, clients[i]->name) == 0) {
			sprintf(send_request, "%d*%s", SEND_IMGS_TO_USER, username);
			send(clients[i]->sockfd, send_request, sizeof(send_request), 0);
			SendFileToClient(clients[i]->sockfd, file_path);
			printf("SEND_MESSAGE: %s\n", send_request);
			if(remove(file_path) == 0){
				printf("[+] DELETED FILE SUCCESS: %s\n", file_path);
			}else{
				printf("[-] DELETED FILE FAILED: %s\n", file_path);
			}
			if(count_send == count_write) {
				count_send = count_write = 0;
				printf("[+]SEND TO %s DONE\n", clients[i]->name);
				break;
			}
		}
	}
}

// Hàm nhận file gửi lên từ client - OK
void receiveUploadedFileServer(int sock, char filePath[200]){
	if(receiveUploadedFile(sock, filePath)) count_write++;
	else return;
}

// Hàm xử lí luồng - OK
void *handleThread(void *my_sock) {
	int new_socket = *((int *)my_sock);
	int REQUEST;
	char buff[1024];
	char *name, *pass;
	user_struct *loginUser = NULL;

	while (1) {
		int n = readWithCheck(new_socket, buff, 1024);
		if(n <= 0 || strlen(buff) == 0) {
			printf("Close request from sockfd = %d\n", new_socket);
			close(new_socket);
			return NULL;
		}
		char *opcode = strtok(buff, "*");
		REQUEST = atoi(buff);
		switch (REQUEST) {
		case REGISTER_REQUEST:
			name = strtok(NULL, "*");
			pass = strtok(NULL, "*");
			printf("[+]REGISTER_REQUEST\n");
			signUp(new_socket, &users, name, pass);
			saveUsers(users);
			break;
		case LOGIN_REQUEST:
			// nhan username va password
			printf("[+]LOGIN_REQUEST\n");
			name = strtok(NULL, "*");
			pass = strtok(NULL, "*");
			if (signIn(new_socket, users, &loginUser, name, pass) == 1) {
				while (REQUEST != LOGOUT_REQUEST) {
					char *username;
					char *filename;
					char file_path[200];
					if(readWithCheck(new_socket, buff, REQUEST_SIZE) == 0) {
						continue;
					}
					printRequest(buff);
					char *opcode;
					opcode = strtok(buff, "*");
					REQUEST = atoi(opcode);
					switch (REQUEST) {
					case FIND_IMG_REQUEST:
						username = strtok(NULL, "*");
						strcpy(main_name, username);
						filename = strtok(NULL, "*");
						// gui yeu cau toi cac may con lai
						send_message(username, filename);
						count_send = num_client - 1;
						printf("[+]SEND TO ALL : %s\n", filename);
						break;
					case FILE_WAS_FOUND:
						username = strtok(NULL, "*");
						printf("[+]FOUND FROM %s\n", username);  
						str_trim_lf(username, strlen(username));
						sprintf(file_path, "./files/%s.jpg", username);
						pthread_mutex_lock(&clients_mutex);
						receiveUploadedFileServer(new_socket, file_path);
						pthread_mutex_unlock(&clients_mutex);
						printf("[+]AMAZING GOOD JOB\n");
						send_message_to_sender(file_path, username);
						break;
					case FILE_WAS_NOT_FOUND:
						count_send--;
						if(count_send == 0) {
							send_code_img_not_found();
						}
						break;
					case LOGOUT_REQUEST: // request code: 14
						printf("[+]LOGOUT_REQUEST\n");
						username = strtok(NULL, "*");
						queue_delete(username);
						sendCode(new_socket, LOGOUT_SUCCESS);
						printf("[+]LOGOUT SUCCESS\n");
						memset(username, '\0', strlen(username) + 1);
						loginUser = NULL;
						break;
					default:
						break;
					}
				}
			}
			break;
		case EXIT_SYS:
			close(new_socket);
			printf("Close request from sockfd = %d\n", new_socket);
			return NULL;
		default:
			break;
		}
		memset(buff, '\0', strlen(buff) + 1);
	}
}

