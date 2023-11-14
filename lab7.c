#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>//afla st_ino, st_uid, st_mode
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

//f1 770
//f2 707
//f3 007

void regFile(struct stat fileInfo){
    //user
    if(fileInfo.st_mode & S_IRUSR)
            printf("R");
        else
            printf("-");
    if(fileInfo.st_mode & S_IWUSR)
            printf("W");
        else
            printf("-");
    if(fileInfo.st_mode & S_IXUSR)
            printf("X-");
        else
            printf("--");

    //group
    if(fileInfo.st_mode & S_IRGRP)
            printf("R");
        else
            printf("-");
    if(fileInfo.st_mode & S_IWGRP)
            printf("W");
        else
            printf("-");
    if(fileInfo.st_mode & S_IXGRP)
            printf("X-");
        else
            printf("--");

    //others
    if(fileInfo.st_mode & S_IROTH)
            printf("R");
        else
            printf("-");
    if(fileInfo.st_mode & S_IWOTH)
            printf("W");
        else
            printf("-");
    if(fileInfo.st_mode & S_IXOTH)
            printf("X");
        else
            printf("-");
}

void permisiuniBmp(int fDest, struct stat fileInfo, int r, int w, int x){
    //user
    if(fileInfo.st_mode & r)
            write(fDest,"R",1);
        else
            write(fDest,"-",1);
    if(fileInfo.st_mode & w)
            write(fDest,"W",1);
        else
            write(fDest,"-",1);
    if(fileInfo.st_mode & x)
            write(fDest,"X",1);
        else
            write(fDest,"-",1);
    write(fDest,"\n",1);
}

void printIntToFile(int fDest, int val, char *text){
    char arr[20];
    sprintf(arr,"%d",val);
    write(fDest,text,strlen(text));
    write(fDest,arr,strlen(arr));
    write(fDest,"\n",1);
}

void bmpFile(struct stat fileInfo, char* pathName, char* fileName){
    //verific daca fisierul statistica.txt exista
    int sursa = open(pathName,O_RDONLY);//deschidem fisierul pentru citire
    if(sursa==-1){
        printf("Eroare la parcurgerea fisierului bmp.\n");
        close(sursa);
        exit(EXIT_FAILURE);
    }
    if(access("statistica.txt",F_OK)==0){
        printf("Verificati fisierul \"statistica.txt\" pentru detalii despre fisierul .bmp accesat.\n");
        //partea de citire din bmp, stocare si afisare in statistica.txt
        {
            /*
                _inaltime: 1920                              //sare 4 bytes citeste 4 (3) 
                _lungime: 1280                               //sare 12 bytes citeste 4 bytes (2)
                _dimensiune: <dimensiune in octeti>          //sare 2 bytes citeste 4 bytes(1)
                _identificatorul utilizatorului: <user id>   //st_uid in struct stat
                _timpul ultimei modificari: 28.10.2023       //st_mtim in struct stat
                contorul de legaturi: <numar legaturi>      //st_nlink in struct stat
                _drepturi de acces user: RWX                 //refa functia de la reg file pentru bmp
                _drepturi de acces grup: R–-                 //refa functia de la reg file pentru bmp
                _drepturi de acces altii: ---                //refa functia de la reg file pentru bmp
            */
            int dimensiune, inaltime, latime;

            //deschidere si verificare deschidere statistica.txt
            int destinatie = open("statistica.txt",O_WRONLY | O_APPEND);
            if(destinatie == -1){
                printf("Eroare la scriere.\n");
                close(sursa);
                exit(EXIT_FAILURE);
            }

            char text[50] = "Numele fisierului: ";
            write(destinatie,text,strlen(text));
            write(destinatie,fileName,strlen(fileName));
            write(destinatie,"\n",1);

            //skip la primii 2 bytes pentru a afla dimensiunea
            lseek(sursa,2,SEEK_SET);
            read(sursa,&dimensiune,4);

            //skip la urmatorii 12 bytes pentru a afla latimea
            lseek(sursa,12,SEEK_CUR);
            read(sursa,&latime,4);

            //skip la urmatorii 4 bytes pentru a afla lungimea
            lseek(sursa,0,SEEK_CUR);
            read(sursa,&inaltime,4);

            strcpy(text,"Inaltimea fisierului: ");
            printIntToFile(destinatie,inaltime,text);

            strcpy(text,"Latimea fisierului: ");
            printIntToFile(destinatie,latime,text);

            strcpy(text,"Dimensiunea fisierului: ");
            printIntToFile(destinatie,dimensiune,text);

            //afisare user id
            strcpy(text,"ID-ul user-ului : ");
            printIntToFile(destinatie,fileInfo.st_uid,text);

            strcpy(text,"Data ultimei modificari: ");
            write(destinatie,text,strlen(text));
            write(destinatie,ctime(&fileInfo.st_mtime),strlen(ctime(&fileInfo.st_mtime)));

            //afisare contor de legaturi
            strcpy(text,"Numarul de legaturi ale fisierului : ");
            printIntToFile(destinatie,fileInfo.st_nlink,text);

            strcpy(text,"Permisiuni user: ");
            write(destinatie,text,strlen(text));
            permisiuniBmp(destinatie, fileInfo, S_IRUSR, S_IWUSR, S_IXUSR);

            strcpy(text,"Permisiuni group: ");
            write(destinatie,text,strlen(text));
            permisiuniBmp(destinatie, fileInfo, S_IRGRP, S_IRGRP, S_IXGRP);

            strcpy(text,"Permisiuni others: ");
            write(destinatie,text,strlen(text));
            permisiuniBmp(destinatie, fileInfo, S_IROTH, S_IWOTH, S_IXOTH);

            write(destinatie,"\n",1);
            write(destinatie,"\n",1);

            close(sursa);
            close(destinatie);
        }
    }else{
        printf("Fisierul nu exista.\n"); 
        close(sursa);
        exit(EXIT_FAILURE);       
    }
    close(sursa);
}

int main(int argc, char *argv[]){
    //verif nr de arg primite
    if(argc!=2){
        printf("Numar nepotrivit de argumente!\n");
        exit(EXIT_FAILURE);
    }
    
    DIR *dir_pointer = opendir(argv[1]);
    
    //verificare daca este null
    if(dir_pointer == NULL){
        printf("EROARE LA DESCHIDEREA DIRECTORULUI DAT.\n");
        exit(EXIT_FAILURE);
    }
    
    //daca nu, continuam
    struct dirent *entry;
    
    while((entry = readdir(dir_pointer))){
        //printam numele endtry.d_name -> file1
        printf("\n%s\n",entry->d_name);

        //filepath = trebuie formata calea relativa dir_name/file_name, ./file_name
        char file_path[100] = "";
        strcat(file_path,argv[1]);
        strcat(file_path,"/");
        strcat(file_path,entry->d_name);
        
        //verificam tipul de file
        struct stat file_info;

        
        
        if(stat(file_path, &file_info)==-1){
            printf("undefined behaviour\n");
            exit(EXIT_FAILURE);
        }

        if(strstr(file_path,".bmp") == NULL)
            //if regular file then
            regFile(file_info);
        else
            //if not regular file then (bmp)
            bmpFile(file_info, file_path, entry->d_name);
    }

    return 0;
}