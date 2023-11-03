#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <postgresql/libpq-fe.h>
#include <libwebsockets.h>

// Fonction de connexion à la base de données PostgreSQL
typedef struct {
    char *id;
    char *ipAddress;
    char *masque;
    char *ip_binary;
    char *ip_hexa;
} DatabaseEntry;



char* convertToJSON(DatabaseEntry entry) {
    // Modifiez le modèle JSON pour inclure les nouvelles variables
    char *jsonTemplate = "{\"ID\":\"%s\",\"IP Address\":\"%s\",\"Masque\":\"%s\",\"Binary IP\":\"%s\",\"Hexadecimal IP\":\"%s\"}";

    // Calculez la taille nécessaire pour la chaîne JSON
    int jsonSize = snprintf(NULL, 0, jsonTemplate, entry.id, entry.ipAddress, entry.masque, entry.ip_binary, entry.ip_hexa) + 1;

    // Allouez de la mémoire pour la chaîne JSON
    char *jsonString = malloc(jsonSize);

    // Formatage de la chaîne JSON
    sprintf(jsonString, jsonTemplate, entry.id, entry.ipAddress, entry.masque, entry.ip_binary, entry.ip_hexa);

    return jsonString;
}
char* SelectAllFromTable(PGconn *conn) {
    PGresult *res;

    // Exécute la requête SELECT * sur la table ip_address
    res = PQexec(conn, "SELECT * FROM ip_address");

    // Vérifie si la requête s'est exécutée avec succès
    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        int numRows = PQntuples(res);

        // Alloue de la mémoire pour jsonArray et initialise avec un crochet ouvrant
        char *jsonArray = malloc(2);
        strcpy(jsonArray, "[");

        for (int i = 0; i < numRows; i++) {
            DatabaseEntry entry;
            entry.id = PQgetvalue(res, i, 0);
            entry.ipAddress = PQgetvalue(res, i, 1);
            entry.masque = PQgetvalue(res, i, 2);
            entry.ip_hexa = PQgetvalue(res, i, 3);
            entry.ip_binary = PQgetvalue(res, i, 4);

            char *jsonString = convertToJSON(entry);

            // Réalloue de la mémoire pour augmenter la taille de jsonArray et ajoute jsonString
            jsonArray = realloc(jsonArray, strlen(jsonArray) + strlen(jsonString) + 2);
            strcat(jsonArray, jsonString);
            if (i < numRows - 1) {
                strcat(jsonArray, ",");
            }
            free(jsonString);
        }

        // Ajoute le crochet fermant à la fin et libère le résultat de la requête
        strcat(jsonArray, "]");
        PQclear(res);

        // Retourne le tableau JSON
        return jsonArray;
    } else {
        // La requête a échoué
        printf("Erreur lors de l'exécution de la requête : %s", PQerrorMessage(conn));

        // Libère le résultat de la requête
        PQclear(res);

        // Retourne une chaîne JSON vide
        return "[]";
    }
}

void Delete(PGconn *conn, char* id) {

    char query[100];
    snprintf(query, sizeof(query), "DELETE FROM ip_address WHERE id = '%s'", id);
    
    PGresult *res;
    res = PQexec(conn, query);
    
    if (PQresultStatus(res) == PGRES_COMMAND_OK) {
        printf("Suppression réussie.\n");
    } else {
        printf("Erreur lors de l'exécution de la requête : %s", PQerrorMessage(conn));
    }
    
    PQclear(res);
}


char* Filter(PGconn *conn, char* mask) {
    PGresult *filter;

    // Construct the SQL query with the provided mask value
    char query[11000]; // Adjust the size as needed
    snprintf(query, sizeof(query), "SELECT * FROM ip_address WHERE mask = '%s'", mask);

    // Execute the query
    filter = PQexec(conn, query);

    if (PQresultStatus(filter) == PGRES_TUPLES_OK) {
        int numRows = PQntuples(filter);

        // Allocate memory for jsonArray and initialize with an opening bracket
        char *jsonArray = malloc(200);
        strcpy(jsonArray, "[");

        for (int i = 0; i < numRows; i++) {
            DatabaseEntry entry;
            entry.id = PQgetvalue(filter, i, 0);
            entry.ipAddress = PQgetvalue(filter, i, 1);
            entry.masque = PQgetvalue(filter, i, 2);
            entry.ip_hexa = PQgetvalue(filter, i, 3);
            entry.ip_binary = PQgetvalue(filter, i, 4);

            char *jsonString = convertToJSON(entry);

            // Reallocate memory to increase the size of jsonArray and add jsonString
            jsonArray = realloc(jsonArray, strlen(jsonArray) + strlen(jsonString) + 2);
            strcat(jsonArray, jsonString);
            if (i < numRows - 1) {
                strcat(jsonArray, ",");
            }
            free(jsonString);
        }

        // Add the closing bracket at the end and free the query result
        strcat(jsonArray, "]");
        PQclear(filter);

        // Return the JSON array
        return jsonArray;
    } else {
        // The query failed
        printf("Erreur lors de l'exécution de la requête : %s", PQerrorMessage(conn));

        // Free the query result
        PQclear(filter);

        // Return an empty JSON array
        return "[]";
    }
}

