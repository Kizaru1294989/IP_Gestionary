#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <postgresql/libpq-fe.h>
#include <libwebsockets.h>
#include "convertion.h"



int is_valid_ip(char *ip) {
    return 1; 
}

char* convert_to_hexadecimal(char *ip) {
    if (is_valid_ip(ip)) {
        unsigned int octet1, octet2, octet3, octet4;
        char *hex_ip = (char*)malloc(18); 
        sscanf(ip, "%u.%u.%u.%u", &octet1, &octet2, &octet3, &octet4);
        snprintf(hex_ip, 18, "%02X.%02X.%02X.%02X", octet1, octet2, octet3, octet4);
        return hex_ip;
    } else {
        return NULL; 
    }
}

char* convert_to_binary(const char *ip) {
    char *binary_ip = malloc(35); 
    if (binary_ip == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }

    int index = 0;
    int octet = 0;
    int isFirstOctet = 1; 
    for (int i = 0; i <= strlen(ip); i++) {
        if (ip[i] == '.' || ip[i] == '\0') {
            for (int j = 7; j >= 0; j--) {
                binary_ip[index++] = (octet & (1 << j)) ? '1' : '0';
            }
            if (isFirstOctet && index < 8) {
                for (int k = 0; k < 8 - index; k++) {
                    binary_ip[index++] = '0';
                }
            }
            if (index < 31) {
                binary_ip[index++] = '.'; 
            }
            isFirstOctet = 0; 
            octet = 0; 
        } else {
            octet = octet * 10 + (ip[i] - '0');
        }
    }
    binary_ip[index] = '\0';

    return binary_ip;
}
