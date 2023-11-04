#ifndef ORM_H
#define ORM_H

#include <postgresql/libpq-fe.h>

typedef struct {
    char *id;
    char *ipAddress;
    char *masque;
    char *ip_binary;
    char *ip_hexa;
} DatabaseEntry;

PGconn* connectToDatabase();
char* SelectAllFromTable(PGconn *conn);
char* convertToJSON(DatabaseEntry entry);
void Delete(PGconn *conn, char* id);
char* Filter(PGconn *conn, char* mask) ;

#endif // !ORM_H