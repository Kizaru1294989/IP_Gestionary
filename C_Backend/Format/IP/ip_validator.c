#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <postgresql/libpq-fe.h>
#include <libwebsockets.h>
#include "ip_validator.h"

bool validate_ip(char *ip) {
    char copieIP[16];
    strcpy(copieIP, ip);
    int octet, num;
    int compteur = 0;
    char *token = strtok(copieIP, ".");
    while (token != NULL) {
        for (int i = 0; i < strlen(token); i++) {
            if (!isdigit(token[i])) {
                return false; 
            }
        }
        num = strtol(token, NULL, 10);

        if (num >= 0 && num <= 255) {
            token = strtok(NULL, ".");
            compteur++;
        } else {
            return false; 
        }
    }
    return compteur == 4;
}

