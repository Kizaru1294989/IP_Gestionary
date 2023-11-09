CC=gcc
CFLAGS=-Wall -Wextra -g
LDFLAGS=-lpq

C_BACKEND_DIR=C_Backend
PYTHON_FRONTEND_DIR=PythonFrontend

.PHONY: all clean run

all: serveur

serveur: $(C_BACKEND_DIR)/main.o $(C_BACKEND_DIR)/ip_validator.o $(C_BACKEND_DIR)/convertion.o $(C_BACKEND_DIR)/createServerSocket.o $(C_BACKEND_DIR)/ORM.o
	$(CC) $(CFLAGS) -o serveur $(C_BACKEND_DIR)/main.o $(C_BACKEND_DIR)/ip_validator.o $(C_BACKEND_DIR)/convertion.o $(C_BACKEND_DIR)/createServerSocket.o $(C_BACKEND_DIR)/ORM.o $(LDFLAGS)

$(C_BACKEND_DIR)/main.o: $(C_BACKEND_DIR)/main.c
	$(CC) $(CFLAGS) -c $(C_BACKEND_DIR)/main.c -o $(C_BACKEND_DIR)/main.o

$(C_BACKEND_DIR)/ip_validator.o: $(C_BACKEND_DIR)/Format/IP/ip_validator.c
	$(CC) $(CFLAGS) -c $(C_BACKEND_DIR)/Format/IP/ip_validator.c -o $(C_BACKEND_DIR)/ip_validator.o

$(C_BACKEND_DIR)/convertion.o: $(C_BACKEND_DIR)/Format/Convertion/convertion.c
	$(CC) $(CFLAGS) -c $(C_BACKEND_DIR)/Format/Convertion/convertion.c -o $(C_BACKEND_DIR)/convertion.o

$(C_BACKEND_DIR)/createServerSocket.o: $(C_BACKEND_DIR)/WebSocket/Create_Socket/createServerSocket.c
	$(CC) $(CFLAGS) -c $(C_BACKEND_DIR)/WebSocket/Create_Socket/createServerSocket.c -o $(C_BACKEND_DIR)/createServerSocket.o

$(C_BACKEND_DIR)/ORM.o: $(C_BACKEND_DIR)/Database/PGSQL/ORM.c
	$(CC) $(CFLAGS) -c $(C_BACKEND_DIR)/Database/PGSQL/ORM.c -o $(C_BACKEND_DIR)/ORM.o

run: serveur
	./serveur &
	 
	cd $(PYTHON_FRONTEND_DIR) && export DISPLAY=:0.0 && python3 interface.py

clean:
	rm -f serveur $(C_BACKEND_DIR)/*.o
