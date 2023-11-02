import socket
import time

# Adresse et port du serveur C
server_address = ('localhost', 12345)

# Crée un socket TCP/IP
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

while True:
    try:
        # Tente de se connecter au serveur
        sock.connect(server_address)

        while True:
            # Saisie utilisateur
            user_input = input("Entrez une donnée à envoyer au serveur C: ")

            # Envoie les données au serveur C
            sock.sendall(user_input.encode())

            # Attends avant de renvoyer les données
            time.sleep(1)  # Attendez 1 seconde (ajustez selon vos besoins)
    except Exception as e:
        print(f"Erreur de connexion: {e}")
        time.sleep(2)  # Attendez 2 secondes avant de réessayer la connexion
