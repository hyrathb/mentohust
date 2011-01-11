#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>
#include <dbus/dbus-glib.h>

#ifdef ENABLE_NLS
#include <glib/gi18n.h>
#else
#define _(String) (String)
#define N_(String) (String)
#endif

static gboolean delete_event(GtkWidget *widget,
                             GdkEvent  *event,
                             gpointer   data)
{
    /* FIXME: 关闭至托盘 */
    g_print ("delete event occurred\n");

    return FALSE;
}

static void destroy(GtkWidget *widget, gpointer data)
{
    /* FIXME: 询问是否退出认证 */
    gtk_main_quit ();
}

int main(int argc, char **argv)
{
#ifdef ENABLE_NLS
    gtk_set_locale();
    textdomain(GETTEXT_PACKAGE);
#endif

    GtkWidget *window; /* 主窗口 */
    GtkWidget *vbox; /* 垂直盒子 */
    GtkWidget *hbox; /* 水平盒子 */
    GtkWidget *label; /* 静态标签 */
    GtkWidget *table; /* 表布局 */
    GtkWidget *entry; /* 文本框 */
    GtkWidget *checkButton; /* 复选项 */
    GtkWidget *button; /* 按键 */
    GtkEntryBuffer *buffer; /* 文本框缓存 */

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);

    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    label = gtk_label_new(_("欢迎使用 Mentohust 认证程序"));
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    /* 开始主体表布局 */
    table = gtk_table_new(2, 1, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);

    label = gtk_label_new(_("用户名"));
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);

    label = gtk_label_new(_("密码"));
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2);

    /* 用户名文本框 */
    buffer = gtk_entry_buffer_new (NULL, 0);
    entry = gtk_entry_new_with_buffer(buffer);
    gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 0, 1);

    /* 密码文本框 */
    buffer = gtk_entry_buffer_new (NULL, 0);
    entry = gtk_entry_new_with_buffer(buffer);
    gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
    gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 1, 2);
    /* 完成主体表布局 */

    hbox = gtk_hbox_new(FALSE, 5);

    checkButton = gtk_check_button_new_with_label(_("保存密码"));
    gtk_box_pack_end(GTK_BOX(hbox), checkButton, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    /* 开始底部按键组 */
    hbox = gtk_hbox_new(FALSE, 5);

    button = gtk_button_new_with_label(_("连接"));
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

    button = gtk_button_new_with_label(_("退出"));
    g_signal_connect (button, "clicked", G_CALLBACK (destroy), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
    
    button = gtk_button_new_with_label(_("消息记录"));
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

    button = gtk_button_new_with_label(_("高级设置"));
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    /* 完成底部按键组 */

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
