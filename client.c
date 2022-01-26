#include <stdio.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "appScreen.h"
#include "communication_code.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "transfer.h"
#define BUFF_SIZE 100

GtkWidget *label_alert = NULL;
GtkWidget *signup_alert_success = NULL;
GtkWidget *signup_alert_faild = NULL;
GtkWidget *label_alert_logged = NULL;
UserData *userData;
char main_message[100] = "";
int is_login = 0;

void initApp();

// ================ CREATE WINDOW =====================
GtkWidget *createWindow(const gint width, const gint height, const gchar *const title);
GtkWidget *create_login_box(GtkWidget *stack);
GtkWidget *create_stack_box(GtkWidget **stack);
GtkWidget *create_login_grid(GtkWidget *stack);
GtkWidget *create_register_grid(GtkWidget *stack);
GtkWidget *create_home_box(GtkWidget *stack);
GtkWidget *create_find_box(GtkWidget *stack);
void initPreLoginScreen();
void create_show_img_downloaded();
void create_show_img_grid();
void create_alert_window(char *label_str);
void load_css(void);

// ================ SWITCH ROUTER =====================
void clicked_clbk(GtkButton *button, GtkStack *stack);
void login_clbk(GtkButton *button, GtkStack *stack);
void main_clbk(GtkButton *button, GtkStack *stack);
void register_clbk(GtkButton *button, GtkStack *stack);
void back_clbk(GtkButton* button, GtkStack *stack);
void home_clbk(GtkButton *button, GtkStack *stack);
void find_clbk(GtkButton *button, GtkStack *stack);
void exit_find_clbk(GtkButton *button, GtkStack *stack);

// ================ THREAD ====================
void recv_msg_handler();

// ================ EVENT CLICK =====================
void download_img(GtkButton *button);
void quit_clbk(GtkButton *button);
void enter_login(GtkButton *button);
void enter_signUp(GtkButton *button);
void find_img(GtkButton *button);
void exitFind(GtkButton *button);
void do_not_download(GtkButton *button);
void delete_img_downloaded(GtkButton *button);
void delete_all_img_func(GtkButton *button);
void quit_sys_clbk(GtkButton *button);
void quit_show_img_downloaded();
void quit_alert();
void destroy_sys();

// ================  =====================
void toUpperString(char *str);
void *SendFileToServer(int new_socket, char *fname);
int count_file_in_dir(char *dir_name);
int initSocket(char *ip_address, int port, UserData *userData);

// =========================================
void initApp() {
    initPreLoginScreen(userData);
}

// ================ MAIN =====================
int main(int argc, char *argv[]) {
    userData = (UserData*)malloc(sizeof(UserData));
    userData->screenApp = (ScreenApp*)malloc(sizeof(ScreenApp));
    gtk_init(&argc, &argv);

    if ((userData->sockFd = initSocket("127.0.0.1", 8888, userData)) <= 0)
        return userData->sockFd;

    initApp(userData);
    free(userData);
    return 0;
}

// =============================================

// ================ CREATE WINDOW =====================

// Tạo màn hình chứa stack cửa sổ - OK
void initPreLoginScreen() {
    GtkWidget *window, *login_box, *login_grid, *register_grid, *stack_box, *stack;
    GtkWidget *home_system, *find_grid;
    load_css();

    /// *** Create a Window
    window = createWindow(600, 400, "SHARE IMAGE APPPICATION");

    /// *** Create the Stack Box
    stack_box = create_stack_box(&stack);
    gtk_container_add(GTK_CONTAINER(window), stack_box);

    /// ***
    login_box = create_login_box(stack);
    login_grid = create_login_grid(stack);
    register_grid = create_register_grid(stack);
    home_system = create_home_box(stack);
    find_grid = create_find_box(stack);

    /// ***
    gtk_stack_add_named(GTK_STACK(stack), login_box, "Main");
    gtk_stack_add_named(GTK_STACK(stack), login_grid, "Login");
    gtk_stack_add_named(GTK_STACK(stack), register_grid, "Register");
    gtk_stack_add_named(GTK_STACK(stack), home_system, "Home_system");
    gtk_stack_add_named(GTK_STACK(stack), find_grid, "Find_img");

    /// ***
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_OVER_DOWN_UP);
    gtk_stack_set_transition_duration(GTK_STACK(stack), 800);
    gtk_stack_set_interpolate_size(GTK_STACK(stack), TRUE);
    userData->screenApp->preLoginContainer.window = window;

    /// ***
    gtk_widget_show_all(window);
    gtk_widget_hide(userData->screenApp->preLoginContainer.label_alert);
    gtk_widget_hide(signup_alert_success);
    gtk_widget_hide(signup_alert_faild);
    gtk_widget_hide(label_alert_logged);

    gtk_main();
}

