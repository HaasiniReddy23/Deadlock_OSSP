
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define PATH_SIZE 1024

void display_header()
{
    printf("\n");
    printf("================================================\n");
    printf("          LINUX FILE MANAGEMENT SYSTEM\n");
    printf("                   MODULE 1\n");
    printf("                OPEN AND CLOSE\n");
    printf("================================================\n");
}

int main()
{
    char path[PATH_SIZE];
    int file_descriptor;

    display_header();

    printf("\nEnter the full path of an existing file: ");
    fflush(stdout);

    /* Read file path */
    if (fgets(path, sizeof(path), stdin) == NULL)
    {
        printf("\n[ERROR] Unable to read file path.\n");
        return 1;
    }

    /* Check if input contains a newline */
    size_t len = strlen(path);

    if (len > 0 && path[len - 1] == '\n')
    {
        path[--len] = '\0';
    }
    else if (!feof(stdin))
    {
        int ch;

        while ((ch = getchar()) != '\n' && ch != EOF);

        printf("\n[ERROR] File path is too long.\n");
        return 1;
    }

    /* Check empty input */
    if (len == 0)
    {
        printf("\n[ERROR] File path cannot be empty.\n");
        return 1;
    }

    printf("\n------------------------------------------------\n");
    printf("                 OPENING FILE\n");
    printf("------------------------------------------------\n");

    /* Open existing file in read-only mode */
    file_descriptor = open(path, O_RDONLY | O_CLOEXEC);

    if (file_descriptor == -1)
    {
        printf("\n[FAILED] Unable to open the file.\n");
        perror("Reason");
        return 1;
    }

    /* Display file details */
    printf("\n[SUCCESS] File opened successfully!\n");

    printf("\nFile Details\n");
    printf("------------------------------------------------\n");
    printf("File Path       : %s\n", path);
    printf("File Descriptor : %d\n", file_descriptor);
    printf("Opening Mode    : Read Only\n");
    printf("File Status     : OPEN\n");
    printf("------------------------------------------------\n");

    /* Close the file */
    printf("\nClosing file...\n");

    if (close(file_descriptor) == -1)
    {
        printf("\n[ERROR] Unable to close the file.\n");
        perror("Reason");
        return 1;
    }

    printf("\n[SUCCESS] File closed successfully!\n");
    printf("File Status     : CLOSED\n");
    printf("Descriptor has been released.\n");

    printf("\n================================================\n");
    printf("          MODULE 1 EXECUTION COMPLETED\n");
    printf("================================================\n\n");

    return 0;
}

