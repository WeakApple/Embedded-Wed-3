#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/embedded_device"

int main() {
    int fd; // device path
    int user_input; // 실제 사용자 입력
    char converted_input; // 저수준 입출력 함수를 통해 작성하기 위해 정수형의 입력을 형변환하여 저장하는 변수
    int mode = 1; // 2개의 모드를 관리 1: 일반모드, 2: 수동모드

    // 디바이스 파일 연결
    fd = open(DEVICE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    printf("Mode 1: 1\nMode 2: 2\nMode 3: 3\nMode 4: 4\nexit: 5\n");
    while (1) {
        if (mode == 1){
            printf("Type a mode: ");   
        }else{
            printf("LED to enable: ");
        }
        // 입력값 유효성 검사 1
        if (scanf("%d", &user_input) != 1) {
            // 버퍼를 비우는 동작 추가
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
            continue;
        }
        // 입력값 유효성 검사 2
        if (user_input > 5 || user_input < 1){
            continue;
        }
        // 사용자 프로그램 종료 조건 설정
        if (user_input == 5) {
            // 종료전 모드 초기화화
            converted_input = '4';
            if (write(fd, &converted_input, sizeof(char)) < 0) {
                perror("Failed to write to device");
            }
            // 파일연결 해제
            close(fd);
            return EXIT_SUCCESS;
        }
        // 모드 설정 조건부
        if (user_input == 4){
            mode = 1;
        } else if (mode == 1 && user_input== 3){
            mode = 2;
        }
        // 저수준 파일 입출력 함수로 전달하기 위해 문자형으로 형변환    
        converted_input = user_input + 48; // 48: 숫자 0의 아스키코드 값

        
        if (write(fd, &converted_input, sizeof(char)) < 0) {
            perror("Failed to write to device");
        }
    }

    close(fd);
    return EXIT_SUCCESS;
}
