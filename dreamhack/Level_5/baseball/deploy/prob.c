//gcc -o prob prob.c -fstack-protector -no-pie -pthread 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int game_count = 1;

void init() {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    srand(time(NULL));
}

// 랜덤 숫자 생성
void generate_numbers(int *numbers) {
    for (int i = 0; i < 3; i++) {
        numbers[i] = rand() % 9 + 1;
    }
}
__asm__(
        ".global pop_rdi_ret\n"
        "pop_rdi_ret:\n"
        "    pop %rdi;\n"
        "    ret;\n"
 );

// 게임 함수
int play_game() {
    int secret[3];
    int guess[3];
    int correct_count = 0;
    int attempts;

    generate_numbers(secret);

    puts("Welcome to the 3-Digit Baseball Game!");
    puts("Try to guess the 3-digit number.");
    for (attempts = 1; attempts <= 4; attempts++) { 
        int strike = 0, ball = 0;

        printf("Attempt %d - Enter your guess (3 digits, each from 1 to 9): ", attempts);
        scanf("%1d%1d%1d", &guess[0], &guess[1], &guess[2]);

        for (int i = 0; i < 3; i++) {
            if (guess[i] == secret[i]) {
                strike++;
            } else if (guess[i] == secret[(i + 1) % 3] || guess[i] == secret[(i + 2) % 3]) {
                ball++;
            }
        }

        if (strike == 3) {
            printf("Congratulations! You guessed correctly in %d attempts.\n", attempts); 
            correct_count= 100 - (attempts-1) * 10;
            break;
        } else {
            printf("%d Strike, %d Ball\n", strike, ball);
        }
    }

    if (correct_count == 0) {
        printf("You've used all attempts! Better luck next time.\n");
    }
    return correct_count;
}

// 스레드 함수
void *game_thread(void *arg) {
    int score = play_game();
    char nickname[256];
    int name_size = game_count * 512; 
    printf("Your score: %d\n", score);
    puts("Enter your nickname: ");
    read(0, nickname, name_size);
}

int main() {
    pthread_t thread;
    char retry[4];

    init();

    while (1) {
        if (pthread_create(&thread, NULL, (void *)game_thread, NULL) != 0) {
            perror("Failed to create thread");
            exit(1);
        }
        pthread_join(thread, NULL);
        // retry 여부 확인
        printf("Do you want to retry? (yes/no): ");
        scanf("%3s", retry);
        if (strcmp(retry, "yes") == 0) {
            game_count++;
        } else {
            printf("Exiting program.\n");
            break;
        }
    }
    return 0;
}
