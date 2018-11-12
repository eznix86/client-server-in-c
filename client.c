#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "document.h"
#include "login.h"
void menu();

size_t serialize(const DOCUMENT* patient, char* buf)
{
size_t bytes = 0;

memcpy(buf + bytes, patient->name, strlen(patient->name) + 1);
bytes += strlen(patient->name) + 1;

memcpy(buf + bytes, patient->surname, strlen(patient->surname) + 1);
bytes += strlen(patient->surname) + 1;

memcpy(buf + bytes, patient->city, strlen(patient->city) + 1);
bytes += strlen(patient->city) + 1;

memcpy(buf + bytes, patient->birth_year, strlen(patient->birth_year) + 1);
bytes += strlen(patient->birth_year) + 1;

return bytes;
}

void deserialize(const char* buf, DOCUMENT* patient)
{
size_t offset = 0;

strcpy(patient->name, (buf + offset));
offset += strlen(buf + offset) + 1;

strcpy(patient->surname, (buf + offset));
offset += strlen(buf + offset) + 1;

strcpy(patient->city, (buf + offset));
offset += strlen(buf + offset) + 1;

strcpy(patient->birth_year, (buf + offset));
}


void error(char *message){
    perror(message);
    exit(1);
}

void clearScreen(){
    system("cls");
    system("clear");
}

void removeNewLineStruct( DOCUMENT *patient){
    int len = 0;
    len=strlen(patient->name);
    if(patient->name[len-1]=='\n')
        patient->name[len-1]='\0';
    
    len=strlen(patient->surname);
    if(patient->surname[len-1]=='\n')
        patient->surname[len-1]='\0';
    
    len=strlen(patient->city);
    if(patient->city[len-1]=='\n')
        patient->city[len-1]='\0';
    
    len=strlen(patient->birth_year);
    if(patient->birth_year[len-1]=='\n')
        patient->birth_year[len-1]='\0';

}


int displayPatient(int socket){
   clearScreen();

    int found, buffer_handler, i = 0;
    char buffer[1024] = {0};
    char n[80];

    
    printf("****************************[LISTE PATIENTS]************************\n");
    printf("%s\t%10s\t%10s\t%10s\t%10s\n", "ID","PRENOM","NOM","ANNEE","VILLE");
    buffer_handler = read(socket,  &n, sizeof(n));
    found = atoi(n);
    while( found && i < found){
        buffer_handler = read(socket, buffer, sizeof(buffer));
        if(buffer_handler < 0 ) error("Error! Unable to read from socket");

        DOCUMENT tmp;
        deserialize(buffer, &tmp);
        printf("%d\t%10s\t%10s\t%10s\t%10s\n", i+1, tmp.name, tmp.surname, tmp.birth_year, tmp.city);

        buffer_handler = write(socket, "OK", 2 );
        if(buffer_handler < 0) {break; error("Error! Unable to write to socket");}

        i ++;        
    }

    printf("\n%d patient(s) trouver\n", found);
    printf("********************************************************************\n");

    
    return found;
}

void insertPatient(int socket){

    DOCUMENT patient;
    char buff[256], choix[50];
    clearScreen();
    printf("***************************[AJOUTER PATIENT]************************\n");
    printf("1. Nom: ");
	fgets( patient.surname, sizeof(patient.surname), stdin);
	printf("2. Prenom: ");
	fgets(patient.name, sizeof(patient.name), stdin);
	printf("3. Annee de Naissance: ");
	fgets(patient.birth_year, sizeof(patient.birth_year), stdin);
	printf("4. Ville: ");
	fgets(patient.city, sizeof(patient.city), stdin);
    printf("********************************************************************\n");
    removeNewLineStruct(&patient);
    do{
        printf("\nVoulez-vous vraiment sauvegarder le patient: %s %s nee en %s, habitant de %s\n\n",patient.surname, patient.name, patient.birth_year, patient.city);
        printf("Tappez y (Oui) ou n (Non), votre choix: ");
        fgets(choix, 50, stdin);
    }while(!(strcmp(choix, "y\n") == 0 || strcmp(choix, "n\n") == 0) );

    if((strcmp(choix, "y\n") == 0 )){
        char received[2];
        int buffer_handler;
        char bufferString[1024] = {0};
        size_t bufLen = serialize(&patient, bufferString);

        buffer_handler = write(socket, &bufferString, bufLen);

        if(buffer_handler < 0) error ("Failed  to write to socket");
    
        buffer_handler = read(socket, &received, sizeof(received) - 1);

        if(buffer_handler < 0 ) error ("Failed to read from socket");

        printf("\nPatient sauvegarder\n");
    }else {
        printf("\nPatient non sauvegarder\n");
       
    }
    fflush(stdout);
    sleep(2);
}


