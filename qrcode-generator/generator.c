#include <stdbool.h>
#include <stdint.h>
#include <qrcodegen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <TinyPngOut.h>

#define SCALE 10
#define QR_MAX_INPUT_LEN 2953

int main() {

    // download tinypng and qrcode libs

    // text data
    uint8_t qr[qrcodegen_BUFFER_LEN_MAX];
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
    char inputText[QR_MAX_INPUT_LEN];
    memset(inputText, 0, QR_MAX_INPUT_LEN);

    // take input from user
    fgets(inputText, QR_MAX_INPUT_LEN, stdin);

    bool ok = qrcodegen_encondeText(inputText, tempBuffer, qr0, qrcodegen_Ecc_MEDIUM ,qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mast_AUTO, true);
    if (!ok) {
        return 0;
    }

    // create out file
    FILE *file = fopen("out.png", "w");
    if (file == NULL) {
        printf("Couldn't open the output file\n");
        exit(-1);
    }

    // squared image 
    int size = qrcodegen_getSize(qr0);
    printf("Size is: %d", size);
    struct TinyPngOut writer;
    enum TinyPngOut_Status init_status = TinyPngOut_init(&writer, size*SCALE, size*SCALE, file);
    if (init_status != TINYPNGOUTPUT_OK) {
        printf("Couldn't initialize TinyPngOut writer. Status: %d\n", init_status);
        exit(-1);
    }

    // draw qr code
    uint8_t rgb_black[] = {0, 0, 0};
    uint8_t rgb_white[] = {255, 255, 255};
    for (int y = 0; y < size*SCALE; y++) {
        for (int x = 0; x < size*SCALE; x++) {
            bool color = qrcodegen_getModule(qr, x/SCALE, y/SCALE);
            TinyPngOut_write(&writer, color ? rgb_white : rgb_black, 1);
        }
    }



    return 0;
}
