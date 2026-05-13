#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tarsau.h"

int main(int argc, char *argv[]) {

    if (argc < 2) {

        printf("Kullanim:\n");
        printf("Arsivleme: ./tarsau -b dosya1 dosya2 -o arsiv.sau\n");
        printf("Cikarma: ./tarsau -a arsiv.sau hedef_dizin\n");

        return 1;
    }

    // ARŞİVLEME MODU
    if (strcmp(argv[1], "-b") == 0) {

        char *output_file = "a.sau";

        int file_count = 0;

        char *input_files[32];

        for (int i = 2; i < argc; i++) {

            // Çıkış dosya adı
            if (strcmp(argv[i], "-o") == 0) {

                if (i + 1 >= argc) {

                    printf("Hata: Arsiv dosya adi eksik.\n");

                    return 1;
                }

                output_file = argv[i + 1];

                i++;
            }

            else {

                if (file_count >= 32) {

                    printf("Hata: Arşivlenebilecek maksimum dosya sayısı 32'dir.\n");

                    return 1;
                }

                input_files[file_count] = argv[i];

                file_count++;
            }
        }

        if (file_count == 0) {

            printf("Hata: Arsivlenecek hic dosya belirtilmedi.\n");

            return 1;
        }

        return archive_files(input_files,
                             file_count,
                             output_file);
    }

    // ÇIKARMA MODU
    else if (strcmp(argv[1], "-a") == 0) {

        if (argc < 3 || argc > 4) {

            printf("Arşiv dosyası uygunsuz veya bozuk!\n");

            return 0;
        }

        char *archive_file = argv[2];

        char *target_dir = NULL;

        if (argc == 4) {
            target_dir = argv[3];
        }

        return extract_archive(archive_file,
                               target_dir);
    }

    // GEÇERSİZ PARAMETRE
    else {

        printf("Hata: Gecersiz islem!\n");
        printf("Lutfen -b veya -a parametrelerini kullanin.\n");

        return 1;
    }

    return 0;
}