void removePatient(int socket){
    char choice[50];
    int line = 0;
    do{
        write(socket, "4", 1);
        line =  displayPatient(socket);    
        printf(line > 0 ? "Pour effacer un patient:\n": "");
        printf(line > 0 ? "Choissez un numero de ligne [1 - %d]:\n" : "", line);
        printf("Tappez le chiffre zero pour quitter\n");
        printf("Votre choix: ");
        fgets(choice, sizeof(choice), stdin);
    }while((atoi(choice) < 0 || atoi(choice) > line) && write(socket, "0", 1) &&(printf("Mauvaise selection, Veuillez reessayez!\n")) && sleep(2));

    if((strcmp(choice, "0\n") == 0 ) || line == 0){
        write(socket, "0", 1);
        return;
    }else {
        char str[80];
        sprintf(str, "%d", atoi(choice));
        write(socket, &str, sizeof(str));
        printf("\nPatient a la ligne %d effacer\n", atoi(choice));
        sleep(2);
    }

}


void modifyPatient(int socket){
    char choice[50];
    int line = 0;
    do{
        write(socket, "3", 1);
        line =  displayPatient(socket);    
        printf(line > 0 ? "Pour effacer un patient:\n": "");
        printf(line > 0 ? "Choissez un numero de ligne [1 - %d]:\n" : "", line);
        printf("Tappez le chiffre zero pour quitter\n");
        printf("Votre choix: ");
        fgets(choice, sizeof(choice), stdin);
    }while((atoi(choice) < 0 || atoi(choice) > line) && write(socket, "0", 1) &&(printf("Mauvaise selection, Veuillez reessayez!\n")) && sleep(2));

    if((strcmp(choice, "0\n") == 0 ) || line == 0){
        write(socket, "0", 1);
        return;
    }else {
        char str[80];
        sprintf(str, "%d", atoi(choice));
        write(socket, &str, sizeof(str));
        DOCUMENT buffer;
        printf("\n**********************[MODIFIER PATIENT]***************************");
        printf("\n\n1. Nouveau Nom: ");
        fgets( buffer.surname, sizeof(buffer.surname), stdin);
        printf("2. Nouveau Prenom: ");
        fgets(buffer.name, sizeof(buffer.name), stdin);
        printf("3. Nouvel Annee de Naissance: ");
        fgets(buffer.birth_year, sizeof(buffer.birth_year), stdin);
        printf("4. Nouvel Ville: ");
        fgets(buffer.city, sizeof(buffer.city), stdin);
        printf("********************************************************************\n");
        char bufferString[1024] = {0};
        removeNewLineStruct(&buffer);
        size_t bufLen = serialize(&buffer, bufferString);

        write(socket, &bufferString, bufLen);

        printf("\nPatient a la ligne %d modifier\n", atoi(choice));
        sleep(2);
    }

}
void recherchePatient(int socket){

    DOCUMENT patient;
    char searchTerms[50], choix[10];
    int found = 0;
    int buffer_handler, i = 0;
    char buffer[1024] = {0};
    char n[80];

    clearScreen();
    printf("***********************[RECHERCHE PATIENT]***************************\n");
    printf("Veuillez inserer un mot clef: ");
    fgets(searchTerms, sizeof(searchTerms), stdin);
    removeNewLine(searchTerms);
    printf("\nMot clef inserer: %s\n", searchTerms);

    buffer_handler = write(socket, &searchTerms, sizeof(searchTerms) );
    if(buffer_handler < 0) error("Error! Unable to write to socket");
    printf("Recherche de votre mot clef dans la base de donnee.... ");
    buffer_handler = read(socket,  &n, sizeof(n));
    found = atoi(n);
    printf("Trouver: %d\n", found);
    if (found)  clearScreen();
    printf("\n*****************[RESULTAT DE LA RECHERCHE]*************************\n");
    printf("%s\t%10s\t%10s\t%10s\t%10s\n", "ID","PRENOM","NOM","ANNEE","VILLE");
    

    while( found && i < found){
        buffer_handler = read(socket, buffer, sizeof(buffer));
        if(buffer_handler < 0 ) error("Error! Unable to read from socket");

        DOCUMENT tmp;
        deserialize(buffer, &tmp);
        printf("%d\t%10s\t%10s\t%10s\t%10s\n", i+1, tmp.name, tmp.surname, tmp.birth_year, tmp.city);

        buffer_handler = write(socket, "OK", 2 );
        if(buffer_handler < 0) {break; error("Error! Unable to write to socket");}

        i ++;        
    }
    printf("********************************************************************\n");

    

    do{
        if (! found ) break;
        printf("\nVoulez-vous,\n");
        printf("1. Modifier un Patient\n");
        printf("2. Effacer un Patient\n");
        printf("3. Revenir a l'ecran principal\n");
        printf("Votre choix: ");
        fgets(choix, 10, stdin);
    }while(!(strcmp(choix, "1\n") == 0 || strcmp(choix, "2\n") == 0 || strcmp(choix, "3\n") == 0)  );

    if((strcmp(choix, "1\n") == 0 )){
         buffer_handler = write(socket, "1", 2 );
        if(buffer_handler < 0)  error("Error! Unable to write to socket");
        do{
            printf(found > 0 ? "Pour Modifier un patient:\n": "");
            printf(found > 0 ? "Choissez un numero de ligne [1 - %d]:\n" : "", found);
            printf("Tappez le chiffre zero pour quitter\n");
            printf("Votre choix: ");
            fgets(choix, sizeof(choix), stdin);
            removeNewLine(choix);
        }while((atoi(choix) < 0 || atoi(choix) > found)  &&(printf("Mauvaise selection, Veuillez reessayez!\n")) && sleep(2));

        if(atoi(choix) == 0){
            write(socket, "0", 1);
            return ;
        }else {
            DOCUMENT tmp;
            buffer_handler = write(socket, &choix, sizeof(choix) );
            if(buffer_handler < 0)  error("Error! Unable to write to socket");
            char tmpLine[500] = {0};
            read(socket, tmpLine, sizeof(tmpLine));
            deserialize(tmpLine, &tmp);

            do{
                printf("\n1. Modifier Nom seulement\n");
                printf("2. Modifier Prenom seulement\n");
                printf("3. Modifier Annee de Naissance seulement\n");
                printf("4. Modifier Ville seulement\n");
                printf("5. Modifier tous les informations\n");
                printf("Votre choix: ");
                fgets(choix, sizeof(choix), stdin);
                removeNewLine(choix);
            }while((atoi(choix) < 0 || atoi(choix) > 5)  &&(printf("Mauvaise selection, Veuillez reessayez!\n")) && sleep(2));

            if(atoi(choix) == 1){
                printf("Nom: ");
	            fgets(tmp.surname, sizeof(tmp.name), stdin);
            }else if(atoi(choix) == 2){
                printf("Prenom: ");
	            fgets(tmp.name, sizeof(tmp.surname), stdin);
            }else if(atoi(choix) == 3){
                printf("Annee de Naissance: ");
	            fgets(tmp.birth_year, sizeof(tmp.birth_year), stdin);
            }else if(atoi(choix) == 4){
                printf("Ville: ");
	            fgets(tmp.city, sizeof(tmp.city), stdin);
            }else if(atoi(choix) == 5){
                printf("1. Nom: ");
                fgets( tmp.surname, sizeof(tmp.surname), stdin);
                printf("2. Prenom: ");
                fgets(tmp.name, sizeof(tmp.name), stdin);
                printf("3. Annee de Naissance: ");
                fgets(tmp.birth_year, sizeof(tmp.birth_year), stdin);
                printf("4. Ville: ");
                fgets(tmp.city, sizeof(tmp.city), stdin);
                
            }
            removeNewLineStruct(&tmp);
            bzero(tmpLine, sizeof(tmpLine));
            size_t len = serialize(&tmp,tmpLine);
            write(socket, &tmpLine, len);
            printf("Patient Modifier !");
            fflush(stdout);
            sleep(2);
        }
      
    }else if((strcmp(choix, "3\n") == 0 )){
        buffer_handler = write(socket, "0", 2 );

    }else if((strcmp(choix, "2\n") == 0 )){
        buffer_handler = write(socket, "2", 2 );
        if(buffer_handler < 0)  error("Error! Unable to write to socket");
        do{
            printf(found > 0 ? "Pour effacer un patient:\n": "");
            printf(found > 0 ? "Choissez un numero de ligne [1 - %d]:\n" : "", found);
            printf("Tappez le chiffre zero pour quitter\n");
            printf("Votre choix: ");
            fgets(choix, sizeof(choix), stdin);
            removeNewLine(choix);
        }while((atoi(choix) < 0 || atoi(choix) > found)  &&(printf("Mauvaise selection, Veuillez reessayez!\n")) && sleep(2));

        if(atoi(choix) == 0){
            write(socket, "0", 1);
            return ;
        }else {
            buffer_handler = write(socket, &choix, sizeof(choix) );
            if(buffer_handler < 0)  error("Error! Unable to write to socket");
        }

       printf("Patient effacer !");
  
    }else if (!found){
        printf("\nAppuyez sur ENTER pour quitter.\n");
        getchar();      
    }
    fflush(stdout);
    sleep(2);
}


