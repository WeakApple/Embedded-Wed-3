#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/led_switch"

int main() {
    int fd, input_i;
    char input;
    char insert_input;
    int mode = 0;

    fd = open(DEVICE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    printf("Mode 1: 1\nMode 2: 2\nMode 3: 3\nMode 4: 4\nexit: 5\n");
    while (1) {
        if (mode == 0){
            printf("Type a mode: ");   
        }else{
            printf("LED to enable: ");
        }
        scanf(" %d", &input_i);

        input = input_i + 48;

        if (input_i > 5 || input_i < 1){
            continue;
        } else if (input_i == 4){
            mode = 0;
        } else if (mode == 0 && input_i == 3){
            mode = 1;
        }
            
        

        if (input == '5') {
            close(fd);
            return EXIT_SUCCESS;
        }

        if (write(fd, &input, sizeof(char)) < 0) {
            perror("Failed to write to device");
        }
    }

    close(fd);
    return EXIT_SUCCESS;
}