// CSS - OK
void load_css(void) {
    GtkCssProvider *provider;
    GdkDisplay *display;
    GdkScreen *screen;
    /// ***
    const gchar *css_style_file = "style.css";
    GFile *css_fp = g_file_new_for_path(css_style_file);
    GError *error = 0;
    /// ***
    provider = gtk_css_provider_new();
    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    /// ***
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_file(provider, css_fp, &error);
    /// ***
}

// Tạo một cửa sổ mới - OK
GtkWidget *createWindow(const gint width, const gint height, const gchar *const title) {
    GtkWidget *window;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), title);
    g_signal_connect(window, "destroy", destroy_sys, NULL);
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
    gtk_container_set_border_width(GTK_CONTAINER(window), 50);
    return window;
}

// Tạo màn hình lựa chọn đăng nhập đăng ký - OK 
GtkWidget *create_login_box(GtkWidget *stack) {
    GtkWidget *box, *login_button, *register_button, *close_button;
    GtkWidget *image;

    /// *** Create the Box
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GdkPixbuf *pb;

    pb = gdk_pixbuf_new_from_file("./assets/bk.png", NULL);
    pb = gdk_pixbuf_scale_simple(pb, 250, 250, GDK_INTERP_BILINEAR);
    image = gtk_image_new_from_pixbuf(gdk_pixbuf_copy(pb));
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pb);

    /// *** Create the Buttons
    login_button = gtk_button_new_with_label("Login");
    register_button = gtk_button_new_with_label("Register");
    close_button = gtk_button_new_with_label("EXIT");

    /// *** Add them to the Box
    gtk_box_pack_start(GTK_BOX(box), login_button, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), register_button, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), close_button, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_image_new_from_pixbuf(pb), 0, 0, 10);

    /// ***
    g_signal_connect(login_button, "clicked", G_CALLBACK(login_clbk), stack);
    g_signal_connect(register_button, "clicked", G_CALLBACK(register_clbk), stack);
    g_signal_connect(close_button, "clicked", G_CALLBACK(quit_sys_clbk), userData);

    /// *** Return the Box
    return box;
}

// Màn hình đăng nhập - OK
GtkWidget *create_login_grid(GtkWidget *stack) {
    GtkWidget *grid, *image;
    GtkWidget *login_button, *back_button;
    GtkWidget *label_username, *entry_username;
    GtkWidget *label_password, *entry_password;

    GdkPixbufAnimation *animation;
    GdkPixbuf *pb;

    animation = gdk_pixbuf_animation_new_from_file("./assets/hello2.gif", NULL);
    pb = gdk_pixbuf_animation_get_static_image(animation);
    pb = gdk_pixbuf_scale_simple(pb, 150, 150, GDK_INTERP_BILINEAR);
    image = gtk_image_new_from_pixbuf(gdk_pixbuf_copy(pb));
    gtk_image_set_from_animation (GTK_IMAGE(image), animation);
    load_css();

    /// *** Create the Grid
    grid = gtk_grid_new();

    /// ***
    label_username = gtk_label_new("Username:");
    label_password = gtk_label_new("Password:");
    label_alert = gtk_label_new("User name or Password incorrect!!!");
    label_alert_logged = gtk_label_new("The account is logged");

    /// ***
    entry_username = gtk_entry_new();
    userData->screenApp->preLoginContainer.login_user_name = entry_username;
    entry_password = gtk_entry_new();
    gtk_entry_set_visibility((GtkEntry *)entry_password, FALSE);
    userData->screenApp->preLoginContainer.login_password = entry_password;
    userData->screenApp->preLoginContainer.label_alert = label_alert;

    /// ***
    login_button = gtk_button_new_with_label("Login");
    back_button = gtk_button_new_with_label("Back to Main");

    /// ***
    gtk_grid_attach(GTK_GRID(grid), label_username, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_username, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label_password, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_password, 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), back_button, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), login_button, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_alert, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_alert_logged, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), image, 1, 4, 1, 1);

    gtk_grid_set_row_spacing((GtkGrid *)grid, 10);
    gtk_grid_set_column_spacing((GtkGrid *)grid, 10);
    /// ***
    g_signal_connect(back_button, "clicked", G_CALLBACK(main_clbk), stack);
    g_signal_connect(login_button, "clicked", G_CALLBACK(home_clbk), stack);
    /// ***
    return grid;
}

