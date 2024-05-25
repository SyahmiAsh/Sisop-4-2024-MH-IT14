# Sisop-4-2024-MH-IT14
Laporan pengerjaan soal shift modul 4 Praktikum Sistem Operasi 2024 oleh Kelompok IT14
## Praktikan Sistem Operasi
1. Tsaldia Hukma Cita : 5027231036
2. Muhammad Faqih Husain : 5027231023
3. Muhammad Syahmi Ash Shidqi : 5027231085

## Soal 1
Adfi merupakan seorang CEO agency creative bernama Ini Karya Kita. Ia sedang melakukan inovasi pada manajemen project photography Ini Karya Kita. Salah satu ide yang dia kembangkan adalah tentang pengelolaan foto project dalam sistem arsip Ini Karya Kita. Dalam membangun sistem ini, Adfi tidak bisa melakukannya sendirian, dia perlu bantuan mahasiswa Departemen Teknologi Informasi angkatan 2023 untuk membahas konsep baru yang akan mengubah project fotografinya lebih menarik untuk dilihat. Adfi telah menyiapkan portofolio hasil project fotonya yang bisa didownload dan diakses di www.inikaryakita.id . Silahkan eksplorasi web Ini Karya Kita dan temukan halaman untuk bisa mendownload projectnya. Setelah kalian download terdapat folder gallery dan bahaya.

langkah yang pertama kita harus membuat sebuah direktori, pada direktori tersebut kita mendownload portofolio dari www.inikaryakita.id didalam web itu kita bisa pilih service fotografi dan nnti ada untuk download portofolio sisop apa bila kita sudah dapat link filenya maka kita bisa mendownloadnya `wget -O Resource "https://drive.google.com/uc?export=download&id=1VP6o84AQfY6QbghFGkw6ghxkBbv7fpum"`

bila sudah terdownload kita harus mengunzipnya `unzip Resource` di dalam file portofolio terdapat file gallery dan bahaya

Pada folder “gallery”:
- Membuat folder dengan prefix "wm." Dalam folder ini, setiap gambar yang dipindahkan ke dalamnya akan diberikan watermark bertuliskan inikaryakita.id. 

Pada folder "bahaya," terdapat file bernama "script.sh." 
- mengubah permission pada file "script.sh" agar bisa dijalankan, karena jika dijalankan maka dapat menghapus semua dan isi dari  "gallery".
- membuat file dengan prefix "test" yang ketika disimpan akan mengalami pembalikan (reverse) isi dari file tersebut.

```
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
```
untuk librarynya dan mendefinisikan fuse veri 31

```
static const char *dirpath = "/home/ash23/soalsisop";
static const char *bahaya_path = "/home/ash23/soalsisop/bahaya/script.sh";
```
untuk alokasi letak programnya
untuk alokasi file script.sh

```
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
```
fungsi untuk menambahkan watermark nya
```
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
```
fungsi untuk membuat folder
```
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
```
fungsi untuk menuliskan isi file dan apabila ada file dengan prefix "test" maka akan msuk ke fungsi reverse_content
```
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
```
untuk inti dari programnya
```
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
```

cara menjalankannya 

`gcc -Wall pkg-config fuse --cflags [file.c] -o [output] pkg-config fuse --libs`

`mkdir filemount`

`./output filemount`

hasil

