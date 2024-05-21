#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

// Deklarasi fungsi decoding
void decode_base64(char *output, const char *input);
void decode_rot13(char *output, const char *input);
void decode_hex(char *output, const char *input);
void decode_rev(char *output, const char *input);
void log_action(const char *status, const char *tag, const char *info);

static const char *base_path = "/path/to/real/folder"; // Ganti dengan path asli

static int myfs_getattr(const char *path, struct stat *stbuf) {
    int res;
    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", base_path, path);

    res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int myfs_open(const char *path, struct fuse_file_info *fi) {
    int res;
    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", base_path, path);

    res = open(fpath, fi->flags);
    if (res == -1)
        return -errno;

    close(res);
    return 0;
}

static int myfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd;
    int res;
    char fpath[1024];
    char content[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", base_path, path);

    fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;

    res = pread(fd, content, sizeof(content) - 1, offset);
    if (res == -1) {
        res = -errno;
    } else {
        content[res] = '\0';
        if (strncmp(path, "/pesan/base64_", 14) == 0) {
            decode_base64(buf, content);
        } else if (strncmp(path, "/pesan/rot13_", 13) == 0) {
            decode_rot13(buf, content);
        } else if (strncmp(path, "/pesan/hex_", 11) == 0) {
            decode_hex(buf, content);
        } else if (strncmp(path, "/pesan/rev_", 11) == 0) {
            decode_rev(buf, content);
        } else {
            memcpy(buf, content, res);
        }
        res = strlen(buf);
    }

    close(fd);
    return res;
}

static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;
    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", base_path, path);

    dp = opendir(fpath);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
    }

    closedir(dp);
    return 0;
}

void decode_base64(char *output, const char *input) {
    // Implementasi decoding Base64
    // Misalnya menggunakan library base64
    // snprintf(output, strlen(input), "decoded base64: %s", input);
}

void decode_rot13(char *output, const char *input) {
    for (int i = 0; input[i]; i++) {
        if ((input[i] >= 'A' && input[i] <= 'Z') || (input[i] >= 'a' && input[i] <= 'z')) {
            if ((input[i] & 32) == 0)
                output[i] = ((input[i] - 'A' + 13) % 26) + 'A';
            else
                output[i] = ((input[i] - 'a' + 13) % 26) + 'a';
        } else {
            output[i] = input[i];
        }
    }
    output[strlen(input)] = '\0';
}

void decode_hex(char *output, const char *input) {
    int len = strlen(input);
    int j = 0;
    for (int i = 0; i < len; i += 2) {
        char hex[3] = {input[i], input[i + 1], '\0'};
        output[j++] = strtol(hex, NULL, 16);
    }
        output[j] = '\0';
}

void decode_rev(char *output, const char *input) {
    int len = strlen(input);
    for (int i = 0; i < len; i++) {
        output[i] = input[len - i - 1];
    }
    output[len] = '\0';
}

void log_action(const char *status, const char *tag, const char *info) {
    FILE *log_file = fopen("/path/to/logs-fuse.log", "a");
    if (log_file != NULL) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        fprintf(log_file, "[%s]::%02d/%02d/%04d-%02d:%02d:%02d::[%s]::[%s]\n",
                status, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
                tm.tm_hour, tm.tm_min, tm.tm_sec, tag, info);
        fclose(log_file);
    }
}

static struct fuse_operations myfs_oper = {
    .getattr = myfs_getattr,
    .open = myfs_open,
    .read = myfs_read,
    .readdir = myfs_readdir,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &myfs_oper, NULL);
}

