#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    char path[1024];
    char input[32];
    char text[1024];
    int choice;

    printf("Enter the path of an existing file: ");
    if (fgets(path, sizeof(path), stdin) == NULL) {
        printf("Could not read the path.\n");
        return 1;
    }
    path[strcspn(path, "\n")] = '\0';

    printf("\n1. Add content and keep the old content\n");
    printf("2. Replace all content\n");
    printf("Choose: ");

    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Could not read your choice.\n");
        return 1;
    }
    choice = input[0] - '0';

    int flags;

    if (choice == 1) {
        flags = O_WRONLY | O_APPEND;
    } else if (choice == 2) {
        char confirm[16];

        printf("This erases the old content. Type YES to continue: ");
        if (fgets(confirm, sizeof(confirm), stdin) == NULL) {
            printf("Could not read your answer.\n");
            return 1;
        }
        confirm[strcspn(confirm, "\n")] = '\0';

        if (strcmp(confirm, "YES") != 0) {
            printf("Replace cancelled. The file was not changed.\n");
            return 0;
        }

        flags = O_WRONLY | O_TRUNC;
    } else {
        printf("Invalid choice.\n");
        return 1;
    }

    int file_descriptor = open(path, flags);
    if (file_descriptor == -1) {
        perror("Could not open file");
        return 1;
    }

    printf("Enter the new text on one line: ");
    if (fgets(text, sizeof(text), stdin) == NULL) {
        printf("Could not read the text.\n");
        close(file_descriptor);
        return 1;
    }

    size_t length = strlen(text);
    size_t written = 0;

    while (written < length) {
        ssize_t result = write(file_descriptor, text + written, length - written);

        if (result == -1) {
            perror("Could not write to file");
            close(file_descriptor);
            return 1;
        }

        written += (size_t)result;
    }

    close(file_descriptor);
    printf("Content saved to: %s\n", path);
    return 0;
}