// Màn hình đăng ký - OK
GtkWidget *create_register_grid(GtkWidget *stack) {
    GtkWidget *grid, *image;
    GtkWidget *register_button, *back_button;
    GtkWidget *label_username, *entry_username;
    GtkWidget *label_password, *entry_password;
    load_css();

    /// *** Create the Grid
    grid = gtk_grid_new();

    /// ***
    label_username = gtk_label_new("Username: ");
    label_password = gtk_label_new("Password:");
    signup_alert_success = gtk_label_new("Successful account registration");
    signup_alert_faild = gtk_label_new("Account registration failed");

    GdkPixbufAnimation *animation;
    GdkPixbuf *pb;

    animation = gdk_pixbuf_animation_new_from_file("./assets/hello2.gif", NULL);
    pb = gdk_pixbuf_animation_get_static_image(animation);
    pb = gdk_pixbuf_scale_simple(pb, 150, 150, GDK_INTERP_BILINEAR);
    image = gtk_image_new_from_pixbuf(gdk_pixbuf_copy(pb));
    gtk_image_set_from_animation (GTK_IMAGE(image), animation);

    /// ***
    entry_username = gtk_entry_new();
    entry_password = gtk_entry_new();
    gtk_entry_set_visibility((GtkEntry *)entry_password, FALSE);
    userData->screenApp->preLoginContainer.register_user_name = entry_username;
    userData->screenApp->preLoginContainer.register_password = entry_password;
    userData->screenApp->preLoginContainer.label_alert = label_alert;

    /// ***
    register_button = gtk_button_new_with_label("Register");
    back_button = gtk_button_new_with_label("Back to Main");

    /// ***
    gtk_grid_attach(GTK_GRID(grid), label_username, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_username, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_password, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_password, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), back_button, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), register_button, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), signup_alert_success, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), signup_alert_faild, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), image, 1, 4, 1, 1);

    /// ***
    g_signal_connect(back_button, "clicked", G_CALLBACK(main_clbk), stack);
    g_signal_connect(register_button, "clicked", G_CALLBACK(enter_signUp), userData);
    gtk_grid_set_row_spacing((GtkGrid *)grid, 10);

    /// ***
    return grid;
}

// Khởi tạo stack để  chuyển màn hình - OK
GtkWidget *create_stack_box(GtkWidget **stack) {
    GtkWidget *box;
    /// *** Create the Box
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    /// *** Create a Stack
    *stack = gtk_stack_new();
    gtk_widget_set_halign(*stack, GTK_ALIGN_CENTER);
    gtk_box_set_center_widget(GTK_BOX(box), *stack);

    /// ***
    return box;
}

// Tạo màn hình lựa chọn tìm kiếm ảnh, xem ảnh đã tải - OK
GtkWidget *create_home_box(GtkWidget *stack) {
    GtkWidget *box, *findLabel, *find_button,  *exit_button, *logou_button, *show_img;
    GtkWidget *grid, *image;

    load_css();
    /// *** Create the Box
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing((GtkGrid *)grid, 10);
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    find_button = gtk_button_new_with_label("FIND IMAGE");
    show_img = gtk_button_new_with_label("SHOW DOWNLOADED IMAGES");
    logou_button = gtk_button_new_with_label("LOG OUT");
    exit_button = gtk_button_new_with_label("EXIT");
    findLabel = gtk_label_new("WELCOME TO SYSTEM");

    GdkPixbuf *pb;

    pb = gdk_pixbuf_new_from_file("./assets/user.png", NULL);
    pb = gdk_pixbuf_scale_simple(pb, 150, 150, GDK_INTERP_BILINEAR);
    image = gtk_image_new_from_pixbuf(gdk_pixbuf_copy(pb));
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pb);
    gtk_grid_attach(GTK_GRID(grid), image, 0, 1, 100, 100);

    gtk_box_pack_start(GTK_BOX(box), findLabel, 0, 1, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_image_new_from_pixbuf(pb), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), find_button, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), show_img, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), logou_button, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), exit_button, 0, 1, 0);

    g_signal_connect(find_button, "clicked", G_CALLBACK(find_clbk), stack);
    g_signal_connect(logou_button, "clicked", G_CALLBACK(exit_find_clbk), stack);
    g_signal_connect(exit_button, "clicked", G_CALLBACK(quit_clbk), userData);
    g_signal_connect(show_img, "clicked", G_CALLBACK(create_show_img_downloaded), userData);

    return box;
}

// Tạo màn hình tìm kiếm ảnh - OK
GtkWidget *create_find_box(GtkWidget *stack) {
    GtkWidget *fileNameLabel, *fileNameEntry;
    GtkWidget *findBtn, *backBtn, *show_temp_btn, *label_alert_img;
    GtkWidget *grid, *box;
    load_css();

    grid = gtk_grid_new();
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    fileNameLabel = gtk_label_new("File name");
    fileNameEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(fileNameEntry), "Please enter the name of file");

    backBtn = gtk_button_new_with_label("Back");
    findBtn = gtk_button_new_with_label("Find");
    show_temp_btn = gtk_button_new_with_label("DOWNLOAD IMG");

    gtk_box_pack_start(GTK_BOX(box), fileNameLabel, 0, 1, 0);
    gtk_box_pack_start(GTK_BOX(box), fileNameEntry, 0, 1, 0);
    gtk_box_pack_start(GTK_BOX(box), findBtn, 0, 1, 0);
    gtk_box_pack_start(GTK_BOX(box), show_temp_btn, 0, 1, 0);
    gtk_box_pack_start(GTK_BOX(box), backBtn, 0, 1, 0);

    userData->screenApp->findContainer.fileNameEntry = fileNameEntry;
    gtk_grid_set_row_spacing((GtkGrid *)grid, 10);

    g_signal_connect(findBtn, "clicked", G_CALLBACK(find_img), userData);
    g_signal_connect(backBtn, "clicked", G_CALLBACK(back_clbk), stack);
    g_signal_connect(show_temp_btn, "clicked", G_CALLBACK(create_show_img_grid), userData);

    return box;
}

