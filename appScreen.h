#include <gtk/gtk.h>

// Màn hình đăng nhập đăng ký
typedef struct{
    GtkWidget *window;
    GtkWidget *label_alert;
    GtkWidget *login_user_name;
    GtkWidget *login_password;
    GtkWidget *register_user_name;
    GtkWidget *register_password;
} PreLoginContainer;

typedef struct{
    GtkWidget *window;
    GtkWidget *fileNameLabel;
    GtkWidget *fileNameEntry;
    GtkWidget *box;
} FindContainer;

// Màn hình hiển thị ảnh để tải
typedef struct{
    GtkWidget *window;
} ShowResultContainer;

// Màn hình hiển thị ảnh đã tải
typedef struct{
    GtkWidget *window;
} ShowResourcesContainer;

// Màn hình chính
typedef struct {
    PreLoginContainer preLoginContainer;
    FindContainer findContainer;
    ShowResultContainer showResultContainer;
    ShowResourcesContainer showResourcesContainer;
} ScreenApp;

// Người dùng
typedef struct{
    ScreenApp *screenApp;
    int sockFd;
    char username[50];
} UserData;