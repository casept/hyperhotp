#include <gtk/gtk.h>
#include <libusb-1.0/libusb.h>
#include <pthread.h>
#include <sys/types.h>

#include "../core/hyperhotp.h"
#include "../core/log.h"
#include "../core/usb.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *program_button;
    GtkWidget *reset_button;
} Gui;

typedef struct {
    pthread_mutex_t mutex;
    Gui gui;
    libusb_device_handle *device;
    FIDOCID cid;
} GlobalState;

static GlobalState GLOBAL_STATE;

static void gui_perform_programming(GtkWidget *widget, gpointer data) { (void)data; }

static void gui_perform_reset(GtkWidget *button, gpointer data) {
    (void)data;
    pthread_mutex_lock(&GLOBAL_STATE.mutex);
    gtk_widget_set_sensitive(button, FALSE);
    gtk_button_set_label(GTK_BUTTON(button), "Resetting, press button on device...");
    hyperhotp_reset(GLOBAL_STATE.device, GLOBAL_STATE.cid);
    gtk_widget_set_sensitive(button, TRUE);
    pthread_mutex_unlock(&GLOBAL_STATE.mutex);
}

static void gui_create(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    pthread_mutex_lock(&GLOBAL_STATE.mutex);

    // Create window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Hyperhotp OTP key programmer");
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
    GLOBAL_STATE.gui.window = window;

    // Create programming button
    GtkWidget *program_button = gtk_button_new_with_label("Program device");
    g_signal_connect(program_button, "clicked", G_CALLBACK(gui_perform_programming), NULL);
    gtk_window_set_child(GTK_WINDOW(window), program_button);
    GLOBAL_STATE.gui.program_button = program_button;

    // Create reset button
    GtkWidget *reset_button = gtk_button_new_with_label("Reset device");
    g_signal_connect(reset_button, "clicked", G_CALLBACK(gui_perform_reset), NULL);
    gtk_window_set_child(GTK_WINDOW(window), reset_button);
    GLOBAL_STATE.gui.reset_button = reset_button;

    // Done
    pthread_mutex_unlock(&GLOBAL_STATE.mutex);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    pthread_mutex_init(&GLOBAL_STATE.mutex, NULL);
    pthread_mutex_lock(&GLOBAL_STATE.mutex);
    GLOBAL_STATE.device = usb_init();
    hyperhotp_init(GLOBAL_STATE.device, GLOBAL_STATE.cid);
    pthread_mutex_unlock(&GLOBAL_STATE.mutex);

    // Run GUI
    log_debug("Starting GUI");
    GtkApplication *app = gtk_application_new("com.github.casept.hyperhotp", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(gui_create), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    // Cleanup
    pthread_mutex_lock(&GLOBAL_STATE.mutex);
    g_object_unref(app);
    log_debug("Stopped GUI");
    usb_cleanup(GLOBAL_STATE.device);
    return status;
}
