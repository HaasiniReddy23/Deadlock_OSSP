
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>

/* Maximum input path and search word sizes */
#define PATH_SIZE 1024
#define WORD_SIZE 100
#define BUFFER_SIZE 1024

/* Read a line safely */
int read_line(char *buffer, size_t size)
{
    if (fgets(buffer, size, stdin) == NULL)
        return 0;

    size_t len = strlen(buffer);

    if (len > 0 && buffer[len - 1] == '\n')
    {
        buffer[len - 1] = '\0';
        return 1;
    }

    int ch = getchar();

    if (ch == '\n' || ch == EOF)
        return 1;

    while (ch != '\n' && ch != EOF)
        ch = getchar();

    return -1;
}

/* Display program heading */
void display_header()
{
    printf("\n");
    printf("================================================\n");
    printf("          LINUX FILE MANAGEMENT SYSTEM\n");
    printf("                   MODULE 2\n");
    printf("                    READ FILE\n");
    printf("================================================\n");
}

/* Write all bytes to terminal */
int write_output(const char *buffer, size_t length)
{
    size_t total = 0;

    while (total < length)
    {
        ssize_t written = write(STDOUT_FILENO,
                                buffer + total,
                                length - total);

        if (written == -1)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (written == 0)
            return -1;

        total += (size_t)written;
    }

    return 0;
}

/* Check whether a character belongs to a word */
int is_word_character(unsigned char ch)
{
    return isalnum(ch) || ch == '_';
}

/* Compare two words without case sensitivity */
int words_match(const char *file_word, const char *search_word,
                size_t word_length)
{
    if (strlen(search_word) != word_length)
        return 0;

    for (size_t i = 0; i < word_length; i++)
    {
        if (tolower((unsigned char)file_word[i]) !=
            tolower((unsigned char)search_word[i]))
        {
            return 0;
        }
    }

    return 1;
}

/* Find the first exact matching word */
ssize_t find_word(const char *content, size_t length,
                  const char *search_word)
{
    size_t word_length = strlen(search_word);

    for (size_t i = 0; i + word_length <= length; i++)
    {
        if (i > 0 && is_word_character((unsigned char)content[i - 1]))
            continue;

        if (!words_match(content + i, search_word, word_length))
            continue;

        if (i + word_length < length &&
            is_word_character((unsigned char)content[i + word_length]))
        {
            continue;
        }

        return (ssize_t)i;
    }

    return -1;
}

