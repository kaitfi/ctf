// Name: fast-growing-cat.c
// Compile: gcc -Wall -fno-stack-protector -no-pie fast-growing-cat.c -o fast-growing-cat
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

struct cat_food_t {
    char name[48];
    uint8_t weight;
    uint32_t price;
};

struct cat_food_t *cat_foods[3];

void win() {
    execve("/bin/sh", 0, 0);
}

void print_cat() {
    puts("\n" \
         " /\\_/\\   meow~\n" \
         "( o.o )\n" \
         " > ^ <\n");
}

void cook_cat_food() {
    int idx;
    struct cat_food_t *cat_food;

    cat_food = calloc(1, sizeof(struct cat_food_t));
    if (cat_food == NULL)
        exit(1);

    printf("\nname of cat food? ");
    scanf("%47s", cat_food->name);

    puts("\nevaluating the name...");
    usleep(0.4 * 1000 * 1000);

    switch (rand() % 3) {
    case 0:  // great
        puts("great cat food name :o");
        cat_food->weight = rand() % 256;
        cat_food->price = rand() % 0x100000000;
        break;
    case 1:  // good
        puts("good cat food name :)");
        cat_food->weight = rand() % 30;
        cat_food->price = rand() % 0x10000;
        break;
    default:  // not bad
        puts("not bad cat food name.");
        cat_food->weight = rand() % 4;
        cat_food->price = rand() % 0x1000;
    }

    printf("\nwhich inventory do you want to put it in? ");
    scanf("%d", &idx);

    if (0 <= idx && idx <= 2)
        cat_foods[idx] = cat_food;
}

void feed_cat() {
    int idx;

    printf("\nwhich cat food do you want to feed? ");
    scanf("%d", &idx);

    if (idx < 0 && 2 < idx)
        exit(1);

    if (!cat_foods[idx])
        exit(1);

    free(cat_foods[idx]);

    puts("\n" \
         " /\\_/\\   yummy~ yummy~\n" \
         "( o.o )\n" \
         " > ^ <\n");
}

void init() {
    setvbuf(stdin, 0, _IONBF, 0);
    setvbuf(stdout, 0, _IONBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);

    srand(time(NULL));
}

void read_str(char *buf, size_t size) {
    size_t readn;

    readn = read(0, buf, size);
    if (readn > 0 && buf[readn - 1] == '\n')
        buf[readn - 1] = '\0';
}

void start() {
    char cmd[32];

    init();

    while (1) {
        print_cat();
        memset(cmd, 0x00, 32);

        while (1) {
            printf("meow? ");
            read_str(cmd, 32);
            printf("%s~\n", cmd);

            if (!strncmp(cmd, "miow", 4)) {
                cook_cat_food();
                break;
            } else if (!strncmp(cmd, "meow", 4)) {
                feed_cat();
                break;
            } else if (!strncmp(cmd, "muow", 4)) {
                return;
            }
        }
    }
}

int main(void) {
    start();

    return 0;
}