PGconn* connectToDatabase() {
    const char *host = "localhost";
    const char *db_name = "ip_manager";
    const char *user = "zindar";
    const char *password = "zindar";

    PGconn *conn;
    conn = PQsetdbLogin(host, NULL, NULL, NULL, db_name, user, password);
    if (PQstatus(conn) == CONNECTION_BAD) {
        fprintf(stderr, "La connexion a échoué : %s\n", PQerrorMessage(conn));
        return NULL; 
    }
     if (PQstatus(conn) == CONNECTION_OK) {
    printf("Connexion a la BDD PGSQL établie.\n");


    }
    return conn; 
}




bool validate_number(char *str) {
   while (*str) {
      if(!isdigit(*str)){ //if the character is not a number, return
         return false;
      }
      str++; //point to next character
   }
   return true;
}
bool validate_ip(char *ip) { //check whether the IP is valid or not
   int i, num, dots = 0;
   char *ptr;
   if (ip == NULL)
      return false;
      ptr = strtok(ip, "."); //cut the string using dor delimiter
      if (ptr == NULL)
         return false;
   while (ptr) {
      if (!validate_number(ptr)) //check whether the sub string is
         return false;
         num = atoi(ptr); //convert substring to number
         if (num >= 0 && num <= 255) {
            ptr = strtok(NULL, "."); //cut the next part of the string
            if (ptr != NULL)
               dots++; //increase the dot count
         } else
            return false;
    }
    if (dots != 3) //if the number of dots are not 3, return false
       return false;
      return true;
}


int is_valid_ip(char *ip) {
    // Vérifie si l'IP est valide (omis pour simplifier l'exemple)
    return 1; // Retourne 1 si l'IP est valide, sinon 0
}

char* convert_to_hexadecimal(char *ip) {
    if (is_valid_ip(ip)) {
        unsigned int octet1, octet2, octet3, octet4;
        char *hex_ip = (char*)malloc(18); // Assez d'espace pour stocker l'adresse IP en hexadécimal (4 octets x 2 chiffres hexadécimaux + 3 séparateurs de points + caractère de fin de chaîne)
        sscanf(ip, "%u.%u.%u.%u", &octet1, &octet2, &octet3, &octet4);
        snprintf(hex_ip, 18, "%02X.%02X.%02X.%02X", octet1, octet2, octet3, octet4);
        return hex_ip;
    } else {
        return NULL; // Retourne NULL si l'IP n'est pas valide
    }
}

char* convert_to_binary(const char *ip) {
    char *binary_ip = malloc(35); // 4 octets * 8 bits + 3 points + 1 caractère nul
    if (binary_ip == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }

    int index = 0;
    int octet = 0;
    int isFirstOctet = 1; // Variable pour vérifier le premier octet
    for (int i = 0; i <= strlen(ip); i++) {
        if (ip[i] == '.' || ip[i] == '\0') {
            // Convertit l'octet en binaire et ajoute au résultat
            for (int j = 7; j >= 0; j--) {
                binary_ip[index++] = (octet & (1 << j)) ? '1' : '0';
            }
            if (isFirstOctet && index < 8) {
                // Remplit de zéros pour le premier octet
                for (int k = 0; k < 8 - index; k++) {
                    binary_ip[index++] = '0';
                }
            }
            if (index < 31) {
                binary_ip[index++] = '.'; // Ajoute un point entre les octets
            }
            isFirstOctet = 0; // Désactive la vérification du premier octet
            octet = 0; // Réinitialise l'octet pour le prochain groupe de chiffres
        } else {
            // Met à jour l'octet en cours de conversion
            octet = octet * 10 + (ip[i] - '0');
        }
    }
    binary_ip[index] = '\0'; // Ajoute le caractère nul à la fin de la chaîne

    return binary_ip;
}



