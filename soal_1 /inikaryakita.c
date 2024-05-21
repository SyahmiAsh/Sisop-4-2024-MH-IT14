#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h> // Include syslog for logging

static const char *dirpath = "/home/ash23/soalsisop";
static const char *bahaya_path = "/home/ash23/soalsisop/bahaya/script.sh";

// Function to add watermark to an image using ImageMagick
int add_watermark(const char *filepath) {
    char command[1024];
    snprintf(command, sizeof(command), "convert %s -gravity SouthEast -pointsize 36 -draw \"text 10,10 'inikaryakita.id'\" %s", filepath, filepath);
    syslog(LOG_INFO, "Executing command: %s", command); // Log the command to be executed
    int result = system(command);
    if (result != 0) {
        syslog(LOG_ERR, "Command execution failed with result: %d", result); // Log if the command fails
    }
    return result;
}

// Reverse the content of the file
void reverse_content(const char *filepath) {
    FILE *file = fopen(filepath, "r+");
    if (file == NULL) return;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(size + 1);
    if (content == NULL) {
        fclose(file);
        return;
    }

    fread(content, 1, size, file);
    for (long i = 0; i < size / 2; ++i) {
        char tmp = content[i];
        content[i] = content[size - 1 - i];
        content[size - 1 - i] = tmp;
    }

    fseek(file, 0, SEEK_SET);
    fwrite(content, 1, size, file);
    fclose(file);
    free(content);
}

static int hello_getattr(const char *path, struct stat *stbuf)
{
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int res = lstat(fpath, stbuf);
    if (res == -1) return -errno;
    return 0;
}

static int hello_mkdir(const char *path, mode_t mode)
{
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int res = mkdir(fpath, mode);
    if (res == -1) return -errno;
    return 0;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);

    DIR *dp = opendir(fpath);
    if (dp == NULL) return -errno;

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0)) break;
    }
    closedir(dp);
    return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);

    int res = open(fpath, fi->flags);
    if (res == -1) return -errno;
    close(res);
    return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);

    int fd = open(fpath, O_RDONLY);
    if (fd == -1) return -errno;

    int res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;
    close(fd);
    return res;
}

static int hello_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    
    // Handling files with prefix 'test'
    if (strncmp(path, "/test", 5) == 0) {
        reverse_content(fpath);
    }

    int fd = open(fpath, O_WRONLY);
    if (fd == -1) return -errno;

    int res = pwrite(fd, buf, size, offset);
    if (res == -1) res = -errno;
    close(fd);
    return res;
}

static int hello_rename(const char *from, const char *to)
{
    char ffrom[1000], fto[1000];
    sprintf(ffrom, "%s%s", dirpath, from);
    sprintf(fto, "%s%s", dirpath, to);

    syslog(LOG_INFO, "Renaming from %s to %s", ffrom, fto); // Log rename paths

    int res = rename(ffrom, fto);
    if (res == -1) {
        syslog(LOG_ERR, "Rename failed: %s", strerror(errno)); // Log error if rename fails
        return -errno;
    }

    // Add watermark if the file is moved to a directory with prefix 'wm-foto'
    if (strstr(to, "/wm-foto/") != NULL) {
        syslog(LOG_INFO, "Adding watermark to %s", fto); // Log before adding watermark
        int wm_res = add_watermark(fto);
        if (wm_res != 0) {
            syslog(LOG_ERR, "Failed to add watermark: %s", strerror(errno)); // Log error if adding watermark fails
        }
    }
    
    return 0;
}

static struct fuse_operations hello_oper = {
    .getattr    = hello_getattr,
    .mkdir      = hello_mkdir,
    .readdir    = hello_readdir,
    .open       = hello_open,
    .read       = hello_read,
    .write      = hello_write,
    .rename     = hello_rename,
};

int main(int argc, char *argv[])
{
    openlog("FUSE_FS", LOG_PID | LOG_CONS, LOG_USER); // Open syslog
    umask(0);

    // Change permission of script.sh to make it executable
    chmod(bahaya_path, 0755);

    int fuse_stat = fuse_main(argc, argv, &hello_oper, NULL);
    closelog(); // Close syslog
    return fuse_stat;
}
