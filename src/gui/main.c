#include <gtk/gtk.h>
#include <libusb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
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
    bool retry_requested;
} GlobalState;

static GlobalState GLOBAL_STATE;

static void gui_on_perform_programming_requested(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;
}

static void gui_on_perform_reset_requested(GtkWidget *button, gpointer data) {
    (void)data;

    // Setup
    pthread_mutex_lock(&GLOBAL_STATE.mutex);
    gtk_widget_set_sensitive(button, FALSE);
    gtk_widget_set_sensitive(GLOBAL_STATE.gui.program_button, FALSE);
    gtk_button_set_label(GTK_BUTTON(button), "Resetting, press button on device...");
    GtkWidget *fidget_spinner = gtk_spinner_new();
    gtk_spinner_start(GTK_SPINNER(fidget_spinner));

    // Action
    hyperhotp_reset(GLOBAL_STATE.device, GLOBAL_STATE.cid);
    // TODO: Handle failed reset (offer to retry)

    // Cleanup
    gtk_spinner_stop(GTK_SPINNER(fidget_spinner));
    g_object_unref(fidget_spinner);
    gtk_widget_set_sensitive(button, TRUE);
    gtk_widget_set_sensitive(GLOBAL_STATE.gui.program_button, TRUE);
    pthread_mutex_unlock(&GLOBAL_STATE.mutex);
}

static void gui_on_retry_requested(GtkWidget *widget, gpointer data) {
    (void)data;
    (void)widget;
    log_debug("Retry requested");
    GLOBAL_STATE.retry_requested = true;
}

static void gui_on_exit_due_to_failure_requested(GtkWidget *widget, gpointer data) {
    (void)data;
    (void)widget;
    pthread_mutex_lock(&GLOBAL_STATE.mutex);
    gtk_window_destroy(GTK_WINDOW(GLOBAL_STATE.gui.window));
    pthread_mutex_unlock(&GLOBAL_STATE.mutex);
    exit(EXIT_FAILURE);
}

// Displays a modal dialog asking the user to retry.
// If the user declines, the app is closed.
static void gui_show_blocking_retry_modal(GtkWidget *parent_window, const char *msg) {
    const GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL;
    GtkWidget *dialog =
        gtk_message_dialog_new(GTK_WINDOW(parent_window), flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", msg);
    GtkWidget *retry_button = gtk_dialog_add_button(GTK_DIALOG(dialog), "Retry", GTK_RESPONSE_ACCEPT);
    g_signal_connect(retry_button, "clicked", G_CALLBACK(gui_on_retry_requested), NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
    // Destroy dialog on response
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);

    gtk_widget_show(dialog);
    // This is no doubt not the cleanest or most efficient way,
    // but it should be fine as a quick&dirty solution.
    // We don't care about the false case, because then the app will be closed already.
    while (!GLOBAL_STATE.retry_requested) {
    }
    gtk_widget_hide(dialog);
    g_object_unref(dialog);
}

// Queries the current device status.
static void gui_update_device_status(GtkLabel *label_to_update) {
    char serial[HYPERHOTP_SERIAL_LEN];
    pthread_mutex_lock(&GLOBAL_STATE.mutex);
    int err = hyperhotp_check_programmed(GLOBAL_STATE.device, GLOBAL_STATE.cid, serial);
    if (err != 0) {
        // TODO: If this doesn't work, chances are the rest of the app won't work either.
        // Should we instead show an error window and bail?
        char *err_str = log_get_last_error_string();
        GtkWidget *bar = gtk_info_bar_new();
        GtkWidget *label = gtk_label_new(err_str);
        gtk_info_bar_add_child(GTK_INFO_BAR(bar), label);
        log_free_error_string(err_str);
    } else {
        // TODO: Content
        gtk_label_set_markup(label_to_update, "<b>Text to be bold</b>");
    }
    pthread_mutex_unlock(&GLOBAL_STATE.mutex);
}

static void gui_init_device_with_retries(GtkWidget *window) {
    pthread_mutex_lock(&GLOBAL_STATE.mutex);
    // Try to establish connection to device, show error and offer retry if unsuccessful
    while (true) {
        int err = hyperhotp_init(&GLOBAL_STATE.device, GLOBAL_STATE.cid);
        if (err != 0) {
            char *err_string = log_get_last_error_string();
            gui_show_blocking_retry_modal(window, err_string);
            log_free_error_string(err_string);
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&GLOBAL_STATE.mutex);
}

static void gui_create(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    pthread_mutex_lock(&GLOBAL_STATE.mutex);

    // Create window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Hyperhotp OTP key programmer");
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 240);
    GLOBAL_STATE.gui.window = window;

    // Create container for a row-based layout
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_window_set_child(GTK_WINDOW(window), box);

    // Create entry field for hotp serial number
    GtkWidget *serial_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    GtkWidget *serial_label = gtk_label_new("HOTP key serial (8 characters):");
    GtkWidget *serial_entry = gtk_entry_new();
    gtk_box_append(GTK_BOX(serial_box), serial_label);
    gtk_box_append(GTK_BOX(serial_box), serial_entry);
    gtk_box_append(GTK_BOX(box), serial_box);

    // Create entry field for hotp hex key
    GtkWidget *key_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    GtkWidget *key_label = gtk_label_new("HOTP hex key (40 characters):");
    GtkWidget *key_entry = gtk_entry_new();
    gtk_box_append(GTK_BOX(key_box), key_label);
    gtk_box_append(GTK_BOX(key_box), key_entry);
    gtk_box_append(GTK_BOX(box), key_box);

    // Create programming button
    GtkWidget *program_button = gtk_button_new_with_label("Program device");
    g_signal_connect(program_button, "clicked", G_CALLBACK(gui_on_perform_programming_requested), NULL);
    gtk_box_append(GTK_BOX(box), program_button);
    GLOBAL_STATE.gui.program_button = program_button;

    // Create reset button
    GtkWidget *reset_button = gtk_button_new_with_label("Reset device");
    g_signal_connect(reset_button, "clicked", G_CALLBACK(gui_on_perform_reset_requested), NULL);
    gtk_box_append(GTK_BOX(box), reset_button);
    GLOBAL_STATE.gui.reset_button = reset_button;

    // Done initializing GUI
    gtk_window_present(GTK_WINDOW(window));
    pthread_mutex_unlock(&GLOBAL_STATE.mutex);

    // Try to initialize the device, showing errors to the user.
    gui_init_device_with_retries(window);
    GtkWidget *device_status = gtk_label_new(NULL);
    gtk_box_append(GTK_BOX(box), device_status);
    // gui_update_device_status(GTK_LABEL(device_status));
}

int main(int argc, char **argv) {
    pthread_mutex_init(&GLOBAL_STATE.mutex, NULL);

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
