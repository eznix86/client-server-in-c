ifdef PORT
	SERVER=$(PORT)
endif
compile: serveur client
	gcc serveur.c -o serveur
	gcc client.c -o client
	./serveur $(SERVER)