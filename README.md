# Sisop-4-2024-MH-IT14

Muhammad Faqih Husain 5027231023

## Soal 1

## Soal 2

## Soal 3

Seorang arkeolog menemukan sebuah gua yang didalamnya tersimpan banyak relik dari zaman praaksara, sayangnya semua barang yang ada pada gua tersebut memiliki bentuk yang terpecah belah akibat bencana yang tidak diketahui. Sang arkeolog ingin menemukan cara cepat agar ia bisa menggabungkan relik-relik yang terpecah itu, namun karena setiap pecahan relik itu masih memiliki nilai tersendiri, ia memutuskan untuk membuat sebuah file system yang mana saat ia mengakses file system tersebut ia dapat melihat semua relik dalam keadaan utuh, sementara relik yang asli tidak berubah sama sekali.

### Solusi 

```
#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

// Debugging macro
#define DEBUG(fmt, args...) fprintf(stderr, fmt, ##args)

#include <errno.h>
#include <sys/stat.h>

#define FUSE_SUCCESS 0

static const char *root_path = "/home/kali/Sisop/modul4/soal_3/relics";
char fullpath[1000];

#include <fcntl.h>
#include <unistd.h>

// Function prototypes
static void get_full_path(char *dest, const char *path);
static int get_file_size(const char *path, struct stat *stbuf);
static int do_getattr(const char *path, struct stat *stbuf);
static int do_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int do_open(const char *path, struct fuse_file_info *fi);
static int do_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int do_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int do_unlink(const char *path);
static int do_create(const char *path, mode_t mode, struct fuse_file_info *fi);
static int do_truncate(const char *path, off_t size);
static int do_mkdir(const char *path, mode_t mode);
static int do_rmdir(const char *path);
static int do_chmod(const char *path, mode_t mode);
static int do_chown(const char *path, uid_t uid, gid_t gid);

// Fuse operations structure
static struct fuse_operations do_oper = {
    .getattr = do_getattr,
    .open = do_open,
    .write = do_write,
    .truncate = do_truncate,
    .create = do_create,
    .mkdir = do_mkdir,
    .unlink = do_unlink,
    .rmdir = do_rmdir,
    .read = do_read,
    .chmod = do_chmod,
    .readdir = do_readdir,
    .chown = do_chown,
};

int main(int argc, char *argv[]) {
    umask(0);
    DEBUG("Starting FUSE filesystem\n");
    return fuse_main(argc, argv, &do_oper, NULL);
}

// Function definitions
static void get_full_path(char *dest, const char *path) {
    snprintf(dest, 1000, "%s%s", root_path, path);
    DEBUG("get_full_path: %s -> %s\n", path, dest);
}

static int get_file_size(const char *path, struct stat *stbuf) {
    char part_path[1100];
    int i = 0;
    int fd;
    stbuf->st_size = 0;

    DEBUG("get_file_size: Calculating size for %s\n", path);

    while (1) {
        snprintf(part_path, sizeof(part_path), "%s.%03d", path, i++);
        fd = open(part_path, O_RDONLY);
        if (fd == -1) {
            if (errno == ENOENT) break;
            DEBUG("get_file_size: Error opening %s: %s\n", part_path, strerror(errno));
            return -errno;
        }
        off_t file_size = lseek(fd, 0, SEEK_END);
        if (file_size == -1) {
            close(fd);
            DEBUG("get_file_size: Error seeking in %s: %s\n", part_path, strerror(errno));
            return -errno;
        }
        stbuf->st_size += file_size;
        close(fd);
    }

    DEBUG("get_file_size: Total size for %s is %ld\n", path, stbuf->st_size);

    return (i == 1) ? -ENOENT : 0;
}

static int do_getattr(const char *path, struct stat *stbuf) {
    DEBUG("do_getattr: %s\n", path);
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return FUSE_SUCCESS ;
    }

    strcpy(fullpath, "");
    get_full_path(fullpath, path);
    stbuf->st_mode = S_IFREG | 0644;
    stbuf->st_nlink = 1;
    stbuf->st_size = 0;

    return get_file_size(fullpath, stbuf);
}

static int do_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    DEBUG("do_readdir: %s\n", path);
    (void) offset; (void) fi;
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    strcpy(fullpath, "");
    get_full_path(fullpath, path);
    DIR *dp = opendir(fullpath);
    if (!dp) return -errno;

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        if (strstr(de->d_name, ".000")) {
            char base_name[256];
            size_t len = strlen(de->d_name);
            if (len >= 4) {
                strncpy(base_name, de->d_name, len - 4);
                base_name[len - 4] = '\0';
                filler(buf, base_name, NULL, 0);
                DEBUG("do_readdir: Adding %s to directory listing\n", base_name);
            }
        }
    }

    closedir(dp);
    return FUSE_SUCCESS ;
}

static int do_open(const char *path, struct fuse_file_info *fi) {
    DEBUG("do_open: %s\n", path);
    strcpy(fullpath, "");
    snprintf(fullpath, sizeof(fullpath), "%s%s.000", root_path, path);
    int fd = open(fullpath, O_RDONLY);
    if (fd == -1) {
        DEBUG("do_open: Error opening %s: %s\n", fullpath, strerror(errno));
        return -errno;
    }
    close(fd);
    return FUSE_SUCCESS ;
}

static int do_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    DEBUG("do_read: %s, size: %zu, offset: %ld\n", path, size, offset);
    (void) fi;
    strcpy(fullpath, "");
    get_full_path(fullpath, path);
    char part_path[1100];
    size_t read_size = 0;
    off_t part_offset = offset;
    int i = 0;

    while (size > 0) {
        snprintf(part_path, sizeof(part_path), "%s.%03d", fullpath, i++);
        int fd = open(part_path, O_RDONLY);
        if (fd == -1) break;

        off_t part_size = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        if (part_offset >= part_size) {
            part_offset -= part_size;
            close(fd);
            continue;
        }

        lseek(fd, part_offset, SEEK_SET);
        ssize_t len = read(fd, buf, size);
        if (len == -1) {
            close(fd);
            DEBUG("do_read: Error reading from %s: %s\n", part_path, strerror(errno));
            return -errno;
        }

        close(fd);
        buf += len;
        size -= len;
        read_size += len;
        part_offset = 0;
    }

    DEBUG("do_read: Read %zu bytes from %s\n", read_size, path);
    return read_size;
}

static int do_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    DEBUG("do_write: %s, size: %zu, offset: %ld\n", path, size, offset);
    (void) fi;
    strcpy(fullpath, "");
    get_full_path(fullpath, path);
    int part_num = offset / 10000;
    off_t part_offset = offset % 10000;
    size_t written_size = 0;
    char part_path[1100];

    while (size > 0) {
        snprintf(part_path, sizeof(part_path), "%s.%03d", fullpath, part_num++);
        int fd = open(part_path, O_WRONLY | O_CREAT, 0644);
        if (fd == -1) {
            DEBUG("do_write: Error opening %s: %s\n", part_path, strerror(errno));
            return -errno;
        }

        lseek(fd, part_offset, SEEK_SET);
        ssize_t write_size = write(fd, buf, size);
        if (write_size == -1) {
            close(fd);
            DEBUG("do_write: Error writing to %s: %s\n", part_path, strerror(errno));
            return -errno;
        }

        close(fd);
        buf += write_size;
        size -= write_size;
        written_size += write_size;
        part_offset = 0;
    }

    DEBUG("do_write: Written %zu bytes to %s\n", written_size, path);
    return written_size;
}

static int do_unlink(const char *path) {
    DEBUG("do_unlink: %s\n", path);
    strcpy(fullpath, "");
    get_full_path(fullpath, path);
    char part_path[1100];
    int part_num = 0;
    int res;

    while (1) {
        snprintf(part_path, sizeof(part_path), "%s.%03d", fullpath, part_num++);
        res = unlink(part_path);
        if (res == -1) {
            if (errno == ENOENT) break;
            DEBUG("do_unlink: Error unlinking %s: %s\n", part_path, strerror(errno));
            return -errno;
        }
    }

    return FUSE_SUCCESS ;
}

static int do_truncate(const char *path, off_t size) {
    DEBUG("do_truncate: %s, size: %ld\n", path, size);
    strcpy(fullpath, "");
    get_full_path(fullpath, path);
    char part_path[1100];
    off_t remaining_size = size;
    int part_num = 0;
    int res;

    while (remaining_size > 0) {
        snprintf(part_path, sizeof(part_path), "%s.%03d", fullpath, part_num++);
        size_t part_size = (remaining_size > 10000) ? 10000 : remaining_size;
        res = truncate(part_path, part_size);
        if (res == -1) {
            DEBUG("do_truncate: Error truncating %s: %s\n", part_path, strerror(errno));
            return -errno;
        }
        remaining_size -= part_size;
        chmod(part_path, 0644);  // Ensure correct permissions
    }

    while (1) {
        snprintf(part_path, sizeof(part_path), "%s.%03d", fullpath, part_num++);
        res = unlink(part_path);
        if (res == -1 && errno == ENOENT) break;
        if (res == -1) {
            DEBUG("do_truncate: Error unlinking %s: %s\n", part_path, strerror(errno));
            return -errno;
        }
    }

    return FUSE_SUCCESS ;
}

static int do_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    DEBUG("do_create: %s, mode: %o\n", path, mode);
    (void) fi;
    strcpy(fullpath, "");
    snprintf(fullpath, sizeof(fullpath), "%s%s.000", root_path, path);
    int res = open(fullpath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (res == -1) {
        DEBUG("do_create: Error creating %s: %s\n", fullpath, strerror(errno));
        return -errno;
    }
    close(res);
    return FUSE_SUCCESS ;
}

static int do_mkdir(const char *path, mode_t mode) {
    strcpy(fullpath, "");
	DEBUG("do_mkdir: %s, mode: %o\n", path, mode);
    get_full_path(fullpath, path);
    int res = mkdir(fullpath, mode);
    if (res == -1) {
        DEBUG("do_mkdir: Error creating directory %s: %s\n", fullpath, strerror(errno));
        return -errno;
    }
    return FUSE_SUCCESS ;
}

static int do_rmdir(const char *path) {
	strcpy(fullpath, "");
    DEBUG("do_rmdir: %s\n", path);
    get_full_path(fullpath, path);
    int res = rmdir(fullpath);
    if (res == -1) {
        DEBUG("do_rmdir: Error removing directory %s: %s\n", fullpath, strerror(errno));
        return -errno;
    }
    return FUSE_SUCCESS ;
}

static int do_chmod(const char *path, mode_t mode) {
    strcpy(fullpath, "");
	DEBUG("do_chmod: %s, mode: %o\n", path, mode);
    get_full_path(fullpath, path);
    int res = chmod(fullpath, mode);
    if (res == -1) {
        DEBUG("do_chmod: Error changing mode of %s: %s\n", fullpath, strerror(errno));
        return -errno;
    }
    return FUSE_SUCCESS ;
}

static int do_chown(const char *path, uid_t uid, gid_t gid) {
    DEBUG("do_chown: %s, uid: %d, gid: %d\n", path, uid, gid);
    strcpy(fullpath, "");
    get_full_path(fullpath, path);
    int res = chown(fullpath, uid, gid);
    if (res == -1) {
        DEBUG("do_chown: Error changing ownership of %s: %s\n", fullpath, strerror(errno));
        return -errno;
    }
    return FUSE_SUCCESS ;
}
```

