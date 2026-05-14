#include "tarsau.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// Giriş dosyalarının toplam boyutu 200 MB'ı geçemez
#define MAX_TOTAL_SIZE (200 * 1024 * 1024)

// 1. ALT FONKSİYON: ASCII Kontrolü
bool is_ascii_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) return false;

    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch > 127 || ch == 0) {
            fclose(file);
            return false;
        }
    }

    fclose(file);
    return true;
}

// 2. ALT FONKSİYON: Organizasyon Bölümünü Hazırlama
static int prepare_organization_section(char *input_files[],
                                        int file_count,
                                        char *header_buffer,
                                        size_t *total_header_size) {
    if (file_count > 32) {
        printf("Hata: Arşivlenebilecek maksimum dosya sayısı 32'dir.\n");
        return 1;
    }

    long total_files_size = 0;
    struct stat st;
    char records_buffer[65536] = "";

    for (int i = 0; i < file_count; i++) {
        if (strchr(input_files[i], ',') || strchr(input_files[i], '|')) {
            printf("Hata: Dosya adinda gecersiz karakter var.\n");
            return 1;
        }

        if (stat(input_files[i], &st) != 0) {
            printf("Hata: %s dosyasinin bilgileri okunamadi.\n", input_files[i]);
            return 1;
        }

        if (!S_ISREG(st.st_mode) || !is_ascii_file(input_files[i])) {
            printf("%s giriş dosyasının formatı uyumsuzdur!\n", input_files[i]);
            return -1;
        }

        total_files_size += st.st_size;
        if (total_files_size > MAX_TOTAL_SIZE) {
            printf("Hata: Giriş dosyalarının toplam boyutu 200 MB'ı geçemez.\n");
            return 1;
        }

        unsigned int perms = st.st_mode & 0777;
        char record[1024];

        snprintf(record, sizeof(record), "%s,%o,%ld|", input_files[i], perms, (long)st.st_size);
        strncat(records_buffer, record, sizeof(records_buffer) - strlen(records_buffer) - 1);
    }

    size_t records_len = strlen(records_buffer);
    *total_header_size = 10 + records_len;

    snprintf(header_buffer, 11, "%010zu", *total_header_size);
    strncat(header_buffer, records_buffer, 65536 - strlen(header_buffer) - 1);

    return 0;
}

// 3. ALT FONKSİYON: Dosyaları Arşive Yazma
static int append_files_to_archive(char *input_files[], int file_count, FILE *out) {
    for (int i = 0; i < file_count; i++) {
        FILE *in = fopen(input_files[i], "rb");
        if (!in) {
            printf("Hata: %s dosyasi okunamadi.\n", input_files[i]);
            return 1;
        }

        char buffer[4096];
        size_t bytes_read;

        while ((bytes_read = fread(buffer, 1, sizeof(buffer), in)) > 0) {
            if (fwrite(buffer, 1, bytes_read, out) != bytes_read) {
                printf("Hata: Arsiv yazma hatasi.\n");
                fclose(in);
                return 1;
            }
        }

        fclose(in);
    }

    return 0;
}

// ANA ARŞİVLEME FONKSİYONU
int archive_files(char *input_files[], int file_count, const char *output_file) {
    char header_buffer[65536] = "";
    size_t total_header_size = 0;

    int prep_status = prepare_organization_section(input_files, file_count, header_buffer, &total_header_size);
    if (prep_status == -1) {
        return 0;
    } else if (prep_status != 0) {
        return 1;
    }

    FILE *out = fopen(output_file, "wb");
    if (!out) {
        printf("Hata: Hedef arsiv dosyasi olusturulamadi.\n");
        return 1;
    }

    if (fwrite(header_buffer, 1, total_header_size, out) != total_header_size) {
        printf("Hata: Arsiv yazma hatasi.\n");
        fclose(out);
        return 1;
    }

    if (append_files_to_archive(input_files, file_count, out) != 0) {
        fclose(out);
        return 1;
    }

    fclose(out);
    printf("Dosyalar birleştirildi.\n");

    return 0;
}

