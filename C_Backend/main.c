#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <postgresql/libpq-fe.h>
#include <libwebsockets.h>
#include "./Format/IP/ip_validator.h"
#include "./Format/Convertion/convertion.h"
#include "./WebSocket/Create_Socket/createServerSocket.h"
#include "./Database/PGSQL/ORM.h"

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
        token = strtok(NULL, ","); 
        if (token != NULL) {
            strncpy(ip, token + 1, sizeof(ip)); 
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
            if(validate_ip(ip)){
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
            } else  {
                printf("Erreur lors de l'insertion : %s\n", PQresultErrorMessage(insert));
                // printf("IP Format Invalide ! : %s\n" ,ip);
                // char* errorMessage = "IP Format Invalide!";
                // return errorMessage;
            }

            } else {
            printf("IP Format Invalide ! : %s\n" ,ip);
            char* errorMessage = "IP Format Invalide!";
            return errorMessage;
            }           
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
        case 4:
            printf("LIST IP \n");
            char *List = SelectAllFromTable(conn);
            return List;
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
        } else {
            printf("Retrying send...\n");
        }
        send(client_socket, data, strlen(data), 0);
    }
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