- watermark foto
![WhatsApp Image 2024-05-25 at 23 27 34](https://github.com/SyahmiAsh/Sisop-4-2024-MH-IT14/assets/149950475/9ee9a91f-8ee3-4097-8e88-da3a24526e00)
- Hasil mengubah permission pada file script.sh
![WhatsApp Image 2024-05-25 at 23 27 28](https://github.com/SyahmiAsh/Sisop-4-2024-MH-IT14/assets/149950475/0e923e3c-df40-455b-aa7f-7909b91ad68f)

### kendala
kesulitan dalam membuat isi file dengan prefix "test" menjadi tereverse.

### Revisi
Terdapat revisi untuk reverse file dengan prefix "test" karena masih belum jalan.
 

## Soal 2

Masih dengan Ini Karya Kita, sang CEO ingin melakukan tes keamanan pada folder sensitif Ini Karya Kita. Karena Teknologi Informasi merupakan departemen dengan salah satu fokus di Cyber Security, maka dia kembali meminta bantuan mahasiswa Teknologi Informasi angkatan 2023 untuk menguji dan mengatur keamanan pada folder sensitif tersebut. Untuk mendapatkan folder sensitif itu, mahasiswa IT 23 harus kembali mengunjungi website Ini Karya Kita pada www.inikaryakita.id/schedule . Silahkan isi semua formnya, tapi pada form subject isi dengan nama kelompok_SISOP24 , ex: IT01_SISOP24 . Lalu untuk form Masukkan Pesanmu, ketik “Mau Foldernya” . Tunggu hingga 1x24 jam, maka folder sensitif tersebut akan dikirimkan melalui email kalian. Apabila folder tidak dikirimkan ke email kalian, maka hubungi sang CEO untuk meminta bantuan.   
- Pada folder "pesan" Adfi ingin meningkatkan kemampuan sistemnya dalam mengelola berkas-berkas teks dengan menggunakan fuse.
- Jika sebuah file memiliki prefix "base64," maka sistem akan langsung mendekode isi file tersebut dengan algoritma Base64.
- Jika sebuah file memiliki prefix "rot13," maka isi file tersebut akan langsung di-decode dengan algoritma ROT13.
- Jika sebuah file memiliki prefix "hex," maka isi file tersebut akan langsung di-decode dari representasi heksadesimalnya.
- Jika sebuah file memiliki prefix "rev," maka isi file tersebut akan langsung di-decode dengan cara membalikkan teksnya.
- Pada folder “rahasia-berkas”, Adfi dan timnya memutuskan untuk menerapkan kebijakan khusus. Mereka ingin memastikan bahwa folder dengan prefix "rahasia" tidak dapat diakses tanpa izin khusus. 
- Jika seseorang ingin mengakses folder dan file pada “rahasia”, mereka harus memasukkan sebuah password terlebih dahulu (password bebas). 
- Setiap proses yang dilakukan akan tercatat pada logs-fuse.log dengan format :
[SUCCESS/FAILED]::dd/mm/yyyy-hh:mm:ss::[tag]::[information]
Ex:
[SUCCESS]::01/11/2023-10:43:43::[moveFile]::[File moved successfully]

### Penyelesaian
```
#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

static const char *source_dir = "/home/tsll/gpp/sensitif";
static const char *log_file_path = "/home/tsll/pst/log.txt";
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void log_event(const char *tag, const char *information, int success) {
    time_t now;
    struct tm *local_time;
    char timestamp[20];

    time(&now);
    local_time = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%d/%m/%Y-%H:%M:%S", local_time);

    FILE *log_file = fopen(log_file_path, "a");
    if (log_file == NULL) {
        fprintf(stderr, "Error opening log file %s: %s\n", log_file_path, strerror(errno));
        return;
    }

    fprintf(log_file, "[%s]::%s::[%s]::[%s]\n", success ? "SUCCESS" : "FAILED", timestamp, tag, information);
    fclose(log_file);
}


static void decrypt_rot13(char *str) {
    if (!str) return;
    for (int i = 0; str[i]; i++) {
        if (isalpha(str[i])) {
            char base = isupper(str[i]) ? 'A' : 'a';
            str[i] = (((str[i] - base) - 13 + 26) % 26) + base;
        }
    }
}

static void decode_hex(const char *input, char *output) {
    size_t len = strlen(input) / 2;
    for (size_t i = 0; i < len; i++) {
        sscanf(input + 2 * i, "%2hhx", &output[i]);
    }
    output[len] = '\0';
}


static void reverse_string(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}


static void decode_base64(const char *input, char *output) {
    int length = strlen(input);
    int decoded_length = length / 4 * 3;
    if (input[length - 1] == '=') decoded_length--;
    if (input[length - 2] == '=') decoded_length--;

    unsigned char *decoded_data = malloc(decoded_length + 1);
    if (!decoded_data) return;

    for (int i = 0, j = 0; i < length;) {
        uint32_t sextet_a = input[i] == '=' ? 0 & i++ : strchr(base64_table, input[i++]) - base64_table;
        uint32_t sextet_b = input[i] == '=' ? 0 & i++ : strchr(base64_table, input[i++]) - base64_table;
        uint32_t sextet_c = input[i] == '=' ? 0 & i++ : strchr(base64_table, input[i++]) - base64_table;
        uint32_t sextet_d = input[i] == '=' ? 0 & i++ : strchr(base64_table, input[i++]) - base64_table;
        uint32_t triple = (sextet_a << 18) | (sextet_b << 12) | (sextet_c << 6) | sextet_d;

        if (j < decoded_length) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < decoded_length) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < decoded_length) decoded_data[j++] = triple & 0xFF;
    }

    decoded_data[decoded_length] = '\0';
    strcpy(output, (char *)decoded_data);
    free(decoded_data);
}


static void decrypt_message(const char *path, char *buf) {
    char *temp_buf = strdup(buf);
    if (!temp_buf) return;

    if (strstr(path, "rev") != NULL) {
        reverse_string(temp_buf);
        log_event("moveFile", "File moved successfully", 1);
    } else if (strstr(path, "hex") != NULL) {
        decode_hex(temp_buf, buf);
        free(temp_buf);
        log_event("moveFile", "File moved successfully", 1);
        return;
    } else if (strstr(path, "base64") != NULL) {
        decode_base64(temp_buf, buf);
        free(temp_buf);
        log_event("moveFile", "File moved successfully", 1);
        return;
    } else if (strstr(path, "rot13") != NULL) {
        decrypt_rot13(temp_buf);
        log_event("moveFile", "File moved successfully", 1);
    } else {
        log_event("moveFile", "File moved successfully", 1);
    }

    strcpy(buf, temp_buf);
    free(temp_buf);
}

static int fs_getattr(const char *path, struct stat *stbuf) {
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else {
        char full_path[1000];
        snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);
        res = lstat(full_path, stbuf);
        if (res == -1) {
            return -errno;
        }
    }
    return res;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    char full_path[1000];
    snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);

    DIR *dp = opendir(full_path);
    if (dp == NULL) {
        return -errno;
    }

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
        if (filler(buf, de->d_name, NULL, 0)) break;
    }

    closedir(dp);
    return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi) {
    char full_path[1000];
    snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);

    int fd = open(full_path, fi->flags);
    if (fd == -1) {
        return -errno;
    }

    close(fd);
    return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char full_path[1000];
    snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);

    int fd = open(full_path, O_RDONLY);
    if (fd == -1) {
        return -errno;
    }

    if (lseek(fd, offset, SEEK_SET) == -1) {
        close(fd);
        return -errno;
    }

    char *read_buf = malloc(size + 1);
    if (!read_buf) {
        close(fd);
        return -ENOMEM;
    }

    ssize_t bytes_read = read(fd, read_buf, size);
    if (bytes_read == -1) {
        close(fd);
        free(read_buf);
        return -errno;
    }

    read_buf[bytes_read] = '\0';


    decrypt_message(path, read_buf);

    size_t decrypted_length = strlen(read_buf);
    if (decrypted_length > size) decrypted_length = size;

    memcpy(buf, read_buf, decrypted_length);
    free(read_buf);
    close(fd);
    return decrypted_length;
}

static int fs_rename(const char *from, const char *to) {
    char full_from[1000];
    char full_to[1000];
    snprintf(full_from, sizeof(full_from), "%s%s", source_dir, from);
    snprintf(full_to, sizeof(full_to), "%s%s", source_dir, to);

    int res = rename(full_from, full_to);
    if (res == -1) {
        log_event("moveFile", "Failed to move file", 0);
        return -errno;
    }

    log_event("moveFile", "File moved successfully", 1);
    return 0;
}

static struct fuse_operations fs_ops = {
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .open = fs_open,
    .read = fs_read,
    .rename = fs_rename,
};

int main(int argc, char *argv[]) {
    umask(0);
    return fuse_main(argc, argv, &fs_ops, NULL);
}
```

Pada kode tersebut terdapat beberapa fitur, seperti untuk mendekrip pesan dan pencatatan dalam file log.txt

Berikut ini merupakan kode untuk pencatatan dalam file log.txt
```
static void log_event(const char *tag, const char *information, int success) {
    time_t now;
    struct tm *local_time;
    char timestamp[20];

    time(&now);
    local_time = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%d/%m/%Y-%H:%M:%S", local_time);

    FILE *log_file = fopen(log_file_path, "a");
    if (log_file == NULL) {
        fprintf(stderr, "Error opening log file %s: %s\n", log_file_path, strerror(errno));
        return;
    }

    fprintf(log_file, "[%s]::%s::[%s]::[%s]\n", success ? "SUCCESS" : "FAILED", timestamp, tag, information);
    fclose(log_file);
}
```

Berikut ini merupakan beberapa fungsi untuk mendekrip pesan dengan menggunakan algoritma base64, reverse, heksadesimal, dan rot13. Selain itu pada bagian bawah, terdapat fungsi untuk pemanggilan fungsi deksrip ketika menemukan judul file yang sesuai dengan algoritma dekripnya 
```
static void decrypt_rot13(char *str) {
    if (!str) return;
    for (int i = 0; str[i]; i++) {
        if (isalpha(str[i])) {
            char base = isupper(str[i]) ? 'A' : 'a';
            str[i] = (((str[i] - base) - 13 + 26) % 26) + base;
        }
    }
}

static void decode_hex(const char *input, char *output) {
    size_t len = strlen(input) / 2;
    for (size_t i = 0; i < len; i++) {
        sscanf(input + 2 * i, "%2hhx", &output[i]);
    }
    output[len] = '\0';
}


static void reverse_string(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}


static void decode_base64(const char *input, char *output) {
    int length = strlen(input);
    int decoded_length = length / 4 * 3;
    if (input[length - 1] == '=') decoded_length--;
    if (input[length - 2] == '=') decoded_length--;

    unsigned char *decoded_data = malloc(decoded_length + 1);
    if (!decoded_data) return;

    for (int i = 0, j = 0; i < length;) {
        uint32_t sextet_a = input[i] == '=' ? 0 & i++ : strchr(base64_table, input[i++]) - base64_table;
        uint32_t sextet_b = input[i] == '=' ? 0 & i++ : strchr(base64_table, input[i++]) - base64_table;
        uint32_t sextet_c = input[i] == '=' ? 0 & i++ : strchr(base64_table, input[i++]) - base64_table;
        uint32_t sextet_d = input[i] == '=' ? 0 & i++ : strchr(base64_table, input[i++]) - base64_table;
        uint32_t triple = (sextet_a << 18) | (sextet_b << 12) | (sextet_c << 6) | sextet_d;

        if (j < decoded_length) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < decoded_length) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < decoded_length) decoded_data[j++] = triple & 0xFF;
    }

    decoded_data[decoded_length] = '\0';
    strcpy(output, (char *)decoded_data);
    free(decoded_data);
}


static void decrypt_message(const char *path, char *buf) {
    char *temp_buf = strdup(buf);
    if (!temp_buf) return;

    if (strstr(path, "rev") != NULL) {
        reverse_string(temp_buf);
        log_event("moveFile", "File moved successfully", 1);
    } else if (strstr(path, "hex") != NULL) {
        decode_hex(temp_buf, buf);
        free(temp_buf);
        log_event("moveFile", "File moved successfully", 1);
        return;
    } else if (strstr(path, "base64") != NULL) {
        decode_base64(temp_buf, buf);
        free(temp_buf);
        log_event("moveFile", "File moved successfully", 1);
        return;
    } else if (strstr(path, "rot13") != NULL) {
        decrypt_rot13(temp_buf);
        log_event("moveFile", "File moved successfully", 1);
    } else {
        log_event("moveFile", "File moved successfully", 1);
    }

    strcpy(buf, temp_buf);
    free(temp_buf);
}
```
Lalu berikut merupakan pendefinisian fuse dan fungsi utama
```
static int fs_getattr(const char *path, struct stat *stbuf) {
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else {
        char full_path[1000];
        snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);
        res = lstat(full_path, stbuf);
        if (res == -1) {
            return -errno;
        }
    }
    return res;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    char full_path[1000];
    snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);

    DIR *dp = opendir(full_path);
    if (dp == NULL) {
        return -errno;
    }

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
        if (filler(buf, de->d_name, NULL, 0)) break;
    }

    closedir(dp);
    return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi) {
    char full_path[1000];
    snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);

    int fd = open(full_path, fi->flags);
    if (fd == -1) {
        return -errno;
    }

    close(fd);
    return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char full_path[1000];
    snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);

    int fd = open(full_path, O_RDONLY);
    if (fd == -1) {
        return -errno;
    }

    if (lseek(fd, offset, SEEK_SET) == -1) {
        close(fd);
        return -errno;
    }

    char *read_buf = malloc(size + 1);
    if (!read_buf) {
        close(fd);
        return -ENOMEM;
    }

    ssize_t bytes_read = read(fd, read_buf, size);
    if (bytes_read == -1) {
        close(fd);
        free(read_buf);
        return -errno;
    }

    read_buf[bytes_read] = '\0';


    decrypt_message(path, read_buf);

    size_t decrypted_length = strlen(read_buf);
    if (decrypted_length > size) decrypted_length = size;

    memcpy(buf, read_buf, decrypted_length);
    free(read_buf);
    close(fd);
    return decrypted_length;
}

static int fs_rename(const char *from, const char *to) {
    char full_from[1000];
    char full_to[1000];
    snprintf(full_from, sizeof(full_from), "%s%s", source_dir, from);
    snprintf(full_to, sizeof(full_to), "%s%s", source_dir, to);

    int res = rename(full_from, full_to);
    if (res == -1) {
        log_event("moveFile", "Failed to move file", 0);
        return -errno;
    }

    log_event("moveFile", "File moved successfully", 1);
    return 0;
}

static struct fuse_operations fs_ops = {
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .open = fs_open,
    .read = fs_read,
    .rename = fs_rename,
};

int main(int argc, char *argv[]) {
    umask(0);
    return fuse_main(argc, argv, &fs_ops, NULL);
}
```

### Hasil Pengerjaan
Direktori sebelum dijalankan
![Screenshot (738)](https://github.com/SyahmiAsh/Sisop-4-2024-MH-IT14/assets/150339585/e1fb7646-5652-4c2c-8701-5f34558a8346)

Isi pesan sebelum program dijalankan
![Screenshot (739)](https://github.com/SyahmiAsh/Sisop-4-2024-MH-IT14/assets/150339585/909d3d88-96d8-47ce-9175-66949bda4370)

Isi pesan setelah program dijalankan 
![Screenshot (740)](https://github.com/SyahmiAsh/Sisop-4-2024-MH-IT14/assets/150339585/2368ab37-3c2a-48fc-ae9e-acd16f282451)


### Revisi 
Terdapat point yang harus direvisi, yaitu kurang untuk penanganan file 'rahasia'


## Soal 3
oleh Muhammad Faqih Husain

Seorang arkeolog menemukan sebuah gua yang didalamnya tersimpan banyak relik dari zaman praaksara, sayangnya semua barang yang ada pada gua tersebut memiliki bentuk yang terpecah belah akibat bencana yang tidak diketahui. Sang arkeolog ingin menemukan cara cepat agar ia bisa menggabungkan relik-relik yang terpecah itu, namun karena setiap pecahan relik itu masih memiliki nilai tersendiri, ia memutuskan untuk membuat sebuah file system yang mana saat ia mengakses file system tersebut ia dapat melihat semua relik dalam keadaan utuh, sementara relik yang asli tidak berubah sama sekali.

File relic
https://drive.google.com/file/d/1BJkaBvGaxqiwPWvXRdYNXzxxmIYQ8FKf/view?usp=sharing

### Solusi 

#### `Archeology.c`
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
#### Deskripsi Fungsi
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

#### Command yang dapat dijalankan di dalam fuse tersebut
- ls: Menampilkan daftar file dan direktori di dalam filesystem.
Perintah ini akan memanggil fungsi do_readdir.
- cat: Menampilkan isi file.
Perintah ini akan memanggil fungsi do_open dan do_read.
- echo: Menulis teks ke file.
Perintah ini akan memanggil fungsi do_create (jika file belum ada) dan do_write.
- touch: Membuat file baru.
Perintah ini akan memanggil fungsi do_create.
- rm: Menghapus file.
Perintah ini akan memanggil fungsi do_unlink.
- mkdir: Membuat direktori baru.
Perintah ini akan memanggil fungsi do_mkdir.
- rmdir: Menghapus direktori kosong.
Perintah ini akan memanggil fungsi do_rmdir.
- chmod: Mengubah izin file atau direktori.
Perintah ini akan memanggil fungsi do_chmod.
- chown: Mengubah kepemilikan file atau direktori.
Perintah ini akan memanggil fungsi do_chown.
- mv: Memindahkan atau mengganti nama file atau direktori.
Meskipun tidak ada fungsi rename yang didefinisikan dalam kode, mv akan bekerja dengan menggabungkan operasi rename dari sistem file dasar.
- cp: Menyalin file.
Perintah ini akan memanggil fungsi do_create dan do_write untuk file tujuan.
- truncate: Mengubah ukuran file.
Perintah ini akan memanggil fungsi do_truncate.

#### Pemecahan File (Splitting)
Ketika menulis ke file, program memecah data menjadi beberapa bagian kecil dan menyimpannya sebagai file yang terpisah. Setiap bagian file disimpan dengan nama yang diakhiri dengan .000, .001, .002, dan seterusnya.

do_write
Fungsi do_write bertanggung jawab untuk menulis data ke file dan memecahnya menjadi beberapa bagian jika diperlukan.
- Lokasi dan Offset: Menentukan bagian file berdasarkan offset.
- Loop: Menulis data ke file bagian hingga semua data tertulis.
- Nama File: Setiap bagian diberi nama dengan format {fullpath}.{part_num}, di mana part_num adalah nomor bagian yang diurutkan.
Berikut adalah langkah-langkah yang diambil oleh do_write:
1. Hitung nomor bagian (part_num) berdasarkan offset.
2. Hitung offset dalam bagian (part_offset).
3. Tulis data ke bagian file.
4. Lanjutkan ke bagian berikutnya hingga semua data tertulis.

#### Penggabungan File (Merging)
Ketika membaca file, program menggabungkan data dari beberapa bagian file menjadi satu konten yang utuh.

do_read
Fungsi do_read bertanggung jawab untuk membaca data dari file yang dipecah menjadi beberapa bagian dan menggabungkannya kembali menjadi satu konten utuh.
- Lokasi dan Offset: Menentukan bagian file berdasarkan offset.
- Loop: Membaca data dari bagian file hingga jumlah data yang diminta tercapai.
- Nama File: Setiap bagian file diakses dengan nama yang berurutan sesuai nomor bagian (.000, .001, dll).
Berikut adalah langkah-langkah singkat yang diambil oleh do_read:
1. Hitung nomor bagian (part_num) berdasarkan offset.
2. Hitung offset dalam bagian (part_offset).
3. Baca data dari bagian file.
4. Lanjutkan ke bagian berikutnya hingga semua data terbaca atau jumlah data yang diminta tercapai.

Cara Penggunaan
```
./archeology fuze
cp twibbon.jpg ./fuze
cp ./fuze/relic_1.png .
cp twibbon.jpg ./fuze
rm ./fuze/relic_1.png
cp fuze/* report/  
usermount -u fuze
```
- Ketika dilakukan listing, isi dari direktori `fuze` adalah semua relic dari relics yang telah tergabung.
- Ketika ada file dibuat, maka pada direktori asal (direktori relics) file tersebut akan dipecah menjadi sejumlah pecahan dengan ukuran maksimum tiap pecahan adalah 10kb.
- File yang dipecah akan memiliki nama [namafile].000 dan seterusnya sesuai dengan jumlah pecahannya.

![image](https://github.com/SyahmiAsh/Sisop-4-2024-MH-IT14/assets/88548292/76241d2b-0f8b-4a9d-b0fd-621de9e8e57f)

- Ketika dilakukan copy (dari direktori fuze ke tujuan manapun), file yang disalin adalah file dari direktori relics yang sudah tergabung.
- Ketika dilakukan penghapusan, maka semua pecahannya juga ikut terhapus.

![image](https://github.com/SyahmiAsh/Sisop-4-2024-MH-IT14/assets/88548292/9729d439-0858-4aa3-ac75-7e7d683aa671)

- Direktori report adalah direktori yang akan dibagikan menggunakan Samba File Server. Setelah kalian berhasil membuat direktori fuze, jalankan FUSE dan salin semua isi direktori fuze pada direktori report.

![image](https://github.com/SyahmiAsh/Sisop-4-2024-MH-IT14/assets/88548292/365e702d-7b09-4aee-b926-81feb6b78028)


Membagikan direktory `report` menggunakan samba

Konfigurasi samba `vim /etc/samba/smb.conf` tambahkan konfigurasi berikut pada line paling bawah
```
[bagibagi]
     comment = Samba on Ubuntu
     path = /home/kali/Sisop/modul4/soal_3/report
     read only = no
     browsable = yes
     writable = yes
     guest ok = no
```
Memulai smb service
```
sudo service smbd start
service smbd status
```
![image](https://github.com/SyahmiAsh/Sisop-4-2024-MH-IT14/assets/88548292/109f605e-af3f-4eeb-a3f5-5856f8b01b51)

Membuat user
```
sudo smbpasswd -a username
```
Melihat ip dari linux
```
ip addr
```

Menghubungkan file manager `windows` folder report dengan menggunakan `\\ip-address\sambashare`

![image](https://github.com/SyahmiAsh/Sisop-4-2024-MH-IT14/assets/88548292/63f83f40-f7b9-4716-9219-10a6b4f55581)

Melihat gambar image yang telah digabungkan

![image](https://github.com/SyahmiAsh/Sisop-4-2024-MH-IT14/assets/88548292/feb48440-4851-42c2-a766-e629fd23b3a2)

```
sudo service smbd stop
```
### Revisi 
Tidak ada catatan dari asisten penguji
