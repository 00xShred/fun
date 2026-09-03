#include <raylib.h>
#include <stdio.h>
#include <string.h>

char *suffix = ".compressed";

int main (int argc, char *argv[]) {


    if (argc != 2 && argc != 3) {
        printf("Usage %s [-d] <file>\n", argv[0]);
        printf("\t-d\t decompress provided file\n");
        return 1;
    }

    if (argc == 2){
        char *fn = argv[1];
    }

    // handle -d flag
    char *fn;
    bool decompress = false;
    if (argc == 3) {
        char *decompress_opt = argv[1];
        if (!strcmp(decompress_opt, "-d")){
            fn = argv[2];
            decompress = true;
        } else {
            printf("Invalid flag provided: %s\n, decompress_opt");
            return 1;
        }
    }

    // input file
    FILE* input_fp = fopen(fn,"r");
    if (input_fp == NULL) {
        perror ("Failed opening the passed file\n");
        return 1;
    }

    // output file
    suffix = decompress ? ".decompressed" : ".compressed";
    int filename_length = strlen(fn) + strlen(suffix);
    char output_fn [filename_length];
    sprintf(output_fn, "%s.%s", output_fn, suffix);

    FILE* output_fp = fopen(output_fn,"r");
    if (output_fp == NULL) {
        perror ("Failed opening the passed file");
        return 1;
    }

    strcat(output_fn, suffix);
    printf("Output filename: %s\n", output_fn);


    return 0;
}