void menu(int socket){


    char choice[50];
    do{
        
        clearScreen();
        printf("*************************[GESTION PATIENT]***************************\n");
        printf("1. Ajouter Patient\n");
        printf("2. Recherche Patient\n");
        printf("3. Modifier Patient\n");
        printf("4. Effacer Patient\n");
        printf("5. Lister tous les Patient\n");
        printf("6. Quitter\n");
        printf("********************************************************************\n");    
        printf("Votre choix: ");
        fgets(choice, sizeof(choice), stdin);
    }while((atoi(choice) < 1 || atoi(choice) > 6) && (printf("Mauvaise selection, Veuillez reessayez!\n")) && sleep(2));

    switch(atoi(choice)){
        case 1:
            write(socket, "1", 1);
            insertPatient(socket);
            break;
        case 2:
            write(socket, "2", 1);
            recherchePatient(socket);
            break;
        case 3:
            modifyPatient(socket);
            break;
        case 4:
            removePatient(socket);
            break;
        case 5:
            write(socket, "5", 1);
            displayPatient(socket);
            printf("\nAppuyez sur la touche ENTER pour quitter\n");
            getchar();
            break;
        case 6:
            clearScreen();
            printf("Quitting...\n");
            exit(0);
            break;
        
    }
    menu(socket);
}


int main(int argc, char *argv[]){
    int available_socket, port_no, buffer_handler;
    struct sockaddr_in server_address;
    struct hostent *server;

   
    
    if (argc < 3){
        printf("Usage: %10s HOSTNAME PORT\n\n", argv[0]);
        exit(0);
    }

    port_no = atoi(argv[2]);

    available_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (available_socket < 0){
        error("Error! Unable to open socket");
    }
    
    server = gethostbyname(argv[1]);

    if (server == NULL){
        printf("No such host available\n\n");
        exit(0);
    }

    bzero((char *) &server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_no);
    bcopy((char *) server->h_addr, (char *) &server_address.sin_addr.s_addr, server->h_length);
    
    if(connect(available_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
        error("Fail to connect to server");
    login();
    menu(available_socket);
    return 0;

}



