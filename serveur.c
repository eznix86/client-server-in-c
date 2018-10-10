/*
Titre: Serveur pour gestion patient
Auteur: Bernard Joseph Jean Bruno
Date: 28/08/2018
Version: 4.0
*/

//header pour necessaire pour le serveur 
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> //pour les types d'appels système
#include <sys/socket.h> // contient les structure pour faire un socket
#include <netinet/in.h> // contient les structure pour creer une adresse domaine
#include "port.h"
#include <string.h>
#include <unistd.h>
#include <signal.h>



#define DEFAULT_SERVER_PORT 8084

//header pour gestion patient
#include "document.h"
#define DATA_FILE "data.bin"
#define TEMP_FILE "temp.bin"

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

int displayData(int socket)
{
	FILE *checkExist, *ptrToFile;

	struct document patient;

	checkExist = fopen(DATA_FILE, "r"); //check if file exist

	if (checkExist)
	{	//if exists
		ptrToFile = fopen(DATA_FILE, "rb");

		fseek(ptrToFile, sizeof(struct document), SEEK_END);		 //get to the end of the file
		int lineNumber = ftell(ptrToFile) / sizeof(struct document); //give the cursor byte size offset divided by the document size.
        printf("\n\nno of line %d\n\n", lineNumber -1 );
         char str[80];

        sprintf(str, "%d", lineNumber - 1);
		if (lineNumber == 1)
		{
			printf("\n\nNo patient recorded\n\n");
			write(socket, &str, sizeof(str));
            return 0;
		}else {
           	write(socket, &str, sizeof(str));
        }

		rewind(ptrToFile);

		int found = 0;
		printf("\n***********LIST OF PATIENTS *************\n");
		while (fread(&patient, sizeof(struct document), 1, ptrToFile))
		{

            char received[2];
            int buffer_handler;
            char bufferString[1024] = {0};
            size_t bufLen = serialize(&patient, bufferString);

            buffer_handler = write(socket, &bufferString, bufLen);
            if(buffer_handler < 0) error ("Failed  to write to socket");
        
            buffer_handler = read(socket, &received, sizeof(received));

            if(buffer_handler < 0 ) error ("Failed to read from socket");
			printf("%d\t%s\t%s\t%s\t%s\n", found+ 1,patient.name, patient.surname, patient.birth_year, patient.city);
			found++;
		}
		printf("\nScan completed: %d patient(s) found\n", found);
		fclose(ptrToFile);
		return 0;
	}
	else
	{
		printf("\nError reading document; file %s not found\n\n", DATA_FILE);
	}
}



