#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

//f1 770
//f2 707
//f3 007

void permisiuni(int fDest, struct stat fileInfo, char *usr,int r, int w, int x){
    write(fDest, "Permisiuni ", strlen("Permisiuni "));
    write(fDest, usr, strlen(usr));
    write(fDest, ": ", strlen(": "));

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

void printFileData(char *filePath, char* fileName, int fileTagNumber){
    struct stat file_info, lfile_info;
    
    if(lstat(filePath, &lfile_info)==-1){
        printf("EROARE LA PROCESAREA FISIERULUI.\n");
        exit(EXIT_FAILURE);
    }
        
    if(stat(filePath, &file_info)==-1){
        printf("EROARE LA PROCESAREA FISIERULUI.\n");
        exit(EXIT_FAILURE);
    }

    //pregatire fisier statistica pentru printare
    int sursa = open(filePath,O_RDONLY);//deschidem fisierul pentru citire
    if(sursa==-1){
        printf("Eroare la parcurgerea fisierului bmp.\n");
        close(sursa);
        exit(EXIT_FAILURE);
    }

    //verificam daca statistica exista
    //daca nu, il cream
    if(access("statistica.txt",F_OK)!=0)
        creat("statistica.txt", 0777);

    int destinatie;
    if((destinatie = open("statistica.txt",O_WRONLY | O_APPEND)) == -1){
        printf("Eroare la deschiderea fisierului statistica.txt.\n");
        close(destinatie);
        exit(EXIT_FAILURE);
    };
    
    printf("Verificati fisierul \"statistica.txt\" pentru detalii despre fisierul .bmp accesat.\n");

    //printare nume - orice tip de fisier
    char text[50] = "Numele fisierului: ";
    write(destinatie,text,strlen(text));
    write(destinatie,fileName,strlen(fileName));
    write(destinatie,"\n",1);

    //printare intaltime si lungime doar in cazul in care fileTagNumber e 4(BMP)
    if(fileTagNumber == 4){
        int lungime, inaltime;
        lseek(sursa,18,SEEK_SET);
        read(sursa,&lungime,4);
        read(sursa,&inaltime,4);
        printIntToFile(destinatie,inaltime,"Inaltimea fisierului: ");
        printf("inaltime: %d\n",inaltime);
        printIntToFile(destinatie,lungime,"Latimea fisierului: ");
        printf("lungime: %d\n",lungime);
    }

    //printare dimensiune fisier din lstat in cazul in care nu e fileTagNumber 2(DIR)
    //cu fd-ul dupa apelul lstat acoperim si dimensiunea symlnk-ului
    if(fileTagNumber != 2){
        strcpy(text,"Dimensiunea fisierului: ");
        printIntToFile(destinatie,lfile_info.st_size,text);
    }

    //printare dimensiune fisier targetat din stat in cazul in care fileTagNumber e 1(SYMLNK)
    if(fileTagNumber == 1){
        strcpy(text,"Dimensiunea fisierului targetat: ");
        printIntToFile(destinatie,file_info.st_size,text);
    }

    //printare user_id in cazul in care fileTagNumber nu e 1(SYMLNK)
    if(fileTagNumber != 1){
        strcpy(text,"ID-ul user-ului : ");
        printIntToFile(destinatie,file_info.st_uid,text);
    }

    //printarea timpului ultimei accesari si contorului de legatura doar in cazul in care
    //fileTagNumber e doar 3 sau 4(.BMP sau REG)
    if(fileTagNumber == 3 || fileTagNumber == 4){
        strcpy(text,"Data ultimei modificari: ");
        write(destinatie,text,strlen(text));
        write(destinatie,ctime(&lfile_info.st_mtime),strlen(ctime(&lfile_info.st_mtime)));
        strcpy(text,"Numarul de legaturi ale fisierului : ");
        printIntToFile(destinatie,file_info.st_nlink,text);
    }
    //printare drepturi de acces pentru orice tip de fisier
    permisiuni(destinatie,lfile_info,"user",S_IRUSR, S_IWUSR, S_IXUSR);
    permisiuni(destinatie,lfile_info,"group",S_IRGRP, S_IRGRP, S_IXGRP);
    permisiuni(destinatie,lfile_info,"others",S_IROTH, S_IWOTH, S_IXOTH);
    write(destinatie,"\n",1);
    write(destinatie,"\n",1);
    close(sursa);
    close(destinatie);
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
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        //printam numele endtry.d_name -> file1
        printf("\n%s\n",entry->d_name);
        //filepath = trebuie formata calea relativa dir_name/file_name, ./file_name
        char file_path[100] = "";
        strcat(file_path,argv[1]);
        strcat(file_path,"/");
        strcat(file_path,entry->d_name);
        //verificam tipul de file
        struct stat file_info;
        if(lstat(file_path, &file_info)==-1){
            printf("undefined behaviour\n");
            exit(EXIT_FAILURE);
        }

        if(S_ISLNK(file_info.st_mode)) {
            printFileData(file_path,entry->d_name,1);
        } else if (S_ISDIR(file_info.st_mode)) {
            printFileData(file_path,entry->d_name,2);
        } else if (S_ISREG(file_info.st_mode) && strstr(file_path, ".bmp") == NULL) {
            printFileData(file_path,entry->d_name,3);
        } else if (S_ISREG(file_info.st_mode) && strstr(file_path, ".bmp") != NULL) {
            printFileData(file_path,entry->d_name,4);
        }
    }
    return 0;
}