char* processClientData(int client_socket, PGconn *conn) {
    char buffer[8024];
    while (1) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received == 0) {
            close(client_socket);
            break;
        } else {
        buffer[bytes_received] = '\0';
        printf("Donnée Recue du Client: %s\n", buffer);
        char *token;
        int id;
        char mask[20];
        char ip[16];
        char binaire[50];
        char hexadecimal[30];
        token = strtok(buffer, ": ");
    if (token != NULL) {
        token = strtok(NULL, ","); // Utilisez "," comme délimiteur pour séparer l'adresse IP du reste de la chaîne
        if (token != NULL) {
            strncpy(ip, token + 1, sizeof(ip)); // +1 pour ignorer l'espace après le délimiteur
        }

        token = strtok(NULL, "Masque: ");
         if (token != NULL) {
             strncpy(mask, token, sizeof(mask));
     }

     token = strtok(NULL, ", ID:");
          if (token != NULL) {
             id = strtol(token, NULL, 10);
     }
        printf("id: %d\n", id);
        printf("Adresse IP: %s\n", ip);
        printf("Masque: %s\n", mask);

    switch (id) {
        case 1:
            printf("Insertion\n");
                    char *hexadecimal = convert_to_hexadecimal(ip);
        char *binaire = convert_to_binary(ip);
        char insert_query[200];
        printf("IP en hexadécimal : %s\n", hexadecimal);
        printf("IP en binaire : %s\n", binaire);

        snprintf(insert_query, sizeof(insert_query), "INSERT INTO ip_address (decimal,mask,hexadecimal,binaire) VALUES ('%s','%s','%s','%s')", ip, mask, hexadecimal,binaire);
        printf("Tentative d'insertion : %s\n", insert_query);

        PGresult *insert = PQexec(conn, insert_query);
        if (PQresultStatus(insert) == PGRES_COMMAND_OK) {
            printf("Insertion réussie\n");
            char *Update = SelectAllFromTable(conn);
           
            return Update;


        } else {
            printf("Erreur lors de l'insertion : %s\n", PQresultErrorMessage(insert));
            return "[]";
        }

        

        PQclear(insert);

            break;
        case 2:
            printf("Filtrage\n");
            char *FilterResult = Filter(conn, mask);
           if (FilterResult != NULL) {
            printf("Result Filter : %s\n", FilterResult);
            return FilterResult;
            free(FilterResult);
        }
            else {
                printf("Error occurred while filtering data.\n");

            }
            break;
        case 3:
            printf("Delete\n");
            printf("ID DELETE IP : %s\n", mask);
            Delete(conn, mask);
            char *Update = SelectAllFromTable(conn);
           
            return Update;
            break;
        default:
            printf("ID non reconnu\n");
            return "[]";
            break;
    }

    } else {
        printf("Format de données invalide : %s\n", buffer);
        return "[]";
    }
        }

    }
}

void handleClientConnections(int server_socket, PGconn *conn, char *initialData) {
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char *data = initialData; 

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Erreur lors de l'acceptation de la connexion");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        printf("Client connecté\n");
        send(client_socket, data, strlen(data), 0);
        char *Update;
        Update = processClientData(client_socket, conn);
        if (Update != NULL && strlen(Update) > 0) {
            
            data = Update; 
            //printf("data Update: %s\n", data);
        } else {
            printf("Retrying send...\n");
        }
        send(client_socket, data, strlen(data), 0);

    }
}



int createServerSocket(int port) {
    int server_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Erreur lors du binding du socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {
        perror("Erreur lors de l'écoute du socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    return server_socket;
}


int main() {
    PGconn *conn = connectToDatabase();
    int server_socket;
    server_socket = createServerSocket(12345);
    printf("Serveur en attente de connexions...\n");
    char *jsonString = SelectAllFromTable(conn);
    // printf("PGSQL TABLE: %s\n", jsonString);
    handleClientConnections(server_socket, conn,jsonString);
    // PQfinish(conn);
    // close(server_socket);

    return 0;
}
