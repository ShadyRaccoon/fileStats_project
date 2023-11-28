#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

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

void printFileData(char *filePath, char *outputFile_path, char* fileName, int fileTagNumber){
    if(access(outputFile_path, F_OK) != 0)
        creat(outputFile_path, 0777);

    int destinatie;
    if((destinatie = open(outputFile_path, O_WRONLY | O_APPEND)) == -1){
        printf("Eroare la deschiderea fisierului %s.\n", outputFile_path);
        close(destinatie);
        exit(EXIT_FAILURE);
    }

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
    
    printf("Verificati fisierul %s pentru detalii despre fisierul %s.\n",outputFile_path,filePath);
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
        printIntToFile(destinatie,lungime,"Latimea fisierului: ");
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
    close(sursa);
    close(destinatie);
}

void printNumarLinii(int n, char *filePath){
    int destinatie;
    if((destinatie = open("statistica.txt", O_WRONLY | O_APPEND)) == -1){
        printf("Eroare la deschiderea fisierului %s.\n", "statistica.txt");
        close(destinatie);
        exit(EXIT_FAILURE);
    }
    char arr[5];
    sprintf(arr,"%d",n);
    char a[250] = "S-au scris ";
    strcat(a, arr);
    strcat(a, " linii despre fisierul ");
    strcat(a, filePath);
    strcat(a, ".\n");
    write(destinatie,a,strlen(a));
}

int main(int argc, char *argv[]){
    if(argc!=3){
        printf("Numar nepotrivit de argumente!\n");
        exit(EXIT_FAILURE);
    }

    DIR *dir_citire = opendir(argv[1]);
    DIR *dir_scriere = opendir(argv[1]);
    if(dir_citire == NULL){
        printf("EROARE LA DESCHIDEREA DIRECTORULUI DAT PENTRU CITIRE.\n");
        exit(EXIT_FAILURE);
    }
    if(dir_scriere == NULL){
        printf("EROARE LA DESCHIDEREA DIRECTORULUI DAT PENTRU SCRIERE.\n");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while((entry = readdir(dir_citire))){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        //printam numele endtry.d_name la stdout
        printf("\n%s\n",entry->d_name);
        //filepath = trebuie formata calea relativa dir_name/file_name, ./file_name
        char file_path[100] = "";
        strcat(file_path,argv[1]);
        strcat(file_path,"/");
        strcat(file_path,entry->d_name);

        char outputFile_path[100] = "";
        strcat(outputFile_path,argv[2]);
        strcat(outputFile_path,"/");
        strcat(outputFile_path,entry->d_name);
        strcat(outputFile_path,".statistica.txt");
        
        struct stat file_info;
        if(lstat(file_path, &file_info)==-1){
            printf("undefined behaviour\n");
            exit(EXIT_FAILURE);
        }

        int f_id = fork();
        if(f_id == -1){
            printf("EROARE.");
            exit(EXIT_FAILURE); 
        }else if(f_id == 0){
            if(S_ISLNK(file_info.st_mode)) {
                printFileData(file_path,outputFile_path,entry->d_name,1);
                printNumarLinii(6,file_path);
            } else if (S_ISDIR(file_info.st_mode)) {
                printFileData(file_path,outputFile_path,entry->d_name,2);
                printNumarLinii(5,file_path); 
            } else if (S_ISREG(file_info.st_mode) && strstr(file_path, ".bmp") == NULL) {
                printFileData(file_path,outputFile_path,entry->d_name,3);
                printNumarLinii(8,file_path);
            } else if (S_ISREG(file_info.st_mode) && strstr(file_path, ".bmp") != NULL) {
                printFileData(file_path,outputFile_path,entry->d_name,4);
                printNumarLinii(10,file_path);
            }

            //inchidere proces
            exit(0);
        }
    }
    wait(NULL);
    return 0;
}
