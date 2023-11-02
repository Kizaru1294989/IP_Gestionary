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
} DatabaseEntry;



char* convertToJSON(DatabaseEntry entry) {
    char *jsonTemplate = "{\"ID\":\"%s\",\"IP Address\":\"%s\",\"Masque\":\"%s\"}";
    char *jsonString = malloc(snprintf(NULL, 0, jsonTemplate, entry.id, entry.ipAddress, entry.masque) + 1);
    sprintf(jsonString, jsonTemplate, entry.id, entry.ipAddress, entry.masque);
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



char* processClientData(int client_socket, PGconn *conn) {
    char buffer[8024];
    while (1) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received == 0) {
            close(client_socket);
            break;
        } else {
        buffer[bytes_received] = '\0';
        char *token;
        char mask[50];
        char ip[16];
        token = strtok(buffer, ": ");
    if (token != NULL) {
        token = strtok(NULL, ","); // Utilisez "," comme délimiteur pour séparer l'adresse IP du reste de la chaîne
        if (token != NULL) {
            strncpy(ip, token + 1, sizeof(ip)); // +1 pour ignorer l'espace après le délimiteur
        }

        // Chercher le jeton contenant le masque
        token = strtok(NULL, "Masque: ");
         if (token != NULL) {
             strncpy(mask, token, sizeof(mask));
     }
  // Libérez la mémoire de l'ancienne data
        
        printf("Adresse IP: %s\n", ip);
        printf("Masque: %s\n", mask);
        char insert_query[100];
        snprintf(insert_query, sizeof(insert_query), "INSERT INTO ip_address (ip,mask) VALUES ('%s','%s')", ip, mask);
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
    char *data = initialData; // Initialisez data avec les données initiales

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Erreur lors de l'acceptation de la connexion");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        printf("Client connecté\n");

        // Envoyer les données initiales au client lors de la première connexion
        send(client_socket, data, strlen(data), 0);

        char *Update;
        Update = processClientData(client_socket, conn);
        if (Update != NULL && strlen(Update) > 0) {
            
            data = Update; // Mettre à jour data avec les nouvelles données
            printf("data: %s\n", data);
        } else {
            printf("Retrying send...\n");
        }

        // Envoyer les nouvelles données au client
        send(client_socket, data, strlen(data), 0);

        // Libérer la mémoire allouée pour Update si nécessaire
        // free(Update);
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
    printf("PGSQL TABLE: %s\n", jsonString);
    handleClientConnections(server_socket, conn,jsonString);
    // PQfinish(conn);
    // close(server_socket);

    return 0;
}