//search into file sequentially
int searchFile(int socket)
{
	
    char searchTerms[50];
	int buffer_handler;
	FILE *checkExist, *ptrToFile;

	struct document patient;

	checkExist = fopen(DATA_FILE, "r");

	buffer_handler = read(socket, searchTerms, sizeof(searchTerms));
	if(buffer_handler < 0 ) error("Error! Unable to read from socket");

	printf("\nSearch Term is:%s\n\n", searchTerms);
	if (checkExist) //check if file exists error handling
	{
		//if exists
		ptrToFile = fopen(DATA_FILE, "rb");

		fseek(ptrToFile, sizeof(struct document), SEEK_END);		 //get to the end of the file
		int lineNumber = ftell(ptrToFile) / sizeof(struct document); //give the cursor byte size offset divided by the document size.

		if (lineNumber == 1)
		{
			printf("\n\nNo patient recorded\n\n");
		}

		//get to the first line
		rewind(ptrToFile);
		int found = 0;
		printf("\n***********SEARCH RESULTS *************\n");
		//start searching
		while (fread(&patient, sizeof(struct document), 1, ptrToFile))
		{
			//if found
			if (strcasestr((patient.name), searchTerms)  != 0 || strcasestr((patient.surname), searchTerms) != 0 || strcasestr((patient.city), searchTerms)  != 0 || strcasestr((patient.birth_year), searchTerms)  != 0 )
			{
				printf("%d\t%s\t%s\t%s\t%s\n", found+ 1,patient.name, patient.surname, patient.birth_year, patient.city);
				found++;
			}
		}
		char str[80];
		
        sprintf(str, "%d", found);
		write(socket, &str, sizeof(str));
		rewind(ptrToFile);
		found = 0;
		while (fread(&patient, sizeof(struct document), 1, ptrToFile))
		{
			//if found
			if (strcasestr((patient.name), searchTerms)  != 0 || strcasestr((patient.surname), searchTerms) != 0 || strcasestr((patient.city), searchTerms)  != 0 || strcasestr((patient.birth_year), searchTerms)  != 0 )
			{
				char received[2];
				int buffer_handler;
				char bufferString[1024] = {0};
				size_t bufLen = serialize(&patient, bufferString);

				buffer_handler = write(socket, &bufferString, bufLen);
				if(buffer_handler < 0) error ("Failed  to write to socket");
			
				buffer_handler = read(socket, &received, sizeof(received));

				if(buffer_handler < 0 ) error ("Failed to read from socket");
				printf("%d\t%s\t%s\t%s\t%s\n", found+ 1,patient.name, patient.surname, patient.birth_year, patient.city);
				found++;
			}
		}
		printf("\nScan completed: %d patient(s) found\n\n", found);
		if (found){
			char deleteOrModify[50];

			buffer_handler = read(socket, deleteOrModify, sizeof(deleteOrModify));
			if(buffer_handler < 0 ) error("Error! Unable to read from socket");
			printf("deleteOr Modify %s\n", deleteOrModify);
			if (atoi(deleteOrModify) == 0) return ;
			char occurenceNo[50];
			
			buffer_handler = read(socket, occurenceNo, sizeof(occurenceNo));
			if(buffer_handler < 0 ) error("Error! Unable to read from socket");
			printf("occurenceNo %s\n", occurenceNo);

			if (atoi(deleteOrModify) == 2 && atoi(occurenceNo) > 0){
				removePatientByOccurence(socket, atoi(occurenceNo), searchTerms);
			
			}else if(atoi(deleteOrModify) == 1 && atoi(occurenceNo) > 0){
				modifyPatientByOccurence(socket, atoi(occurenceNo), searchTerms);
			}
		}
		return 0;
	}
	else
	{
		printf("\nError reading document; file %s not found\n\n", DATA_FILE);
	}
}
int removePatientByOccurence(int socket, int occurenceNo, char searchTerms[50])
{
    int position;
    char n[80];

	FILE *checkExist, *ptrToFile, *tempPtrToFile;

	struct document patient;
	
	checkExist = fopen(DATA_FILE, "r");

	if (checkExist)
	{
		ptrToFile = fopen(DATA_FILE, "rb");
		fseek(ptrToFile, sizeof(struct document), SEEK_END);		 //get to the end of the file
		int lineNumber = ftell(ptrToFile) / sizeof(struct document); //give the cursor byte size offset divided by the document size.

		if (lineNumber == 1)
		{
			printf("\n\nNo patient recorded\n\n");
			return 0;
		}

		rewind(ptrToFile);
		int counter = 0;
		tempPtrToFile = fopen(TEMP_FILE, "wb");
		if (!tempPtrToFile)
		{
			printf("Unable to open file temp file.");
			return -1;
		}
		int found = 1 ;
		while (fread(&patient, sizeof(struct document), 1, ptrToFile))
		{

			if ( (strcasestr(patient.name, searchTerms)  != 0 || strcasestr((patient.surname), searchTerms) != 0 || strcasestr((patient.city), searchTerms)  != 0 || strcasestr((patient.birth_year), searchTerms)  != 0))
			{
				if (found == occurenceNo){
					printf("%d\t%s\t%s\t%s\t%s deleted\n", found+ 1,patient.name, patient.surname, patient.birth_year, patient.city);

				}else
					fwrite(&patient, sizeof(struct document), 1, tempPtrToFile);
				found++;
			}
			else
			{
				fwrite(&patient, sizeof(struct document), 1, tempPtrToFile);
			}
			counter++;
		}

		fclose(ptrToFile);
		fclose(tempPtrToFile);
		remove(DATA_FILE);
		rename(TEMP_FILE, DATA_FILE);

		return 0;
	}
	else
	{
		printf("\nError reading document; file %s not found\n\n", DATA_FILE);
	}
	return 0;
}
int modifyPatientByOccurence(int socket, int occurenceNo, char searchTerms[50])
{
    int position;
    char n[80];

	FILE *checkExist, *ptrToFile, *tempPtrToFile;

	struct document patient;
	
	checkExist = fopen(DATA_FILE, "r");

	if (checkExist)
	{
		ptrToFile = fopen(DATA_FILE, "rb");
		fseek(ptrToFile, sizeof(struct document), SEEK_END);		 //get to the end of the file
		int lineNumber = ftell(ptrToFile) / sizeof(struct document); //give the cursor byte size offset divided by the document size.

		if (lineNumber == 1)
		{
			printf("\n\nNo patient recorded\n\n");
			return 0;
		}

		rewind(ptrToFile);
		int counter = 0;
		tempPtrToFile = fopen(TEMP_FILE, "wb");
		if (!tempPtrToFile)
		{
			printf("Unable to open file temp file.");
			return -1;
		}
		int found = 1 ;
		while (fread(&patient, sizeof(struct document), 1, ptrToFile))
		{

			if ( (strcasestr(patient.name, searchTerms)  != 0 || strcasestr((patient.surname), searchTerms) != 0 || strcasestr((patient.city), searchTerms)  != 0 || strcasestr((patient.birth_year), searchTerms)  != 0))
			{
				if (found == occurenceNo){
					char tmpLine[500] = {0};
					size_t len = serialize(&patient,tmpLine);
					write(socket, &tmpLine, len);
					printf("line to be modified sent");
					bzero(tmpLine, sizeof(tmpLine));
					DOCUMENT tmp;
					read(socket, tmpLine, sizeof(tmpLine));
					printf("modified line received");
					deserialize(tmpLine, &tmp);
					printf("%d\t%s\t%s\t%s\t%s modified\n", found+ 1,patient.name, patient.surname, patient.birth_year, patient.city);
					fwrite(&tmp, sizeof(struct document), 1, tempPtrToFile);
				}else
					fwrite(&patient, sizeof(struct document), 1, tempPtrToFile);
				found++;
			}
			else
			{
				fwrite(&patient, sizeof(struct document), 1, tempPtrToFile);
			}
			counter++;
		}

		fclose(ptrToFile);
		fclose(tempPtrToFile);
		remove(DATA_FILE);
		rename(TEMP_FILE, DATA_FILE);

		return 0;
	}
	else
	{
		printf("\nError reading document; file %s not found\n\n", DATA_FILE);
	}
	return 0;
}
int removePatient(int socket)
{
    int position;
    displayData(socket);
    char n[80];

    read(socket,  &n, sizeof(n));
    position = atoi(n);
    printf("%d ---------", position);
    if(position == 0) return 0;
	FILE *checkExist, *ptrToFile, *tempPtrToFile;

	struct document patient;
	
	checkExist = fopen(DATA_FILE, "r");

	if (checkExist)
	{
		ptrToFile = fopen(DATA_FILE, "rb");
		fseek(ptrToFile, sizeof(struct document), SEEK_END);		 //get to the end of the file
		int lineNumber = ftell(ptrToFile) / sizeof(struct document); //give the cursor byte size offset divided by the document size.

		if (lineNumber == 1)
		{
			printf("\n\nNo patient recorded\n\n");
			return 0;
		}

		rewind(ptrToFile);
		int counter = 0;
		tempPtrToFile = fopen(TEMP_FILE, "wb");
		if (!tempPtrToFile)
		{
			printf("Unable to open file temp file.");
			return -1;
		}

		while (fread(&patient, sizeof(struct document), 1, ptrToFile))
		{

			if ((position - 1) == counter)
			{
				printf("%s\t%s\t%s\t%s deleted\n", patient.name, patient.surname, patient.birth_year, patient.city);
			}
			else
			{
				fwrite(&patient, sizeof(struct document), 1, tempPtrToFile);
			}
			counter++;
		}

		fclose(ptrToFile);
		fclose(tempPtrToFile);
		remove(DATA_FILE);
		rename(TEMP_FILE, DATA_FILE);

		return 0;
	}
	else
	{
		printf("\nError reading document; file %s not found\n\n", DATA_FILE);
	}
	return 0;
}
int modifyPatient(int socket)
{
	printf("here....");
    int position;
    displayData(socket);
	char buffer[500] = {0};
    char n[80];
	printf("here....");
    read(socket,  &n, sizeof(n));
    position = atoi(n);
	if (!position) return;
    printf("%d ---------", position);
	read(socket, buffer, sizeof(buffer));
    if(position == 0) return 0;
	FILE *checkExist, *ptrToFile, *tempPtrToFile;
	printf("here....");
	DOCUMENT tmp;
    deserialize(buffer, &tmp);
    
	struct document patient;
	
	checkExist = fopen(DATA_FILE, "r");

	if (checkExist)
	{
		ptrToFile = fopen(DATA_FILE, "rb");
		fseek(ptrToFile, sizeof(struct document), SEEK_END);		 //get to the end of the file
		int lineNumber = ftell(ptrToFile) / sizeof(struct document); //give the cursor byte size offset divided by the document size.

		if (lineNumber == 1)
		{
			printf("\n\nNo patient recorded\n\n");
			
		}

		rewind(ptrToFile);
		int counter = 0;
		tempPtrToFile = fopen(TEMP_FILE, "wb");
		if (!tempPtrToFile)
		{
			printf("Unable to open file temp file.");
			return -1;
		}
		printf("here....");
		while (fread(&patient, sizeof(struct document), 1, ptrToFile))
		{

			if ((position - 1) == counter)
			{
				printf("%s\t%s\t%s\t%s modified\n", tmp.name, tmp.surname, tmp.birth_year, tmp.city);
				fwrite(&tmp, sizeof(struct document), 1, tempPtrToFile);
			}
			else
			{
				fwrite(&patient, sizeof(struct document), 1, tempPtrToFile);
			}
			counter++;
		}
		printf("here....");
		fclose(ptrToFile);
		fclose(tempPtrToFile);
		remove(DATA_FILE);
		rename(TEMP_FILE, DATA_FILE);

		return 0;
	}
	else
	{
		printf("\nError reading document; file %s not found\n\n", DATA_FILE);
	}
	return 0;
	printtf("Quitting modify patient");
}


