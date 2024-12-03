#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/led_switch"

int main() {
    int fd;
    char input;

    fd = open(DEVICE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    while (1) {
        printf("Enter mode (0: Reset, 1: All, 2: Individual, 3: Manual, 4: Exit): ");
        scanf(" %c", &input);

        if (input == '4') {
            break;
        }

        if (write(fd, &input, sizeof(input))) < 0) {
            perror("Failed to write to device");
        }
    }

    close(fd);
    return EXIT_SUCCESS;
}