int main()
{
    char path[PATH_SIZE];
    char search_word[WORD_SIZE];
    char choice_input[20];

    char *content = NULL;
    size_t content_size = 0;
    size_t capacity = 0;

    char buffer[BUFFER_SIZE];

    int file_descriptor;
    int choice;
    int status = 1;

    display_header();

    /* Get existing file path */
    printf("\nEnter the full path of an existing file: ");
    fflush(stdout);

    int result = read_line(path, sizeof(path));

    if (result != 1 || strlen(path) == 0)
    {
        printf("\n[ERROR] Invalid file path input.\n");
        return 1;
    }

    /* Open file in read-only mode */
    file_descriptor = open(path, O_RDONLY | O_CLOEXEC);

    if (file_descriptor == -1)
    {
        printf("\n[FAILED] Unable to open file.\n");
        perror("Reason");
        return 1;
    }

    printf("\n[SUCCESS] File opened successfully!\n");
    printf("File Path       : %s\n", path);
    printf("File Descriptor : %d\n", file_descriptor);

    /* Display menu */
    printf("\n------------------------------------------------\n");
    printf("                READING OPTIONS\n");
    printf("------------------------------------------------\n");
    printf("1. Read the entire file\n");
    printf("2. Read from a specific word\n");
    printf("------------------------------------------------\n");

    printf("Enter your choice (1 or 2): ");
    fflush(stdout);

    result = read_line(choice_input, sizeof(choice_input));

    if (result != 1)
    {
        printf("\n[ERROR] Invalid menu input.\n");
        goto cleanup;
    }

    char *end;
    errno = 0;
    long selected = strtol(choice_input, &end, 10);

    while (isspace((unsigned char)*end))
        end++;

    if (errno == ERANGE || end == choice_input || *end != '\0' ||
        (selected != 1 && selected != 2))
    {
        printf("\n[ERROR] Please enter only 1 or 2.\n");
        goto cleanup;
    }

    choice = (int)selected;

    /* Option 2: Get word to search */
    if (choice == 2)
    {
        printf("\nEnter the word to search: ");
        fflush(stdout);

        result = read_line(search_word, sizeof(search_word));

        if (result != 1 || strlen(search_word) == 0)
        {
            printf("\n[ERROR] Invalid search word.\n");
            goto cleanup;
        }

        /* Allow only one word */
        for (size_t i = 0; search_word[i] != '\0'; i++)
        {
            if (isspace((unsigned char)search_word[i]))
            {
                printf("\n[ERROR] Please enter only one word.\n");
                goto cleanup;
            }
        }
    }

    /*
       Read the file into memory to search for the word.
       The file is opened read-only, so its contents are unchanged.
    */
    while (1)
    {
        if (capacity - content_size < BUFFER_SIZE)
        {
            size_t new_capacity = capacity + BUFFER_SIZE;

            char *temp = realloc(content, new_capacity);

            if (temp == NULL)
            {
                printf("\n[ERROR] Not enough memory.\n");
                goto cleanup;
            }

            content = temp;
            capacity = new_capacity;
        }

        ssize_t bytes_read = read(file_descriptor,
                                  buffer,
                                  sizeof(buffer));

        if (bytes_read > 0)
        {
            if ((size_t)bytes_read > (size_t)-1 - content_size)
            {
                printf("\n[ERROR] File is too large to process.\n");
                goto cleanup;
            }

            if (content_size + (size_t)bytes_read > capacity)
            {
                size_t new_capacity = content_size + (size_t)bytes_read;
                char *temp = realloc(content, new_capacity);

                if (temp == NULL)
                {
                    printf("\n[ERROR] Not enough memory.\n");
                    goto cleanup;
                }

                content = temp;
                capacity = new_capacity;
            }

            memcpy(content + content_size, buffer, (size_t)bytes_read);
            content_size += (size_t)bytes_read;
        }
        else if (bytes_read == 0)
        {
            break;
        }
        else if (errno == EINTR)
        {
            continue;
        }
        else
        {
            perror("\n[ERROR] Unable to read file");
            goto cleanup;
        }
    }

    /* Option 2: Find matching word */
    if (choice == 2)
    {
        ssize_t position = find_word(content, content_size, search_word);

        if (position == -1)
        {
            printf("\n[NOT FOUND] The word '%s' does not exist in the file.\n",
                   search_word);
            goto cleanup;
        }

        /* Move descriptor to the matching word */
        if (lseek(file_descriptor, (off_t)position, SEEK_SET) == -1)
        {
            perror("\n[ERROR] Unable to move to the word position");
            goto cleanup;
        }

        printf("\n[SUCCESS] Word '%s' found!\n", search_word);
        printf("Starting position: %zd\n", position);
        printf("Reading from the matching word...\n");
    }
    else
    {
        /* Option 1: Return to beginning */
        if (lseek(file_descriptor, 0, SEEK_SET) == -1)
        {
            perror("\n[ERROR] Unable to return to file beginning");
            goto cleanup;
        }

        printf("\n[SUCCESS] Reading the entire file...\n");
    }

    /* Display selected file contents */
    printf("\n================================================\n");
    printf("                  FILE CONTENTS\n");
    printf("================================================\n\n");

    int read_error = 0;
    int data_found = 0;

    while (1)
    {
        ssize_t bytes_read = read(file_descriptor,
                                  buffer,
                                  sizeof(buffer));

        if (bytes_read > 0)
        {
            data_found = 1;

            if (write_output(buffer, (size_t)bytes_read) == -1)
            {
                perror("\n[ERROR] Unable to display contents");
                read_error = 1;
                break;
            }
        }
        else if (bytes_read == 0)
        {
            break;
        }
        else if (errno == EINTR)
        {
            continue;
        }
        else
        {
            perror("\n[ERROR] Reading failed");
            read_error = 1;
            break;
        }
    }

    if (!data_found && !read_error)
        printf("[INFO] No content available.\n");

    printf("\n\n================================================\n");
    printf("                   END OF FILE\n");
    printf("================================================\n");

    status = read_error ? 1 : 0;

cleanup:

    /* Release allocated memory */
    free(content);

    /* Close file descriptor */
    printf("\nClosing file...\n");

    if (close(file_descriptor) == -1)
    {
        perror("[ERROR] Unable to close file");
        return 1;
    }

    printf("[SUCCESS] File closed successfully!\n");
    printf("\n          MODULE 2 EXECUTION COMPLETED\n\n");

    return status;
}

