#include <stdio.h>
#include <gpiod.h>
#include <unistd.h>
#include <glob.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/timerfd.h>
#include <errno.h>

#define PTP_CLOCK_ID CLOCK_REALTIME // Replace with CLOCK_PTP if supported

// Global flag for interrupt handling
volatile bool running = true;

// Signal handler function
void signal_handler(int signum) {
    if (signum == SIGINT) {
        printf("\nReceived interrupt signal. Cleaning up...\n");
        running = false;
    }
}

int main(int argc, char *argv[]) {
    const char* gpiolineName0 = "CSI0-TRIGGER";
    const char* gpiolineName1 = "CSI1-TRIGGER";
    struct gpiod_line* gpioline0 = NULL;
    struct gpiod_line* gpioline1 = NULL;
    int timer_fd = timerfd_create(PTP_CLOCK_ID, 0);
    if (timer_fd == -1) {
        perror("timerfd_create");
        return -1;
    }

    int nanotime = 28566667;
    int sectime = 0;
    if(argc == 2) {
        nanotime = (int)((float)1e9/atof(argv[1]));
    }
    
    if (nanotime >= 1000000000) {
        sectime = nanotime/1e9;
        nanotime = nanotime - sectime*1e9;
    }
    printf("nanoseconds = %i\n",nanotime);
    printf("seconds     = %i\n",sectime);
    

    struct itimerspec timer_spec = {0};
    timer_spec.it_interval.tv_sec = sectime;
    timer_spec.it_interval.tv_nsec = nanotime; // 60 Hz interval (16.67 ms)
    timer_spec.it_value.tv_sec = sectime;
    timer_spec.it_value.tv_nsec = nanotime;

    if (timerfd_settime(timer_fd, 0, &timer_spec, NULL) == -1) {
        perror("timerfd_settime");
        close(timer_fd);
        return -1;
    }

    // Set up signal handler
    signal(SIGINT, signal_handler);

    gpioline0 = gpiod_line_find(gpiolineName0);
    gpioline1 = gpiod_line_find(gpiolineName1);

    if (gpioline0 == NULL) {
        printf("Invalid line name.\n");
        return 1;
    }
    if (gpioline1 == NULL) {
        printf("Invalid line name.\n");
        return 1;
    }

    if (gpiod_line_request_output(gpioline0, "GPIO application", 0) < 0) {
        printf("Failed to request GPIO line.\n");
        return 1;
    }
    if (gpiod_line_request_output(gpioline1, "GPIO application", 0) < 0) {
        printf("Failed to request GPIO line.\n");
        return 1;
    }

    printf("Waiting for periodic events...\n");
    while (running) {
        uint64_t expirations;
        gpiod_line_set_value(gpioline0, 1);
        gpiod_line_set_value(gpioline1, 1);
        usleep(5000); // 1 ms
        gpiod_line_set_value(gpioline0, 0);
        gpiod_line_set_value(gpioline1, 0);
        ssize_t s = read(timer_fd, &expirations, sizeof(expirations));
        if (s != sizeof(expirations)) {
            perror("read");
            break;
        }
    }

    close(timer_fd);

    // Cleanup
    gpiod_line_set_value(gpioline0, 0);  // Set to low before releasing
    gpiod_line_set_value(gpioline1, 0);  // Set to low before releasing
    gpiod_line_release(gpioline0);
    gpiod_line_release(gpioline1);
    printf("GPIO cleanup completed.\n");

    return 0;
}
