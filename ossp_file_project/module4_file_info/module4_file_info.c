
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#define PATH_SIZE 1024

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

/* Display heading */
void display_header()
{
    printf("\n");
    printf("================================================\n");
    printf("          LINUX FILE MANAGEMENT SYSTEM\n");
    printf("                   MODULE 4\n");
    printf("                 FILE INFORMATION\n");
    printf("================================================\n");
}

/* Convert file type into readable text */
const char *get_file_type(mode_t mode)
{
    if (S_ISREG(mode))
        return "Regular File";

    if (S_ISDIR(mode))
        return "Directory";

    if (S_ISLNK(mode))
        return "Symbolic Link";

    if (S_ISCHR(mode))
        return "Character Device";

    if (S_ISBLK(mode))
        return "Block Device";

    if (S_ISFIFO(mode))
        return "FIFO / Pipe";

    if (S_ISSOCK(mode))
        return "Socket";

    return "Unknown";
}

/* Display permissions in rwx format */
void display_permissions(mode_t mode)
{
    char permissions[10];

    permissions[0] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[1] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[2] = (mode & S_IXUSR) ? 'x' : '-';

    permissions[3] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[4] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[5] = (mode & S_IXGRP) ? 'x' : '-';

    permissions[6] = (mode & S_IROTH) ? 'r' : '-';
    permissions[7] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[8] = (mode & S_IXOTH) ? 'x' : '-';

    permissions[9] = '\0';

    printf("Permissions (rwx): %s\n", permissions);
    printf("Permissions (octal): %04o\n", mode & 07777);
}

/* Display formatted date and time */
void display_time(const char *label, time_t value)
{
    struct tm time_info;
    char formatted_time[100];

    if (localtime_r(&value, &time_info) == NULL)
    {
        printf("%s: Unavailable\n", label);
        return;
    }

    if (strftime(formatted_time, sizeof(formatted_time),
                 "%d-%m-%Y %H:%M:%S", &time_info) == 0)
    {
        printf("%s: Unavailable\n", label);
        return;
    }

    printf("%-24s: %s\n", label, formatted_time);
}

int main()
{
    char path[PATH_SIZE];

    int file_descriptor;
    struct stat file_info;

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

    /* Open existing file in read-only mode */
    file_descriptor = open(path, O_RDONLY | O_CLOEXEC);

    if (file_descriptor == -1)
    {
        perror("\n[ERROR] Unable to open file");
        return 1;
    }

    printf("\n[SUCCESS] File opened successfully!\n");

    /* Get file information */
    if (fstat(file_descriptor, &file_info) == -1)
    {
        perror("\n[ERROR] Unable to retrieve file information");

        if (close(file_descriptor) == -1)
            perror("[ERROR] Unable to close file");

        return 1;
    }

    /* Display file information */
    printf("\n================================================\n");
    printf("              FILE INFORMATION\n");
    printf("================================================\n");

    printf("\nFile Details\n");
    printf("------------------------------------------------\n");

    printf("File Path             : %s\n", path);
    printf("File Descriptor       : %d\n", file_descriptor);
    printf("File Name             : ");

    const char *name = strrchr(path, '/');

    if (name != NULL)
        printf("%s\n", name + 1);
    else
        printf("%s\n", path);

    printf("Inode Number          : %llu\n",
           (unsigned long long)file_info.st_ino);

    printf("File Size             : %lld bytes\n",
           (long long)file_info.st_size);

    printf("Owner ID (UID)        : %u\n",
           (unsigned int)file_info.st_uid);

    printf("Group ID (GID)        : %u\n",
           (unsigned int)file_info.st_gid);

    printf("Hard Link Count       : %lu\n",
           (unsigned long)file_info.st_nlink);

    printf("File Type             : %s\n",
           get_file_type(file_info.st_mode));

    printf("\nFile Permissions\n");
    printf("------------------------------------------------\n");

    display_permissions(file_info.st_mode);

    printf("\nFile Timestamps\n");
    printf("------------------------------------------------\n");

    display_time("Last Accessed", file_info.st_atime);
    display_time("Last Modified", file_info.st_mtime);
    display_time("Last Status Changed", file_info.st_ctime);

    printf("\n================================================\n");

    /* Close file descriptor */
    printf("\nClosing file...\n");

    if (close(file_descriptor) == -1)
    {
        perror("[ERROR] Unable to close file");
        return 1;
    }

    printf("[SUCCESS] File closed successfully!\n");

    printf("\n================================================\n");
    printf("          MODULE 4 EXECUTION COMPLETED\n");
    printf("================================================\n\n");

    return 0;
}

