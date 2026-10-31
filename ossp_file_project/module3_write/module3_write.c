
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define PATH_SIZE 1024

/* Read a complete line */
char *get_input(const char *message)
{
    char *line = NULL;
    size_t size = 0;

    printf("%s", message);
    fflush(stdout);

    if (getline(&line, &size, stdin) == -1)
    {
        free(line);
        return NULL;
    }

    line[strcspn(line, "\n")] = '\0';
    return line;
}

/* Display program heading */
void display_header()
{
    printf("\n");
    printf("================================================\n");
    printf("          LINUX FILE MANAGEMENT SYSTEM\n");
    printf("                   MODULE 3\n");
    printf("              DISPLAY AND EDIT TEXT\n");
    printf("================================================\n");
}

/* Display existing file contents */
int display_file(const char *path)
{
    char buffer[1024];

    int fd = open(path, O_RDONLY | O_CLOEXEC);

    if (fd == -1)
    {
        perror("[ERROR] Unable to open file for reading");
        return -1;
    }

    printf("\n------------------------------------------------\n");
    printf("                CURRENT FILE CONTENT\n");
    printf("------------------------------------------------\n\n");

    int found = 0;
    int failed = 0;

    while (1)
    {
        ssize_t bytes = read(fd, buffer, sizeof(buffer));

        if (bytes > 0)
        {
            found = 1;

            size_t total = 0;

            while (total < (size_t)bytes)
            {
                ssize_t written = write(STDOUT_FILENO,
                                        buffer + total,
                                        (size_t)bytes - total);

                if (written == -1)
                {
                    if (errno == EINTR)
                        continue;

                    perror("[ERROR] Display failed");
                    failed = 1;
                    break;
                }

                if (written == 0)
                {
                    failed = 1;
                    break;
                }

                total += (size_t)written;
            }

            if (failed)
                break;
        }
        else if (bytes == 0)
        {
            break;
        }
        else if (errno == EINTR)
        {
            continue;
        }
        else
        {
            perror("[ERROR] Reading failed");
            failed = 1;
            break;
        }
    }

    if (!found && !failed)
        printf("[INFO] File is empty.\n");

    if (close(fd) == -1)
    {
        perror("[ERROR] Closing file failed");
        failed = 1;
    }

    printf("\n\n------------------------------------------------\n");

    return failed ? -1 : 0;
}

/* Collect multiple lines of text */
char *collect_text()
{
    char *text = NULL;
    size_t length = 0;

    char *line;

    printf("\nEnter your text below.\n");
    printf("Type a single dot (.) on a new line to finish.\n");
    printf("------------------------------------------------\n");

    while (1)
    {
        line = get_input("");

        if (line == NULL)
        {
            free(text);
            return NULL;
        }

        if (strcmp(line, ".") == 0)
        {
            free(line);
            break;
        }

        size_t line_length = strlen(line);

        if (line_length > (size_t)-1 - length - 2)
        {
            printf("[ERROR] Text is too large.\n");
            free(line);
            free(text);
            return NULL;
        }

        char *temp = realloc(text, length + line_length + 2);

        if (temp == NULL)
        {
            printf("[ERROR] Memory allocation failed.\n");
            free(line);
            free(text);
            return NULL;
        }

        text = temp;

        memcpy(text + length, line, line_length);
        length += line_length;

        text[length++] = '\n';
        text[length] = '\0';

        free(line);
    }

    if (text == NULL)
    {
        printf("[ERROR] No text was entered.\n");
        return NULL;
    }

    return text;
}

/* Write all collected text */
int write_all(int fd, const char *text, size_t length)
{
    size_t written_total = 0;

    while (written_total < length)
    {
        ssize_t result = write(fd,
                               text + written_total,
                               length - written_total);

        if (result == -1)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (result == 0)
            return -1;

        written_total += (size_t)result;
    }

    return 0;
}

int main()
{
    char path[PATH_SIZE];
    char *input;
    char *text;

    int choice;
    int file_descriptor;
    int flags;

    display_header();

    /* Get existing file path */
    input = get_input("\nEnter the full path of an existing text file: ");

    if (input == NULL || strlen(input) == 0)
    {
        printf("\n[ERROR] Invalid file path.\n");
        free(input);
        return 1;
    }

    if (strlen(input) >= sizeof(path))
    {
        printf("\n[ERROR] File path is too long.\n");
        free(input);
        return 1;
    }

    strcpy(path, input);
    free(input);

    /* Display original contents */
    if (display_file(path) == -1)
        return 1;

    /* Display editing options */
    printf("\n                EDITING OPTIONS\n");
    printf("------------------------------------------------\n");
    printf("1. Append new text (keep old content)\n");
    printf("2. Replace all existing content\n");
    printf("3. Exit without making changes\n");
    printf("------------------------------------------------\n");

    input = get_input("Enter your choice (1-3): ");

    if (input == NULL)
        return 1;

    char *end;
    errno = 0;
    long selected = strtol(input, &end, 10);

    while (*end == ' ' || *end == '\t')
        end++;

    if (errno == ERANGE || end == input ||
        *end != '\0' || selected < 1 || selected > 3)
    {
        printf("\n[ERROR] Invalid choice.\n");
        free(input);
        return 1;
    }

    choice = (int)selected;
    free(input);

    if (choice == 3)
    {
        printf("\n[INFO] Editing cancelled. File unchanged.\n");
        return 0;
    }

    /* Collect text before opening file for writing */
    text = collect_text();

    if (text == NULL)
        return 1;

    /* Select editing mode */
    if (choice == 1)
    {
        flags = O_WRONLY | O_APPEND | O_CLOEXEC;
        printf("\n[INFO] Append mode selected.\n");
    }
    else
    {
        input = get_input(
            "\nThis will erase the existing content.\n"
            "Type YES to confirm replacement: ");

        if (input == NULL)
        {
            free(text);
            return 1;
        }

        if (strcmp(input, "YES") != 0)
        {
            printf("\n[INFO] Replacement cancelled.\n");
            printf("[INFO] Original file remains unchanged.\n");

            free(input);
            free(text);
            return 0;
        }

        free(input);

        flags = O_WRONLY | O_TRUNC | O_CLOEXEC;
        printf("\n[INFO] Replace mode selected.\n");
    }

    /* Open file for selected operation */
    file_descriptor = open(path, flags);

    if (file_descriptor == -1)
    {
        perror("\n[ERROR] Unable to open file for editing");
        free(text);
        return 1;
    }

    printf("\n[SUCCESS] File opened for editing.\n");

    /* Write new content */
    if (write_all(file_descriptor, text, strlen(text)) == -1)
    {
        perror("\n[ERROR] Writing failed");

        free(text);

        if (close(file_descriptor) == -1)
            perror("[ERROR] Closing file failed");

        return 1;
    }

    free(text);

    /* Close file */
    if (close(file_descriptor) == -1)
    {
        perror("[ERROR] Closing file failed");
        return 1;
    }

    printf("\n[SUCCESS] File updated successfully!\n");

    /* Display updated contents */
    printf("\n              UPDATED FILE CONTENT\n");

    if (display_file(path) == -1)
        return 1;

    printf("\n================================================\n");
    printf("          MODULE 3 EXECUTION COMPLETED\n");
    printf("================================================\n\n");

    return 0;
}