//Write patients to file
int writeToFile(struct document patient)
{

	FILE *ptrToFile;

	ptrToFile = fopen(DATA_FILE, "ab+");

	fwrite(&patient, sizeof(struct document), 1, ptrToFile);

	fclose(ptrToFile);

	return 0;
}
//INSERT PATIENT 
int insertPatient(int accepted_socket)
{
	DOCUMENT patient;
	FILE *ptrToFile;
    int buffer_handler;
    char buffer[1024] = {0};
	ptrToFile = fopen(DATA_FILE, "r");
	if (ptrToFile)
	{
		// fseek(ptrToFile, sizeof(struct document), SEEK_END);		 //get to the end of the file
		// int lineNumber = ftell(ptrToFile) / sizeof(struct document); //give the cursor byte size offset divided by the document size.

		//printf("%d", lineNumber);
        buffer_handler = read(accepted_socket, buffer, sizeof(buffer));
    
        if(buffer_handler < 0 ) error("Error! Unable to read from socket");

        deserialize(buffer, &patient);

        // extract(buff);
    	// if (lineNumber > 1){
		// 	patient.id = lineNumber;
		// }else
		// 	patient.id = 1;

		writeToFile(patient);
        buffer_handler = write(accepted_socket, "1", 1);
        printf("Added Patient %s to file !", patient.name);
		fclose(ptrToFile);
	}
	else
	{

		printf("\nAn Error has occured: File cannot be found or opened\n\n");
		return 0;
	}
    
}