// Tạo màn hình xem ảnh đã tải - OK
void create_show_img_downloaded() {
    GtkWidget *window, *grid;
    GtkWidget *image, *image_button, *back_btn, *alert_no_img, *delete_all_img;
    GdkPixbuf *pb;
    window = createWindow(500, 360, "ダウンロードした写真");
    grid = gtk_grid_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_grid_set_column_spacing((GtkGrid *)grid, 10);
    int k = 0, count_img = 0;
    char list_img_clients[1024] = "";
    back_btn = gtk_button_new_with_label("BACK");
    delete_all_img = gtk_button_new_with_label("DELETE ALL");
    alert_no_img = gtk_label_new("ダウンロードした画像はありません");
    DIR *d;
    struct dirent *dir;
    d = opendir("./download_imgs/");
    if (d) {
      while ((dir = readdir(d)) != NULL) {
          if(dir->d_type == DT_REG) {
            strcat(list_img_clients, dir->d_name);
            strcat(list_img_clients, "*");
            count_img++;
          }
      }
      closedir(d);
    }
    char *fileName = strtok(list_img_clients, "*");
    for (int i = 0; i <= count_img; i++) {
        if(i == count_img) {
            if(count_img == 0) {
                GdkPixbufAnimation *animation;
                GdkPixbuf *pb;

                animation = gdk_pixbuf_animation_new_from_file("./assets/trash.gif", NULL);
                pb = gdk_pixbuf_animation_get_static_image(animation);
                pb = gdk_pixbuf_scale_simple(pb, 200, 200, GDK_INTERP_BILINEAR);
                image = gtk_image_new_from_pixbuf(gdk_pixbuf_copy(pb));
                gtk_image_set_from_animation (GTK_IMAGE(image), animation);

                gtk_box_pack_start(GTK_BOX(box), alert_no_img, 0, 1, 0);
                gtk_box_pack_start(GTK_BOX(box), image, 0, 1, 0);
                gtk_box_pack_start(GTK_BOX(box), back_btn, 0, 1, 0);
                break;
            }
            gtk_grid_attach(GTK_GRID(grid), delete_all_img, 0, count_img / 3 + 2, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), back_btn, 1, count_img / 3 + 2, 1, 1);
            break;
        }
        char file_path[200];
        sprintf(file_path, "./download_imgs/%s", fileName);
        image_button = gtk_button_new_with_label(fileName);
        pb = gdk_pixbuf_new_from_file(file_path, NULL);
        pb = gdk_pixbuf_scale_simple(pb, 100, 100, GDK_INTERP_BILINEAR);
        image = gtk_image_new_from_pixbuf(gdk_pixbuf_copy(pb));
        gtk_image_set_from_pixbuf(GTK_IMAGE(image), pb);
        gtk_grid_attach(GTK_GRID(grid), image, k, (i / 3) * 2, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), image_button, k, (i / 3) * 2 + 1, 1, 1);
        if (k == 2)
            k = 0;
        else
            k++;
        fileName = strtok(NULL, "*");
        g_signal_connect(image_button, "clicked", G_CALLBACK(delete_img_downloaded), NULL);
    }
    gtk_box_pack_start(GTK_BOX(box), grid, 0, 1, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    gtk_window_set_deletable((GtkWindow*)window, FALSE);
    userData->screenApp->showResourcesContainer.window = window;
    g_signal_connect(back_btn, "clicked", G_CALLBACK(quit_show_img_downloaded), userData);
    g_signal_connect(delete_all_img, "clicked", G_CALLBACK(delete_all_img_func), NULL);

    gtk_window_set_position((GtkWindow*)window, GTK_WIN_POS_MOUSE);
    gtk_widget_show_all(window);
}

// Tạo cửa sổ thông báo - OK
void create_alert_window(char *label_str) {
    GtkWidget *window, *label_STR, *button_OK, *box; 
    window = createWindow(400, 150, "結果");
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    label_STR = gtk_label_new(label_str);
    button_OK = gtk_button_new_with_label("OK");
    gtk_box_pack_start(GTK_BOX(box), label_STR, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), button_OK, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    gtk_window_set_deletable((GtkWindow*)window, FALSE);
    label_alert = window;
    g_signal_connect_swapped(button_OK, "clicked", G_CALLBACK(quit_alert), NULL);
    gtk_window_set_position((GtkWindow*)window, GTK_WIN_POS_MOUSE);

    gtk_widget_show_all(window);
}

