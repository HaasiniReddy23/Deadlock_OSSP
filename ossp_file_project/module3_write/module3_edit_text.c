#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE 1024

void remove_newline(char text[])
{
    text[strcspn(text, "\n")] = '\0';
}

void show_file(const char path[])
{
    char buffer[SIZE];
    int fd = open(path, O_RDONLY);

    if (fd == -1) {
        perror("Could not open file");
        return;
    }

    printf("\n--- Current file content ---\n");

    ssize_t count;
    while ((count = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, (size_t)count);
    }

    if (count == -1) {
        perror("Could not read file");
    }

    printf("\n--- End of content ---\n");
    close(fd);
}

int main(void)
{
    char path[SIZE];
    char answer[32];
    char text[SIZE];

    printf("Enter the path of an existing text file: ");
    if (fgets(path, sizeof(path), stdin) == NULL) {
        printf("Could not read the path.\n");
        return 1;
    }
    remove_newline(path);

    show_file(path);

    printf("\n1. Add text and keep the current content\n");
    printf("2. Replace all current content\n");
    printf("Choose: ");

    if (fgets(answer, sizeof(answer), stdin) == NULL) {
        printf("Could not read your choice.\n");
        return 1;
    }

    int choice = answer[0] - '0';
    int flags;

    if (choice == 1) {
        flags = O_WRONLY | O_APPEND;
    } else if (choice == 2) {
        printf("This erases the current content. Type YES to continue: ");

        if (fgets(answer, sizeof(answer), stdin) == NULL) {
            printf("Could not read your answer.\n");
            return 1;
        }
        remove_newline(answer);

        if (strcmp(answer, "YES") != 0) {
            printf("Replace cancelled. The file was not changed.\n");
            return 0;
        }

        flags = O_WRONLY | O_TRUNC;
    } else {
        printf("Invalid choice.\n");
        return 1;
    }

    printf("Enter the new text on one line: ");
    if (fgets(text, sizeof(text), stdin) == NULL) {
        printf("Could not read the text.\n");
        return 1;
    }

    int fd = open(path, flags);
    if (fd == -1) {
        perror("Could not open file for writing");
        return 1;
    }

    size_t length = strlen(text);
    size_t written = 0;

    while (written < length) {
        ssize_t result = write(fd, text + written, length - written);

        if (result == -1) {
            perror("Could not write to file");
            close(fd);
            return 1;
        }

        written += (size_t)result;
    }

    close(fd);

    printf("\nText saved. Here is the updated file:");
    show_file(path);

    return 0;
}