int gestion(int clientSocket, int choice){
    switch(choice){
        case 1:
            insertPatient(clientSocket);
            break;
        case 2:
			searchFile(clientSocket);
            break;
        case 3:
			modifyPatient(clientSocket);
            break;
        case 4:
            removePatient(clientSocket);
            break;
        case 5:
            displayData(clientSocket);
            break;
        
    };
    char selection[256];
    read(clientSocket,  &selection, sizeof(selection));
    gestion(clientSocket, atoi(selection) );
    
}


int main(int argc, char *argv[]){

    struct sockaddr_in server_address,  client_address; 
    int available_socket, accepted_socket, port_no, client_address_length, buffer_handler;
    char buffer[1024] = {0};

    if (argc < 2){
        printf("Usage: %s [PORT NUMBER]\n\nNo port provided, trying default port: %d\n", argv[0], DEFAULT_SERVER_PORT );
        port_no = DEFAULT_SERVER_PORT;
    }else{
        printf("Port selected: %s\n", argv[1]);
        port_no = atoi(argv[1]);
    }

    available_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (available_socket < 0){
        error("\nError! Socket not opened");
    }
    bzero((char *) &server_address, sizeof(server_address));

    server_address.sin_port = htons(port_no);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    if(bind(available_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0){
        printf("\nError! Failed to use port: %d\n\n", port_no);
        getAvailablePort(&available_socket, &port_no);
        server_address.sin_port = htons(port_no);
    }
    printf("\nPort successfully set to %d\n", port_no);

    listen(available_socket, 5);
    client_address_length = sizeof(client_address);

    accepted_socket = accept(available_socket, (struct sockaddr *) &client_address, &client_address_length);
    printf("\nNew Client connected !\n");

    if (accepted_socket < 0){
        error("\nError! Unable to accept socket");
    }
    
    char selection[256];

    buffer_handler = read(accepted_socket,  &selection, sizeof(selection));
    
    if(buffer_handler < 0 ) error("Error! Unable to read from socket");
    gestion(accepted_socket, atoi(selection) );
    
   

    close(available_socket);
    close(accepted_socket);
    return 0;
}



//  while (1){
//         buffer_handler = read(accepted_socket, buffer, sizeof(buffer));
    
//         if(buffer_handler < 0 ) error("Error! Unable to read from socket");

//         DOCUMENT tmp;
//         deserialize(buffer, &tmp);
//         printf("Message1: %s\n", tmp.name);
//         printf("Message3: %s\n", tmp.surname);
//         printf("Message2: %s\n", tmp.city);
//         printf("Message4: %s\n", tmp.birth_year);


//         buffer_handler = write(accepted_socket, "Well received", 14 );
//         if(buffer_handler < 0) error("Error! Unable to write to socket");

//     }