// Tạo màn hình để tải ảnh - OK 
void create_show_img_grid() {
    if(count_file_in_dir("./temporary_image/") == 0) {
        create_alert_window("NO IMAGE FOUND");
        return;
    }
    GtkWidget *grid, *window;
    window = createWindow(500, 360, "SHARE IMAGE APPPICATION");
    grid = gtk_grid_new();
    gtk_grid_set_column_spacing((GtkGrid *)grid, 5);
    gtk_grid_set_row_spacing((GtkGrid *)grid, 10);
    int k = 0, m = 0, count_img = 0;
    char list_img_clients[1024] = "";
    DIR *d;
    struct dirent *dir;
    d = opendir("./temporary_image/");
    if (d) {
      while ((dir = readdir(d)) != NULL) {
          if(dir->d_type == DT_REG) {
            strcat(list_img_clients, dir->d_name);
            strcat(list_img_clients, "*");
            count_img++;
          }
      }
      closedir(d);
    }
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    char *fileName = strtok(list_img_clients, "*");
    GtkWidget *not_download = gtk_button_new_with_label("DON'T WANT DOWNLOAD");
    GtkWidget *label_choose = gtk_label_new("PLEASE CHOOSE IMAGE TO DOWNLOAD");
    gtk_box_pack_start(GTK_BOX(box), label_choose, 0, 1, 0);
    // gtk_grid_attach(GTK_GRID(grid), label_choose, 1, 0, 1, 1);
    for (int i = 0; i < count_img; i++) {
        GdkPixbuf *pb;
        GtkWidget *image;
        GtkWidget *image_button;
        gchar *name;
        char file_path[200];
        image_button = gtk_button_new_with_label(fileName);
        sprintf(file_path, "./temporary_image/%s", fileName);
        pb = gdk_pixbuf_new_from_file(file_path, NULL);
        pb = gdk_pixbuf_scale_simple(pb, 100, 100, GDK_INTERP_BILINEAR);
        image = gtk_image_new_from_pixbuf(gdk_pixbuf_copy(pb));
        gtk_image_set_from_pixbuf(GTK_IMAGE(image), pb);
        gtk_grid_attach(GTK_GRID(grid), image, k, (i / 3) * 2, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), image_button, k, (i / 3) * 2 + 1, 1, 1);
        if (k == 2)
            k = 0;
        else
            k++;
        
        fileName = strtok(NULL, "*");
        g_signal_connect(image_button, "clicked", G_CALLBACK(download_img), userData);
        g_signal_connect(not_download, "clicked", G_CALLBACK(do_not_download), userData);
    } 
    gtk_box_pack_start(GTK_BOX(box), grid, 0, 1, 0);
    gtk_box_pack_start(GTK_BOX(box), not_download, 0, 1, 0);
    gtk_container_add(GTK_CONTAINER(window), box);
    gtk_window_set_deletable((GtkWindow*)window, FALSE);
    userData->screenApp->showResultContainer.window = window;

    gtk_window_set_position((GtkWindow*)window, GTK_WIN_POS_MOUSE);
    gtk_widget_show_all(window);
}

// ================ END CREATE WINDOW =====================

// ================ SWITCH ROUTER =====================
 
// Chuyển đến màn hình lựa chọn đăng nhập đăng ký - OK
void main_clbk(GtkButton *button, GtkStack *stack) {
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Main", GTK_STACK_TRANSITION_TYPE_SLIDE_DOWN);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
}

// Chuyển đến màn hình đăng nhập - OK
void login_clbk(GtkButton *button, GtkStack *stack) {
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Login", GTK_STACK_TRANSITION_TYPE_SLIDE_UP);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
}

// Chuyen den man hinh home - OK
void home_clbk(GtkButton *button, GtkStack *stack) {
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));
    enter_login(button);
    if(is_login == 1) {
        gtk_stack_set_visible_child_full(stack, "Home_system", GTK_STACK_TRANSITION_TYPE_SLIDE_UP);
        g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
    }
}

// Chuyển đến màn hình đăng ký - OK
void register_clbk(GtkButton *button, GtkStack *stack) {
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Register", GTK_STACK_TRANSITION_TYPE_SLIDE_UP);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
} 

// Chuyen den man hinh tim kiem anh - OK
void find_clbk(GtkButton *button, GtkStack *stack) {
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Find_img", GTK_STACK_TRANSITION_TYPE_SLIDE_UP);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
}