Program FUSE di atas menyediakan implementasi sistem file virtual yang melakukan berbagai operasi pada file dan direktori. Berikut adalah deskripsi singkat tentang setiap fungsi yang diimplementasikan dalam program ini dan fungsinya:
- get_full_path: Menggabungkan root_path dengan path yang diberikan untuk mendapatkan jalur absolut lengkap dari file atau direktori.
- get_file_size: Menghitung ukuran total file dengan menggabungkan ukuran dari semua bagian file yang dipecah.
- do_getattr: Mengambil atribut file atau direktori yang ditentukan oleh path. Mengembalikan informasi seperti jenis file, ukuran, dan izin akses.
- do_readdir: Membaca konten dari direktori dan mengisi buffer dengan nama-nama file yang ada. Hanya file dengan ekstensi .000 yang ditampilkan, tanpa menampilkan ekstensi.
- do_open: Membuka file yang ditentukan oleh path untuk membaca. Membuka bagian pertama dari file yang dipecah.
- do_read: Membaca data dari file yang ditentukan oleh path dari offset yang diberikan hingga sejumlah size byte. Membaca dari bagian-bagian file yang dipecah.
- do_write: Menulis data ke file yang ditentukan oleh path dari offset yang diberikan. Data ditulis ke bagian-bagian file yang dipecah dengan ukuran maksimum 10.000 byte per bagian.
- do_unlink: Menghapus file yang ditentukan oleh path. Menghapus semua bagian file yang dipecah.
- do_create: Membuat file baru dengan mode yang ditentukan. Membuat bagian pertama dari file yang dipecah.
- do_truncate: Memotong atau memperpanjang file yang ditentukan oleh path ke ukuran yang diberikan. Menyesuaikan bagian-bagian file yang dipecah agar sesuai dengan ukuran baru.
- do_mkdir: Membuat direktori baru dengan mode yang ditentukan.
- do_rmdir: Menghapus direktori yang ditentukan oleh path.
- do_chmod: Mengubah izin akses dari file atau direktori yang ditentukan oleh path.
- do_chown: Mengubah pemilik dan grup dari file atau direktori yang ditentukan oleh path.

Cara Penggunaan
```
cp twibbon.jpg ./fuze
cp ./fuze/relic_1.png .
cp twibbon.jpg ./fuze
rm twibbon*
cp fuze/* report/  
```

Membagikan direktory `report` menggunakan samba

```
sudo service smbd start
service smbd status
vim /etc/samba/smb.conf
sudo service smbd stop
```

### Revisi 
Tidak ada catatan dari asisten penguji
