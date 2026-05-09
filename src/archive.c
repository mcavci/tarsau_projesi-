#include "tarsau.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_TOTAL_SIZE (200 * 1024 * 1024) // 200 MB sınır

// 1. ALT FONKSİYON: ASCII Kontrolü
bool is_ascii_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) return false;

    int ch;
    while ((ch = fgetc(file)) != EOF) {
        // Giriş dosyaları yalnızca metin dosyaları olabilir (ASCII, 1 bayt)
        if ((unsigned char)ch > 127) { 
            fclose(file);
            return false;
        }
    }
    fclose(file);
    return true;
}

// 2. ALT FONKSİYON: Organizasyon (İçerik) Bölümünü Hazırlama
static int prepare_organization_section(char *input_files[], int file_count, char *header_buffer, size_t *total_header_size) {
    long total_files_size = 0;
    struct stat st;
    char records_buffer[8192] = "";

    for (int i = 0; i < file_count; i++) {
        if (stat(input_files[i], &st) != 0) {
            printf("Hata: %s dosyasinin bilgileri okunamadi.\n", input_files[i]);
            return 1;
        }

        // Hatalı formatta bir giriş dosyası kontrolü
        if (!is_ascii_file(input_files[i])) {
            printf("%s giriş dosyasının formatı uyumsuzdur!\n", input_files[i]);
            return -1; // -1 özel durumu: sorunsuz bir şekilde çıkılması için
        }

        total_files_size += st.st_size;
        if (total_files_size > MAX_TOTAL_SIZE) {
            printf("Hata: Giriş dosyalarının toplam boyutu 200 MB'ı geçemez.\n");
            return 1;
        }

        unsigned int perms = st.st_mode & 0777;
        char record[512];
        
        // Kayıttaki alanlar: |Dosya adı, izinler, boyut|
        snprintf(record, sizeof(record), "|%s,%04o,%ld|", input_files[i], perms, (long)st.st_size);
        strcat(records_buffer, record);
    }

    size_t records_len = strlen(records_buffer);
    *total_header_size = 10 + records_len; // İlk 10 bayt + kayıtların boyutu

    // İlk 10 bayt, ilk bölümün ASCII formatındaki sayısal boyutunu içerir
    snprintf(header_buffer, 11, "%010zu", *total_header_size);
    strcat(header_buffer, records_buffer);

    return 0;
}

// 3. ALT FONKSİYON: Arşivlenmiş Dosyaları Fiziksel Olarak Yazma
static int append_files_to_archive(char *input_files[], int file_count, FILE *out) {
    for (int i = 0; i < file_count; i++) {
        FILE *in = fopen(input_files[i], "rb");
        if (!in) {
            printf("Hata: %s dosyasi okunamadi.\n", input_files[i]);
            return 1;
        }

        char buffer[4096];
        size_t bytes_read;
        
        // Arşivlenmiş dosyalar, herhangi bir ayırıcı kullanılmadan art arda yerleştirilir
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), in)) > 0) {
            fwrite(buffer, 1, bytes_read, out);
        }
        fclose(in);
    }
    return 0;
}

// ANA ORKESTRATÖR FONKSİYON (-b modunun çağıracağı yer)
int archive_files(char *input_files[], int file_count, const char *output_file) {
    char header_buffer[8192] = "";
    size_t total_header_size = 0;

    // Meta verileri topla ve doğrulamaları yap
    int prep_status = prepare_organization_section(input_files, file_count, header_buffer, &total_header_size);
    if (prep_status == -1) {
        return 0; // Format uyumsuzsa sorunsuz çıkış yapıyoruz
    } else if (prep_status != 0) {
        return 1; // Başka bir hata oluştuysa çık
    }

    FILE *out = fopen(output_file, "wb");
    if (!out) {
        printf("Hata: Hedef arsiv dosyasi olusturulamadi.\n");
        return 1;
    }

    // 1) Organizasyon (içerik) bilgilerini yaz
    fwrite(header_buffer, 1, total_header_size, out);

    // 2) Arşivlenmiş dosyaları ekle
    if (append_files_to_archive(input_files, file_count, out) != 0) {
        fclose(out);
        return 1;
    }

    fclose(out);
    printf("Dosyalar birleştirildi.\n");
    return 0;
}