// Dang xuat va tro ve man hinh dang nhap - OK
void exit_find_clbk(GtkButton *button, GtkStack *stack) {
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Login", GTK_STACK_TRANSITION_TYPE_SLIDE_UP);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));

    char send_request[1024];
    sprintf(send_request, "%d*%s", LOGOUT_REQUEST, userData->username);
    sendWithCheck(userData->sockFd, send_request, sizeof(send_request));
    memset(send_request, '\0', strlen(send_request) + 1);

    is_login = 0;
    gtk_entry_set_text((GtkEntry*)userData->screenApp->preLoginContainer.login_user_name, "");
    gtk_entry_set_text((GtkEntry*)userData->screenApp->preLoginContainer.login_password, "");
}

// Chuyển đến màn hình lựa chọn tìm kiếm ảnh, xem ảnh đã tải - OK
void back_clbk(GtkButton* button, GtkStack *stack) {
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Home_system", GTK_STACK_TRANSITION_TYPE_SLIDE_DOWN);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
}

// ================ END SWITCH ROUTER =====================

// ================ THREAD =====================

// Luồng xử lí nhận thông điệp từ server - OK
void recv_msg_handler() {
    int REQUEST;
	int sockfd = userData->sockFd;
	char recvReq[1024];
	char sendReq[1024];
	char *fileName;
	while (1) {
		char message[BUFF_SIZE] = {}; 
		int receive = readWithCheck(sockfd, recvReq, REQUEST_SIZE);
        printf("RECV MESSAGE: %s\n", recvReq);
		if(receive <= 0) {
            continue;
        }
        char *opcode;
        char file_path[200];
		opcode = strtok(recvReq, "*");
		if (receive > 0) {
			REQUEST = atoi(opcode);
			switch (REQUEST) {
			case FIND_IMG_IN_USERS:
				printf(FG_GREEN "\n[+]FIND_IMG_IN_USERS\n");
				fileName = strtok(NULL, "*");
                sprintf(file_path, "./client_file/%s", fileName);
				printf(FG_GREEN"[+]FILENAME_TO_SEARCH : %s \n"NORMAL, fileName);
				// neu tim thay:
				if(access(file_path, F_OK) != -1) {
					sprintf(sendReq, "%d*%s", FILE_WAS_FOUND, userData->username);
					send(sockfd, sendReq, sizeof(sendReq), 0);
					printf(FG_GREEN"[+]FILE_WAS_FOUND\n"NORMAL);
					SendFileToServer(sockfd, file_path);
					printf(FG_GREEN"[+]SEND FILE DONE\n"NORMAL);
				}else {
					sprintf(sendReq, "%d*%s", FILE_WAS_NOT_FOUND, userData->username);
					send(sockfd, sendReq, sizeof(sendReq), 0);
                    memset(sendReq, '\0', strlen(sendReq) + 1);
				}
                memset(file_path, '\0', strlen(file_path) + 1);
				break;
            case SEND_IMGS_TO_USER:
                fileName = strtok(NULL, "*");
                sprintf(file_path, "./temporary_image/%s.jpg", fileName);
                receiveUploadedFile(sockfd, file_path);
                memset(file_path, '\0', strlen(file_path) + 1);
                memset(fileName, '\0', strlen(fileName) + 1);
                break;
			case NO_IMG_FOUND:
				printf(FG_RED "No images found!!!\n" );
				break;
            case LOGOUT_SUCCESS:
                return;
			default:
				break;
			}
		}
	}
}

// ================ END THREAD=====================

// ================ EVENT CLICK =====================

// Xứ lí nút enter_login - OK
void enter_login(GtkButton *button) {
    const gchar *userNameData = gtk_entry_get_text((GtkEntry *)userData->screenApp->preLoginContainer.login_user_name);
    const gchar *pass = gtk_entry_get_text((GtkEntry *)userData->screenApp->preLoginContainer.login_password);
    char sign_in_request[1024];
    char buff[BUFF_SIZE];
    printf("[+] USERNAME: %s\t PASSWORD: %s\n", userNameData, pass);
    sprintf(sign_in_request, "%d*%s*%s", LOGIN_REQUEST, userNameData, pass);
    sendWithCheck(userData->sockFd, sign_in_request, sizeof(sign_in_request));
    readWithCheck(userData->sockFd, buff, BUFF_SIZE);
    if(atoi(buff) == IS_CURRENTLY_LOGGED) {
        gtk_widget_show(label_alert_logged);
        printf("[-]Login failed!!\n");
        return;
    }
    if (atoi(buff) != LOGIN_SUCCESS) {
        gtk_widget_show(userData->screenApp->preLoginContainer.label_alert);
        printf("[-]Login failed!!\n");
        return;
    }else {
        strcpy(userData->username, userNameData);
        printf("[+]Login success!!!\n");
        is_login = 1;
        pthread_t recv_msg_thread;			
		if (pthread_create(&recv_msg_thread, NULL, (void *)recv_msg_handler, userData) != 0) {
			printf("[-]ERROR: pthread\n");
			exit(EXIT_FAILURE);
		}
    }
}

