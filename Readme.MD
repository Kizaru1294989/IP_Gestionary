I - Run Manuellement

Terminal-1 :

    - cd C_Backend
    - gcc -o serveur main.c Format/IP/ip_validator.c Format/Convertion/convertion.c WebSocket/Create_Socket/createServerSocket.c Database/PGSQL/ORM.c -lpq

Terminal-2 :

    - cd PythonFrontend
    - python3 interface.py

II - Makefile

- Run "make"
- make run
- make clean ( clear tout les executables)