// ÇIKARMA FONKSİYONU
int extract_archive(const char *archive_name, const char *target_dir) {
    const char *ext = strrchr(archive_name, '.');
    if (!ext || strcmp(ext, ".sau") != 0) {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        return 0;
    }

    FILE *sau_file = fopen(archive_name, "rb");
    if (!sau_file) {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        return 0;
    }

    char old_dir[1024];
    getcwd(old_dir, sizeof(old_dir));

    char header_size_str[11] = {0};
    if (fread(header_size_str, 1, 10, sau_file) != 10) {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        fclose(sau_file);
        return 0;
    }

    char *endptr;
    long header_val = strtol(header_size_str, &endptr, 10);
    if (*endptr != '\0' || header_val <= 10) {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        fclose(sau_file);
        return 0;
    }

    size_t header_size = (size_t)header_val;
    size_t records_size = header_size - 10;

    char *header_buffer = (char *)malloc(records_size + 1);
    if (!header_buffer) {
        printf("Hata: Bellek tahsis edilemedi.\n");
        fclose(sau_file);
        return 1;
    }

    if (fread(header_buffer, 1, records_size, sau_file) != records_size) {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        free(header_buffer);
        fclose(sau_file);
        return 0;
    }

    header_buffer[records_size] = '\0';

    if (target_dir != NULL) {
        struct stat st = {0};
        if (stat(target_dir, &st) == -1) {
            if (mkdir(target_dir, 0777) != 0) {
                printf("Hata: %s dizini oluşturulamadı.\n", target_dir);
                free(header_buffer);
                fclose(sau_file);
                return 1;
            }
        }

        if (chdir(target_dir) != 0) {
            printf("Hata: %s dizinine erişilemedi.\n", target_dir);
            free(header_buffer);
            fclose(sau_file);
            return 1;
        }
    }

    char *saveptr;
    char *token = strtok_r(header_buffer, "|", &saveptr);

    // Açılan dosya adlarını saklamak için dizi (En fazla 32 dosya sınırı)
    char extracted_names[32][256] = {0};
    int file_count = 0;

    while (token != NULL) {
        char file_name[256] = {0};
        unsigned int perms = 0;
        long file_size = 0;

        if (sscanf(token, "%255[^,],%o,%ld", file_name, &perms, &file_size) == 3) {
            FILE *out_file = fopen(file_name, "wb");
            if (!out_file) {
                printf("Hata: %s dosyası açılamadı.\n", file_name);
            } else {
                char data_buffer[4096];
                long bytes_left = file_size;

                while (bytes_left > 0) {
                    size_t chunk = (bytes_left > (long)sizeof(data_buffer)) ? sizeof(data_buffer) : (size_t)bytes_left;
                    size_t read_bytes = fread(data_buffer, 1, chunk, sau_file);

                    if (read_bytes != chunk) {
                        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
                        fclose(out_file);
                        free(header_buffer);
                        fclose(sau_file);
                        return 0;
                    }

                    if (fwrite(data_buffer, 1, read_bytes, out_file) != read_bytes) {
                        printf("Hata: Dosya yazma hatasi.\n");
                        fclose(out_file);
                        free(header_buffer);
                        fclose(sau_file);
                        return 1;
                    }

                    bytes_left -= read_bytes;
                }

                fclose(out_file);
                chmod(file_name, perms);

                // Dosya adını listeye kaydet
                if (file_count < 32) {
                    strncpy(extracted_names[file_count], file_name, 255);
                    file_count++;
                }
            }
        }

        token = strtok_r(NULL, "|", &saveptr);
    }

    free(header_buffer);
    fclose(sau_file);

    if (target_dir != NULL) {
        chdir(old_dir);
    }

    char extracted_files_list[4096] = "";
    for (int i = 0; i < file_count; i++) {
        strncat(extracted_files_list, extracted_names[i], sizeof(extracted_files_list) - strlen(extracted_files_list) - 1);

        if (i < file_count - 2) {
            strncat(extracted_files_list, ", ", sizeof(extracted_files_list) - strlen(extracted_files_list) - 1);
        } else if (i == file_count - 2) {
            strncat(extracted_files_list, " ve ", sizeof(extracted_files_list) - strlen(extracted_files_list) - 1);
        }
    }

    const char *dir_print_name = (target_dir != NULL) ? target_dir : ".";
    printf("%s dizininde %s dosyaları açıldı.\n", dir_print_name, extracted_files_list);

    return 0;
}