// Xứ lí nút enter_signup - OK
void enter_signUp(GtkButton *button) {
    const gchar *userNameData = gtk_entry_get_text((GtkEntry *)userData->screenApp->preLoginContainer.register_user_name);
    const gchar *pass = gtk_entry_get_text((GtkEntry *)userData->screenApp->preLoginContainer.register_password);

    char sign_in_request[1024];
    char buff[BUFF_SIZE];
    printf("USERNAME: %s\t PASSWORD: %s\n", userNameData, pass);
    sprintf(sign_in_request, "%d*%s*%s", REGISTER_REQUEST, userNameData, pass);
    sendWithCheck(userData->sockFd, sign_in_request, sizeof(sign_in_request));
    readWithCheck(userData->sockFd, buff, BUFF_SIZE);
    if (atoi(buff) == REGISTER_SUCCESS) {
        gtk_entry_set_text((GtkEntry*)userData->screenApp->preLoginContainer.register_user_name, "");
        gtk_entry_set_text((GtkEntry*)userData->screenApp->preLoginContainer.register_password, "");
        gtk_widget_show(signup_alert_success);
        gtk_widget_hide(signup_alert_faild);
        printf(FG_GREEN "\n[+]Successful account registration!!!\n");
        return;
    }else if(atoi(buff) == EXISTENCE_USERNAME) {
        gtk_widget_hide(signup_alert_success);
        gtk_widget_show(signup_alert_faild);
        printf(FG_GREEN "\n[+]Account registration failed!!!\n");
        return;
    }
}

// Xử lí nút thoát - OK
void quit_clbk(GtkButton *button) {
    g_print("GoodBye\n");
    char send_request[1024];
    sprintf(send_request, "%d*%s", LOGOUT_REQUEST, userData->username);
    sendWithCheck(userData->sockFd, send_request, sizeof(send_request));
    memset(send_request, '\0', strlen(send_request) + 1);
    sprintf(send_request, "%d*%s", EXIT_SYS, userData->username);
    sendWithCheck(userData->sockFd, send_request, sizeof(send_request));
    memset(send_request, '\0', strlen(send_request) + 1);
    gtk_main_quit();
    free(userData);
    exit(EXIT_SUCCESS);
}

// Xử lí nút download ảnh - OK
void download_img(GtkButton *button) {
    gchar *text = gtk_button_get_label(button);
    char file_path1[1024], file_path2[1024];
    sprintf(file_path2, "./temporary_image/%s", text);
    text[strlen(text) - 4] = '\0';
    sprintf(file_path1, "./download_imgs/%s_%s", text, main_message);
    FILE *fp2 = fopen(file_path2, "r");
    FILE *fp1 = fopen(file_path1, "w");
    char buff[1024];
    while(fread(buff, sizeof(char), 1024, fp2) > 0) {
        fwrite(buff, sizeof(char), 1024, fp1);
    }
    fclose(fp1);
    fclose(fp2);
    DIR *d;
    struct dirent *dir;
    d = opendir("./temporary_image/");
    if (d) {
      while ((dir = readdir(d)) != NULL) {
          if(dir->d_type == DT_REG) {
              char file_path[1024];
              sprintf(file_path, "./temporary_image/%s", dir->d_name);
            if(remove(file_path) == 0){
		    	printf("[+] DELETED FILE SUCCESS: %s\n", file_path);
		    }else{
		    	printf("[+] DELETED FILE FAILED: %s\n", file_path);
		    }
          }
      }
      closedir(d);
    }
    memset(main_message, '\0', strlen(main_message) + 1);
    gtk_widget_hide(userData->screenApp->showResultContainer.window);
}

// Xử lí nút không download ảnh - OK
void do_not_download(GtkButton *button) {
    DIR *d;
    struct dirent *dir;
    d = opendir("./temporary_image/");
    if (d) {
      while ((dir = readdir(d)) != NULL) {
          if(dir->d_type == DT_REG) {
              char file_path[1024];
              sprintf(file_path, "./temporary_image/%s", dir->d_name);
            if(remove(file_path) == 0){
		    	printf("[+] DELETED FILE SUCCESS: %s\n", file_path);
		    }else{
		    	printf("[+] DELETED FILE FAILED: %s\n", file_path);
		    }
          }
      }
      closedir(d);
    }
    gtk_widget_hide(userData->screenApp->showResultContainer.window);
}

// Xu li xoa anh da tai - OK
void delete_img_downloaded(GtkButton *button) {
    gchar *text = gtk_button_get_label(button);
    char file_path2[1024];
    sprintf(file_path2, "./download_imgs/%s", text);

    if(remove(file_path2) == 0){
		printf("[+] DELETED FILE SUCCESS: %s\n", file_path2);
	}else{
		printf("[+] DELETED FILE FAILED: %s\n", file_path2);
	}
    memset(file_path2, '\0', strlen(file_path2) + 1);
    gtk_widget_hide(userData->screenApp->showResourcesContainer.window);
}

