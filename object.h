typedef struct node node;

typedef struct node {
	void *element;
	node *next;
}node;

typedef struct singleList{
  node * root;
  node * cur;
  node * prev;
  node * tail;
} singleList;


typedef struct user
{
	char user_name[50];
	char password[50];
	int status;
}user_struct;

typedef struct
{
	int sockfd;
	int uid;
	char name[100];
} client_t;


