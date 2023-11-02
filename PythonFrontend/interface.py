import tkinter as tk
from tkinter import ttk
from tkinter import *
import socket
import time
import threading
import re
import os
import matplotlib
import ipaddress as ip


# win.mainloop()
class App(tk.Tk):
    def __init__(self):

        super().__init__()
        self.title("IP Manager")
        self.geometry(f"{830}x{550}")



        self.new_entry_ip_addresses = []
        self.id = []
        self.ip_addresses = []
        self.mask = []
        self.receive_data_from_server()
        

     # Texte explicatif pour l'adresse IP
        ip_label = tk.Label(self, text="Adresse IP:")
        ip_label.grid(row=0, column=0, padx=10, pady=10, sticky="w")

# Entrée pour l'adresse IP
        self.ip_entry = tk.Entry(self, width=15)
        self.ip_entry.grid(row=0, column=1, padx=10, pady=10, sticky="w")

# Texte explicatif pour le masque
        mask_label = tk.Label(self, text="Masque:")
        mask_label.grid(row=0, column=2, padx=10, pady=10, sticky="w")

# Entrée pour le masque
        self.mask_entry = tk.Entry(self, width=15)
        self.mask_entry.grid(row=0, column=3, padx=10, pady=10, sticky="w")


# Bouton pour ajouter l'adresse IP et le masque à la liste
        add_ip_button = tk.Button(self, text="Ajouter IP et Masque", command=self.add_ip_event)
        add_ip_button.grid(row=0, column=4, padx=10, pady=10, sticky="w")

# Bouton pour supprimer une adresse IP et son masque de la liste
        delete_ip_button = tk.Button(self, text="Supprimer IP")
        delete_ip_button.grid(row=0, column=5, padx=10, pady=10, sticky="w")

# Liste des adresses IP et masques
        self.ip_listbox = ttk.Treeview(self, show='headings',column=("#1", "#2", "#3"), height=15)
        self.ip_listbox.grid(row=10, column=0, columnspan=6, padx=10, pady=10, sticky="w")
        self.ip_listbox.column("#1", anchor=tk.CENTER)
        self.ip_listbox.heading("#1", text="ID")
        self.ip_listbox.column("#2", anchor=tk.CENTER)
        self.ip_listbox.heading("#2", text="IP Adresse")
        self.ip_listbox.column("#3", anchor=tk.CENTER)
        self.ip_listbox.heading("#3", text="Masque CIDR")

        # send_to_server_button = tk.Button(self, text="Envoyer au serveur C", command=self.send_to_server)
        # send_to_server_button.grid(row=2, column=0, columnspan=6, padx=10, pady=10, sticky="w")

            # Label pour afficher les messages d'erreur
        self.error_label = tk.Label(self, text="", fg="red")
        self.error_label.grid(row=2, column=3 , columnspan=6,padx=0, pady=0, sticky="w")

        self.update_ip_listbox()

    def send_to_server(self):
        if not self.new_entry_ip_addresses:
            messagebox.showerror("Erreur", "La liste d'adresses IP est vide.")
        else:
            print("ENVOI AU SERVEUR C ")
            #print(self.new_entry_ip_addresses)
            data = ', '.join(self.new_entry_ip_addresses)
            threading.Thread(target=self.send_data_to_server, args=(data,)).start()
            self.ip_listbox.delete(*self.ip_listbox.get_children())


    def add_ip_event(self):
        ip_address = self.ip_entry.get()
        mask = self.mask_entry.get()

        
        if not ip_address or not mask:
            self.error_label.config(text="Les champs IP et Masque ne peuvent pas être vides.")
        else:
            self.error_label.config(text="")  # Réinitialise le message d'erreur
            self.new_entry_ip_addresses.append(f"IP: {ip_address}, Masque: {mask}")
            self.ip_entry.delete(0, tk.END)
            self.mask_entry.delete(0, tk.END)
            self.send_to_server()



    def format_ip_reception(self):
        print('format')



    def update_data_from_server(self, new_data):
        self.id.clear()
        self.ip_addresses.clear()
        self.mask.clear()


        for item in new_data:
            pattern = r'{ID:(\d+),IP Address:([\d.]+),Masque:(\/\d+)}'
            matches = re.findall(pattern, item)
            for match in matches:
                self.id.append(match[0])
                self.ip_addresses.append(match[1])
                self.mask.append(match[2])

        self.update_ip_listbox()  # Mettez à jour l'affichage de la liste après avoir mis à jour les données

    def receive_data_from_server(self):
        try:
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_address = ('localhost', 12345)
            client_socket.connect(server_address)
            data = client_socket.recv(4096).decode()
            # print("data" + data)
            formatted_data = data.strip("[]").replace('"', '').split("}, {")
            self.update_data_from_server(formatted_data)  # Mettez à jour les données et l'affichage de la liste

        except Exception as e:
            print(f"Erreur lors de la réception de données du serveur C : {e}")
        finally:
            client_socket.close()

    def update_ip_listbox(self):

        for i in range(min(len(self.id), len(self.ip_addresses), len(self.mask))):
            unique = self.id[i]
            print(unique)
            ip_address = self.ip_addresses[i]
            masque = self.mask[i]

            self.ip_listbox.insert('', 'end', text=i, values=(unique, ip_address, masque))



   

    def send_data_to_server(self, data):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_address = ('localhost', 12345)
            sock.connect(server_address)
            sock.sendall(data.encode())
            print(f"Données envoyées au serveur C: {data}")
            self.new_entry_ip_addresses = []
            self.receive_data_from_server()
        except Exception as e:
            messagebox.showerror("Erreur", f"Erreur lors de l'envoi de données au serveur C : {e}")

        finally:
            sock.close()

if __name__ == "__main__":
    app = App()
    app.mainloop()
