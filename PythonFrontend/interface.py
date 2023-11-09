import tkinter as tk
from tkinter import ttk
from tkinter import *
import socket
import threading
import re
import customtkinter




# win.mainloop()
class App(customtkinter.CTk):
    def __init__(self):

        super().__init__()
        self.title("IP Manager")
        self.geometry(f"{920}x{550}")
     
        #
        self.delete_button_clicked = False
        self.new_entry_ip_addresses = []

        self.id = []
        self.ip_addresses = []
        self.ip_addresses_binary = []
        self.ip_addresses_hexa = []
        self.mask = []

        self.delete_by_id = []
        self.fetch_list_ip = []

        self.ip_addresses = []
        self.mask_filter = []

        self.receive_data_from_server()
        
        ip_label = customtkinter.CTkLabel(self, text="Adresse IP:")
        ip_label.grid(row=0, column=0, padx=10, pady=10, sticky="w")

        self.ip_entry = customtkinter.CTkEntry(self, width=100)
        self.ip_entry.grid(row=0, column=1, padx=10, pady=10, sticky="w")

        mask_label = customtkinter.CTkLabel(self, text="Masque:")
        mask_label.grid(row=0, column=2, padx=10, pady=10, sticky="w")

        self.mask_entry = customtkinter.CTkEntry(self, width=100)
        self.mask_entry.grid(row=0, column=3, padx=10, pady=10, sticky="w")

        add_ip_button = customtkinter.CTkButton(self, text="Ajouter IP et Masque", command=self.add_ip_event)
        add_ip_button.grid(row=0, column=4, padx=10, pady=10, sticky="w")

        delete_ip_button = customtkinter.CTkButton(self, text="Supprimer IP" , command=self.delete_ip_event)
        delete_ip_button.grid(row=0, column=6, padx=10, pady=10, sticky="w")

        filter_button = customtkinter.CTkButton(self, text="Filtrer", command=self.filter_ip_event)
        filter_button.grid(row=0, column=8, padx=10, pady=10, sticky="w")

        fetch_list_ip_button = customtkinter.CTkButton(self, text="Liste IP" , command=self.fetch_list_ip_event)
        fetch_list_ip_button.grid(row=1, column=8, padx=10, pady=10, sticky="w")

        self.ip_listbox = ttk.Treeview(self, show='headings',column=( "#1" ,"#2", "#3", "#4", "#5"), height=15)
        self.ip_listbox.grid(row=10, column=1, columnspan=6, padx=10, pady=10, sticky="w")
        self.ip_listbox.column("#1", width=40, anchor=tk.CENTER)
        self.ip_listbox.heading("#1", text="ID")
        self.ip_listbox.column("#2", width=110, anchor=tk.CENTER)
        self.ip_listbox.heading("#2", text="IP Adresse")
        self.ip_listbox.column("#3",  width=120,anchor=tk.CENTER)
        self.ip_listbox.heading("#3", text="Héxadecimal")
        self.ip_listbox.column("#4",  width=200, anchor=tk.CENTER)
        self.ip_listbox.heading("#4", text="Binaire")
        self.ip_listbox.column("#5",  width=70, anchor=tk.CENTER)
        self.ip_listbox.heading("#5", text="Masque")
        
        self.error_label = customtkinter.CTkLabel(self, text="" , text_color="red")
        self.error_label.grid(row=2, column=3 , columnspan=6,padx=0, pady=0, sticky="w")

        self.update_ip_listbox()

    def fetch_list_ip_event(self):
       # self.ip_listbox.delete(*self.ip_listbox.get_children())
        self.fetch_list_ip = f"IP: list, "
        self.send_to_server_fetch()

    def send_to_server_fetch(self):
        if not self.fetch_list_ip:
            messagebox.showerror("Erreur", "La liste d'adresses IP est vide.")
        else:
            print("Demande de la list au serveur C")
            print(self.fetch_list_ip)
            # Ajouter l'ID au début de la chaîne de données
            data_with_id = self.fetch_list_ip + ' , ID: 4'
            threading.Thread(target=self.send_data_to_server, args=(data_with_id,)).start()
            self.ip_listbox.delete(*self.ip_listbox.get_children())

    def delete_ip_event(self):
        selected_item = self.ip_listbox.selection()
        if selected_item:
            self.delete_by_id = f"IP: delete, Masque: {self.ip_listbox.item(selected_item, 'values')[0]}"
            print(f"Selected IP's ID to delete: {self.delete_by_id}")
            self.send_to_server_delete()
        else:
            self.error_label.configure(text="Aucunes IP séléctionnés")

    def send_to_server_delete(self):
        if not self.delete_by_id:
            messagebox.showerror("Erreur", "La liste d'adresses IP est vide.")
        else:
            print("ENVOI D'UNE SUPPRESSION AU SERVEUR C")
            print(self.delete_by_id)
            # Ajouter l'ID au début de la chaîne de données
            data_with_id = self.delete_by_id + ' , ID: 3'
            threading.Thread(target=self.send_data_to_server, args=(data_with_id,)).start()
            self.ip_listbox.delete(*self.ip_listbox.get_children())





    
    def send_to_server(self):
        if not self.new_entry_ip_addresses:
            messagebox.showerror("Erreur", "La liste d'adresses IP est vide.")
        else:
            print("ENVOI D'UNE NOUVELLE IP AU SERVEUR C ")
            #print(self.new_entry_ip_addresses)
            data = ', '.join(self.new_entry_ip_addresses)
            data_with_id = data + ' , ID: 1'
            threading.Thread(target=self.send_data_to_server, args=(data_with_id,)).start()
            self.ip_listbox.delete(*self.ip_listbox.get_children())

    def send_to_server_filter(self):
        if not self.new_entry_ip_addresses:
            messagebox.showerror("Erreur", "La liste d'adresses IP est vide.")
        else:
            print("ENVOI D'UN FILTRE AU SERVEUR C")
            #print(self.new_entry_ip_addresses)
            data = ', '.join(self.new_entry_ip_addresses)
            # Ajouter l'ID au début de la chaîne de données
            data_with_id = data + ' , ID: 2'
            threading.Thread(target=self.send_data_to_server, args=(data_with_id,)).start()
            self.ip_listbox.delete(*self.ip_listbox.get_children())


    def add_ip_event(self):
        ip_address = self.ip_entry.get()
        mask = self.mask_entry.get() 
        
        if not ip_address or not mask:
            self.error_label.configure(text="Les champs IP et Masque ne peuvent pas être vides.")
        else:
            self.error_label.configure(text="")  # Réinitialise le message d'erreur
            self.new_entry_ip_addresses.append(f"IP: {ip_address}, Masque: {mask}")
            self.ip_entry.delete(0, tk.END)
            self.mask_entry.delete(0, tk.END)
            self.send_to_server()

    def filter_ip_event(self):
        ip_address = self.ip_entry.get()
        mask = self.mask_entry.get()

        
        if not ip_address or not mask:
            self.error_label.configure(text="Les champs IP et Masque de Filtre ne peuvent pas être vides.")
        else:
            self.error_label.configure(text="")  # Réinitialise le message d'erreur
            self.new_entry_ip_addresses.append(f"IP: {ip_address}, Masque: {mask}")
            self.send_to_server_filter()

    def update_data_from_server(self, new_data):
        self.id.clear()
        self.ip_addresses.clear()
        self.mask.clear()
        self.ip_addresses_binary.clear()
        self.ip_addresses_hexa.clear()
        for item in new_data:
            pattern = r'{ID:(\d+),IP Address:([\d.]+),Masque:(\/\d+),Binary IP:([01.]+),Hexadecimal IP:([0-9A-Fa-f.]+)}'
            matches = re.findall(pattern, item)
            for match in matches:
                #print(match)
                self.id.append(match[0])
                self.ip_addresses.append(match[1])
                self.mask.append(match[2])
                self.ip_addresses_binary.append(match[3])
                self.ip_addresses_hexa.append(match[4])

        self.update_ip_listbox()  # Mettez à jour l'affichage de la liste après avoir mis à jour les données

    def receive_data_from_server(self):
        try:
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_address = ('localhost', 12345)
            client_socket.connect(server_address)
            data = client_socket.recv(4096).decode()

            #print("data" + data)
     


            if data == "IP Format Invalide!":
                self.error_label.configure(text=data)
            else:
                formatted_data = data.strip("[]").replace('"', '').split("}, {")
                #print(data)
                self.update_data_from_server(formatted_data)
            #


                    

        except Exception as e:
            print(f"Erreur lors de la réception de données du serveur C : {e}")
        finally:
            client_socket.close()

    def update_ip_listbox(self):

        for i in range(min(len(self.id), len(self.ip_addresses), len(self.mask))):
            unique = self.id[i]
            ip_address = self.ip_addresses[i]
            masque = self.mask[i]
            binary = self.ip_addresses_binary[i]
            hexa = self.ip_addresses_hexa[i]

            self.ip_listbox.insert('', 'end', text=i, values=(unique, ip_address, hexa , binary , masque))



   

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
            messagebox.showerror("Erreur", f"Erreujr lors de l'envoi de données au serveur C : {e}")

        finally:
            sock.close()

if __name__ == "__main__":
    app = App()
    app.mainloop()
