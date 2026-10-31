
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define PATH_SIZE 1024

/* Display heading */
void display_header()
{
    printf("\n");
    printf("================================================\n");
    printf("          LINUX FILE MANAGEMENT SYSTEM\n");
    printf("                   MODULE 5\n");
    printf("              CLOSE FILE DESCRIPTOR\n");
    printf("================================================\n");
}

/* Read file path safely */
int read_path(char *path, size_t size)
{
    if (fgets(path, size, stdin) == NULL)
        return 0;

    size_t len = strlen(path);

    if (len > 0 && path[len - 1] == '\n')
    {
        path[len - 1] = '\0';
        return 1;
    }

    int ch = getchar();

    if (ch == '\n' || ch == EOF)
        return 1;

    while (ch != '\n' && ch != EOF)
        ch = getchar();

    return -1;
}

int main()
{
    char path[PATH_SIZE];
    int file_descriptor;

    display_header();

    /* Get existing file path */
    printf("\nEnter the full path of an existing file: ");
    fflush(stdout);

    int result = read_path(path, sizeof(path));

    if (result == 0)
    {
        printf("\n[ERROR] Unable to read file path.\n");
        return 1;
    }

    if (result == -1)
    {
        printf("\n[ERROR] File path is too long.\n");
        return 1;
    }

    if (strlen(path) == 0)
    {
        printf("\n[ERROR] File path cannot be empty.\n");
        return 1;
    }

    /* Open existing file */
    file_descriptor = open(path, O_RDONLY | O_CLOEXEC);

    if (file_descriptor == -1)
    {
        perror("\n[ERROR] Unable to open file");
        return 1;
    }

    printf("\n------------------------------------------------\n");
    printf("                 FILE OPENED\n");
    printf("------------------------------------------------\n");

    printf("[SUCCESS] File opened successfully!\n");
    printf("File Path       : %s\n", path);
    printf("File Descriptor : %d\n", file_descriptor);
    printf("File Status     : OPEN\n");

    /* Close file descriptor */
    printf("\n------------------------------------------------\n");
    printf("                 CLOSING FILE\n");
    printf("------------------------------------------------\n");

    if (close(file_descriptor) == -1)
    {
        perror("[ERROR] Unable to close file");
        return 1;
    }

    printf("[SUCCESS] File closed successfully!\n");
    printf("File Status     : CLOSED\n");

    /* Verify descriptor release */
    printf("\n------------------------------------------------\n");
    printf("            VERIFYING DESCRIPTOR\n");
    printf("------------------------------------------------\n");

    errno = 0;

    if (fcntl(file_descriptor, F_GETFD) == -1)
    {
        if (errno == EBADF)
        {
            printf("[SUCCESS] File descriptor is no longer valid.\n");
            printf("[SUCCESS] Descriptor released by Linux.\n");
        }
        else
        {
            perror("[ERROR] Unable to verify descriptor");
            return 1;
        }
    }
    else
    {
        printf("[ERROR] File descriptor is still valid.\n");

        if (close(file_descriptor) == -1)
            perror("[ERROR] Cleanup failed");

        return 1;
    }

    printf("\n================================================\n");
    printf("          MODULE 5 EXECUTION COMPLETED\n");
    printf("================================================\n\n");

    return 0;
}

