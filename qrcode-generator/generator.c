#include <stdbool.h>
#include <stdint.h>
#include "lib/qrcodegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "lib/TinyPngOut.h"

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
    if (fgets(inputText, QR_MAX_INPUT_LEN, stdin) == NULL) {
        printf("Failed to read input\n");
        return 1;
    }
    // Remove trailing newline character(s)
    inputText[strcspn(inputText, "\r\n")] = '\0';

    bool ok = qrcodegen_encodeText(inputText, tempBuffer, qr, qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
    if (!ok) {
        printf("Failed to encode QR code\n");
        return 1;
    }

    // create out file
    FILE *file = fopen("out.png", "wb");
    if (file == NULL) {
        printf("Couldn't open the output file\n");
        exit(-1);
    }

    // squared image with border/quiet zone (4 modules)
    int size = qrcodegen_getSize(qr);
    int border = 4;
    int imgSize = (size + border * 2) * SCALE;
    printf("QR Code Size: %d, Output Image Size: %d x %d\n", size, imgSize, imgSize);

    struct TinyPngOut writer;
    enum TinyPngOut_Status init_status = TinyPngOut_init(&writer, imgSize, imgSize, file);
    if (init_status != TINYPNGOUT_OK) {
        printf("Couldn't initialize TinyPngOut writer. Status: %d\n", init_status);
        fclose(file);
        exit(-1);
    }

    // draw qr code
    uint8_t rgb_black[] = {0, 0, 0};
    uint8_t rgb_white[] = {255, 255, 255};
    for (int y = 0; y < imgSize; y++) {
        for (int x = 0; x < imgSize; x++) {
            int qrX = x / SCALE - border;
            int qrY = y / SCALE - border;
            bool color = false;
            if (qrX >= 0 && qrX < size && qrY >= 0 && qrY < size) {
                color = qrcodegen_getModule(qr, qrX, qrY);
            }
            TinyPngOut_write(&writer, color ? rgb_black : rgb_white, 1);
        }
    }

    fclose(file);



    return 0;
}