// Xu li xoa tat ca anh da tai - OK
void delete_all_img_func(GtkButton *button) {
    DIR *d;
    struct dirent *dir;
    d = opendir("./download_imgs/");
    if (d) {
      while ((dir = readdir(d)) != NULL) {
          if(dir->d_type == DT_REG) {
              char file_path[1024];
              sprintf(file_path, "./download_imgs/%s", dir->d_name);
            if(remove(file_path) == 0){
		    	printf("[+] DELETED FILE SUCCESS: %s\n", file_path);
		    }else{
		    	printf("[+] DELETED FILE FAILED: %s\n", file_path);
		    }
          }
      }
      closedir(d);
    }
    gtk_widget_hide(userData->screenApp->showResourcesContainer.window);
}

// Xử lí nút tìm kiếm ảnh - OK
void find_img(GtkButton *button) {
    char send_request[1024];
    const gchar *message = gtk_entry_get_text((GtkEntry*)userData->screenApp->findContainer.fileNameEntry);
    if(strlen(message) > 0) {
        sprintf(send_request, "%d*%s*%s", FIND_IMG_REQUEST, userData->username, message);
        strcpy(main_message, message);
	    sendWithCheck(userData->sockFd, send_request, strlen(send_request) + 1);
        memset(send_request, '\0', strlen(send_request) + 1);
        gtk_entry_set_text((GtkEntry*)userData->screenApp->findContainer.fileNameEntry, "");
    }
}

// Nut thoat he thong - OK
void destroy_sys() {
    g_print("GoodBye\n");
    char send_request[1024];
    sprintf(send_request, "%d*%s", LOGOUT_REQUEST, userData->username);
    sendWithCheck(userData->sockFd, send_request, sizeof(send_request));
    memset(send_request, '\0', strlen(send_request) + 1);
    sprintf(send_request, "%d*%s", EXIT_SYS, userData->username);
    sendWithCheck(userData->sockFd, send_request, sizeof(send_request));
    memset(send_request, '\0', strlen(send_request) + 1);
    gtk_main_quit();
    free(userData);
    exit(EXIT_SUCCESS);
}

// Thoat man hinh dang nhap dang ky - OK
void quit_sys_clbk(GtkButton *button) {
    g_print("GoodBye!!!\n");
    char send_request[1024];
    sprintf(send_request, "%d*%s", EXIT_SYS, userData->username);
    sendWithCheck(userData->sockFd, send_request, sizeof(send_request));
    memset(send_request, '\0', strlen(send_request) + 1);
    gtk_main_quit();
}

// Xử lí nút thoát màn hình tìm kiếm ảnh -> logOut - OK
void exitFind(GtkButton *button) {
    char send_request[1024];
    sprintf(send_request, "%d*%s", LOGOUT_REQUEST, userData->username);
    sendWithCheck(userData->sockFd, send_request, sizeof(send_request));
    memset(send_request, '\0', strlen(send_request) + 1);
    gtk_widget_show(userData->screenApp->preLoginContainer.window);
    gtk_widget_hide(userData->screenApp->findContainer.window);
    free(userData);
    gtk_entry_set_text((GtkEntry*)userData->screenApp->preLoginContainer.login_user_name, "");
    gtk_entry_set_text((GtkEntry*)userData->screenApp->preLoginContainer.login_password, "");
}

// Xử lí thoát màn hình xem ảnh đã tải - OK
void quit_show_img_downloaded() {
    gtk_widget_hide(userData->screenApp->showResourcesContainer.window);
}

// Xử lí thoát cửa sổ thông báo - OK
void quit_alert() {
    gtk_widget_hide(label_alert);
}

// ================ END EVENT CLICK =====================

// ======================       ==========================

// Hàm gửi file cho server - OK
void *SendFileToServer(int new_socket, char fname[50]) {
	SendFile(new_socket, fname);
}

// Hàm in hoa chuỗi - OK
void toUpperString(char *str) {
    for(int i = 0; i < strlen(str); i++) {
        str[i] = toupper(str[i]);
    }
}

// Hàm tính số file trong thư mục - OK
int count_file_in_dir(char *dir_name) {
    int count = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir("./temporary_image/");
    if (d) {
      while ((dir = readdir(d)) != NULL) {
          if(dir->d_type == DT_REG) {
            count++;
          }
      }
      closedir(d);
    }
    return count;
}

// Hàm khởi tạo kết nối socket - OK
int initSocket(char *ip_address, int port, UserData *userData) {
	int sock = 0;
	struct sockaddr_in serv_addr;

	// Try catch false when connecting
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n [-]Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0) {
		printf("\n[-]Invalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\n[-]Connection Failed \n");
		return -1;
	}
    userData->sockFd = sock;
    return userData->sockFd;
}