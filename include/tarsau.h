#ifndef TARSAU_H
#define TARSAU_H

#include <stdbool.h>

int archive_files(char *input_files[], int file_count, const char *output_file);
bool is_ascii_file(const char *filename);

int extract_archive(const char *archive_name, const char *target_dir);
#endif
