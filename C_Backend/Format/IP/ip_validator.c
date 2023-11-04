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
   int i, num, dots = 0;
   char *ptr;
   if (copieIP == NULL)
      return false;
      ptr = strtok(copieIP, "."); 
      if (ptr == NULL)
         return false;
   while (ptr) {
      if (!validate_number(ptr)) 
         return false;
         num = atoi(ptr); 
         if (num >= 0 && num <= 255) {
            ptr = strtok(NULL, ".");
            if (ptr != NULL)
               dots++; 
         } else
            return false;
    }
    if (dots != 3) 
       return false;
      return true;
}

bool validate_number(char *str) {
   while (*str) {
      if(!isdigit(*str)){ 
         return false;
      }
      str++; 
   }
   return true;
}
