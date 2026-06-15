#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

int main() {
    // fake mouse
    struct libevdev *dev          = libevdev_new();
    struct libevdev_uinput *uidev = NULL;

    libevdev_set_name(dev, "USB fake mouse");

    // mouse capabilities
    libevdev_enable_event_type(dev, EV_REL);
    libevdev_enable_event_code(dev, EV_REL, REL_X, NULL);
    libevdev_enable_event_code(dev, EV_REL, REL_Y, NULL);
    libevdev_enable_event_type(dev, EV_KEY);
    libevdev_enable_event_code(dev, EV_KEY, BTN_LEFT, NULL);
    libevdev_enable_event_code(dev, EV_KEY, BTN_RIGHT, NULL);
    libevdev_enable_event_type(dev, EV_SYN);
    libevdev_enable_event_code(dev, EV_SYN, SYN_REPORT, 0);

    int code = libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);
    if (code < 0) {
        fprintf(stderr, "Error creating virtual uinput device: %s (%d)\n", strerror(-code), code);
        fprintf(stderr, "Please verify that you have permission to write to /dev/uinput.\n");
        libevdev_free(dev);
        return -1;
    }

    printf("Successfully created virtual mouse device!\n");
    printf("Wiggling mouse every second. Press Ctrl+C to stop.\n");
    fflush(stdout);

    srand(time(NULL));
    //fire fake events
    while (1) {
        sleep(1);
        int amplitude = rand() % 100 - 50;
        int direction = rand() % 2 == 0 ? REL_X : REL_Y;

        // Skip 0 amplitude movements to ensure we always perform a wiggle
        if (amplitude == 0) { amplitude = 15; }

        printf("Wiggling: %s by %d pixels\n", (direction == REL_X ? "X" : "Y"), amplitude);
        fflush(stdout);

        libevdev_uinput_write_event(uidev, EV_REL, direction, amplitude);
        libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    }

    // Clean up (unreachable but good style)
    libevdev_uinput_destroy(uidev);
    libevdev_free(dev);
    return 0;
}
