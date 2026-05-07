#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Argüman sayısı kontrolü
    if (argc < 2) {
        printf("Kullanim: \n");
        printf("Arsivleme: ./tarsau -b dosya1 dosya2 -o arsiv.sau\n");
        printf("Cikarma: ./tarsau -a arsiv.sau hedef_dizin\n");
        return 1;
    }

    // ARŞİVLEME MODU (-b)
    if (strcmp(argv[1], "-b") == 0) {
        printf("--- Arsivleme Modu Baslatildi ---\n");
        
        char *output_file = "a.sau"; // Varsayılan arşiv adı
        int file_count = 0;
        char *input_files[32]; // En fazla 32 dosya sınırı

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 < argc) {
                    output_file = argv[i + 1];
                    i++; // -o parametresinden sonraki dosya adını atla
                }
            } else {
                if (file_count < 32) {
                    input_files[file_count] = argv[i];
                    file_count++;
                } else {
                    printf("Hata: Giris dosyasi sayisi en fazla 32 olabilir!\n");
                    return 1;
                }
            }
        }

        printf("Hedef Arsiv Dosyasi: %s\n", output_file);
        printf("Arsivlenecek Dosyalar (%d adet):\n", file_count);
        for (int i = 0; i < file_count; i++) {
            printf(" -> %s\n", input_files[i]);
        }
        
        // TODO: Dosyaların boyut ve metin (ASCII) formatı kontrolleri buraya eklenecek.

    } 
    // ARŞİVDEN ÇIKARMA MODU (-a)
    else if (strcmp(argv[1], "-a") == 0) {
        printf("--- Arsivden Cikarma Modu Baslatildi ---\n");
        
        if (argc < 3 || argc > 4) {
            printf("Hata: -a parametresinden sonra en fazla 2 parametre almalidir.\n");
            return 1;
        }

        char *archive_file = argv[2];
        char *target_dir = "."; // İkinci parametre verilmezse geçerli dizin (nokta)

        if (argc == 4) {
            target_dir = argv[3];
        }

        printf("Acilacak Arsiv Dosyasi: %s\n", archive_file);
        printf("Hedef Dizin: %s\n", target_dir);
        
        // TODO: Arşiv dosyası bütünlük kontrolü ve çıkarma işlemleri buraya eklenecek.

    } 
    else {
        printf("Hata: Gecersiz islem! Lutfen -b veya -a parametrelerini kullanin.\n");
        return 1;
    }

    return 0;
}