#include <postgresql/libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include "ORM.h"

char* convertToJSON(DatabaseEntry entry) {
    const char *jsonTemplate = "{\"ID\":\"%s\",\"IP Address\":\"%s\",\"Masque\":\"%s\",\"Binary IP\":\"%s\",\"Hexadecimal IP\":\"%s\"}";
    int jsonSize = snprintf(NULL, 0, jsonTemplate, entry.id, entry.ipAddress, entry.masque, entry.ip_binary, entry.ip_hexa) + 1;
    char *jsonString = (char*)malloc(jsonSize);
    sprintf(jsonString, jsonTemplate, entry.id, entry.ipAddress, entry.masque, entry.ip_binary, entry.ip_hexa);
    return jsonString;
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

char* SelectAllFromTable(PGconn *conn) {
    PGresult *res;
    res = PQexec(conn, "SELECT * FROM ip_address");
    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        int numRows = PQntuples(res);
        char *jsonArray = (char*)malloc(2);
        strcpy(jsonArray, "[");
        for (int i = 0; i < numRows; i++) {
            DatabaseEntry entry;
            entry.id = PQgetvalue(res, i, 0);
            entry.ipAddress = PQgetvalue(res, i, 1);
            entry.masque = PQgetvalue(res, i, 2);
            entry.ip_hexa = PQgetvalue(res, i, 3);
            entry.ip_binary = PQgetvalue(res, i, 4);
            char *jsonString = convertToJSON(entry);
            jsonArray = (char*)realloc(jsonArray, strlen(jsonArray) + strlen(jsonString) + 2);
            strcat(jsonArray, jsonString);
            if (i < numRows - 1) {
                strcat(jsonArray, ",");
            }
            free(jsonString);
        }
        strcat(jsonArray, "]");
        PQclear(res);
        return jsonArray;
    } else {
        printf("Erreur lors de l'exécution de la requête : %s", PQerrorMessage(conn));
        PQclear(res);
        char *result = (char*)malloc(3); 
        strcpy(result, "[]");
        return result;
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
    char query[11000]; 
    snprintf(query, sizeof(query), "SELECT * FROM ip_address WHERE mask = '%s'", mask);
    filter = PQexec(conn, query);
    if (PQresultStatus(filter) == PGRES_TUPLES_OK) {
        int numRows = PQntuples(filter);
        char *jsonArray = (char*)malloc(200);
        strcpy(jsonArray, "[");
        for (int i = 0; i < numRows; i++) {
            DatabaseEntry entry;
            entry.id = PQgetvalue(filter, i, 0);
            entry.ipAddress = PQgetvalue(filter, i, 1);
            entry.masque = PQgetvalue(filter, i, 2);
            entry.ip_hexa = PQgetvalue(filter, i, 3);
            entry.ip_binary = PQgetvalue(filter, i, 4);
            char *jsonString = convertToJSON(entry);
            jsonArray = (char*)realloc(jsonArray, strlen(jsonArray) + strlen(jsonString) + 2);
            strcat(jsonArray, jsonString);
            if (i < numRows - 1) {
                strcat(jsonArray, ",");
            }
            free(jsonString);
        }
        strcat(jsonArray, "]");
        PQclear(filter);
        return jsonArray;
    } else {
        printf("Erreur lors de l'exécution de la requête : %s", PQerrorMessage(conn));
        PQclear(filter);
        char *result = (char*)malloc(3); 
        strcpy(result, "[]");
        return result;
    }
}