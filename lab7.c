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
#include <ctype.h>

//TO DO - FUNC TO CHECK READ
void checkedRead(int fd, void *text, int size){
        if(read(fd,text,size) == -1){
            printf("EROARE LA CITIRE.\n");
            exit(EXIT_FAILURE);
        }
}

//TO DO - FUNC TO CHECK WRITE
void checkedWrite(int fd, void *text, int size){
        if(write(fd,text,size) == -1){
            printf("EROARE LA CITIRE.\n");
            exit(EXIT_FAILURE);
        }
}

//TO DO - FUNC TO CHECK READDIR
struct dirent * checkedReaddir(DIR *dirCitire){
    struct dirent *dir = readdir(dirCitire);
    if(dir == NULL && errno != 0){
        printf("EROARE LA CITIREA DIN FISIER.\n");
        exit(EXIT_FAILURE);
    }
    return dir;
}

//TO DO - FUNC TO CHECK CLOSE
void checkedClose(int fd){
    if(close(fd) == -1){
        printf("EROARE LA INCHIDEREA FISIERULUI.\n");
        exit(EXIT_FAILURE);
    }
}

//TO DO - FUNC TO CHECK DUP2
void checkedDup2(int nfd, int ofd){
    if(dup2(ofd, nfd) == -1){
        printf("EROARE LA REDIRECTARE.\n");
        exit(EXIT_FAILURE);
    }
}

void permisiuni(int fDest, struct stat fileInfo, char *usr,int r, int w, int x){
    checkedWrite(fDest, "Permisiuni ", strlen("Permisiuni "));
    checkedWrite(fDest, usr, strlen(usr));
    checkedWrite(fDest, ": ", strlen(": "));

    if(fileInfo.st_mode & r)
            checkedWrite(fDest,"R",1);
        else
            checkedWrite(fDest,"-",1);
    if(fileInfo.st_mode & w)
            checkedWrite(fDest,"W",1);
        else
            checkedWrite(fDest,"-",1);
    if(fileInfo.st_mode & x)
            checkedWrite(fDest,"X",1);
        else
            checkedWrite(fDest,"-",1);
    checkedWrite(fDest,"\n",1);
}

void printIntToFile(int fDest, int val, char *text){
    char arr[20];
    sprintf(arr,"%d",val);
    checkedWrite(fDest,text,strlen(text));
    checkedWrite(fDest,arr,strlen(arr));
    checkedWrite(fDest,"\n",1);
}

void printFileData(char *filePath, char *outputFile_path, char* fileName, int fileTagNumber){
    if(access(outputFile_path, F_OK) != 0)
        creat(outputFile_path, 0777);

    int destinatie;
    if((destinatie = open(outputFile_path, O_WRONLY)) == -1){
        printf("Eroare la deschiderea fisierului %s.\n", outputFile_path);
        checkedClose(destinatie);
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
        checkedClose(sursa);
        exit(EXIT_FAILURE);
    }
    
    printf("Verificati fisierul %s pentru detalii despre fisierul %s.\n",outputFile_path,filePath);
    //printare nume - orice tip de fisier
    char text[50] = "Numele fisierului: ";
    checkedWrite(destinatie,text,strlen(text));
    checkedWrite(destinatie,fileName,strlen(fileName));
    checkedWrite(destinatie,"\n",1);
    //printare intaltime si lungime doar in cazul in care fileTagNumber e 4(BMP)
    if(fileTagNumber == 4){
        int lungime, inaltime;
        lseek(sursa,18,SEEK_SET);
        checkedRead(sursa,&lungime,4);
        checkedRead(sursa,&inaltime,4);
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
        checkedWrite(destinatie,text,strlen(text));
        checkedWrite(destinatie,ctime(&lfile_info.st_mtime),strlen(ctime(&lfile_info.st_mtime)));
        strcpy(text,"Numarul de legaturi ale fisierului : ");
        printIntToFile(destinatie,file_info.st_nlink,text);
    }
    //printare drepturi de acces pentru orice tip de fisier
    permisiuni(destinatie,lfile_info,"user",S_IRUSR, S_IWUSR, S_IXUSR);
    permisiuni(destinatie,lfile_info,"group",S_IRGRP, S_IRGRP, S_IXGRP);
    permisiuni(destinatie,lfile_info,"others",S_IROTH, S_IWOTH, S_IXOTH);
    checkedClose(sursa);
    checkedClose(destinatie);
}

void greyTones(char *filePath) {
    int sursa = open(filePath, O_RDWR); 
    if(sursa == -1) {
        printf("Error opening file.\n");
        exit(EXIT_FAILURE);
    }
    lseek(sursa, 18, SEEK_SET);
    int x, y, cnt = 0;
    checkedRead(sursa, &x, sizeof(x));
    checkedRead(sursa, &y, sizeof(y));

    lseek(sursa, 54, SEEK_SET);
    //aveam nevoie de un tip de un byte care era pozitiv
    unsigned char red, green, blue;
    while(cnt < x * y) {
        checkedRead(sursa, &blue, 1);
        checkedRead(sursa, &green, 1);
        checkedRead(sursa, &red, 1);
        unsigned char gray = (unsigned char)(0.299 * red + 0.587 * green + 0.114 * blue);
        lseek(sursa, -3, SEEK_CUR);
        checkedWrite(sursa, &gray, 1);
        checkedWrite(sursa, &gray, 1);
        checkedWrite(sursa, &gray, 1);
        cnt++;
    }
    checkedClose(sursa);
}

void closeProcesses(int cnt){
    int status;
    int exitStatus;
    for(int i = 0 ; i < cnt ; i++){
        int pid = wait(&status);
        if (WIFEXITED(status) && (exitStatus = WEXITSTATUS(status)) != 3) {
            printf("S-A INCHEIAT PROCESUL %d CU CODUL %d.\n", pid, WEXITSTATUS(status));
        } 
    }
}

int main(int argc, char *argv[]){
    int cnt = 0;
    int propCorecte = 0;
    if(argc!=4){
        printf("Numar nepotrivit de argumente!\n");
        exit(EXIT_FAILURE);
    }
    //verifica daca argv[3] e caracter
    if(strlen(argv[3]) != 1 || isalpha(argv[3][0])==0){
        printf("AL TREILEA ARGUMENT NU SATISFACE CONDITIILE DE CARACTER.\n");
        exit(EXIT_FAILURE);
    }
    DIR *dir_citire = opendir(argv[1]);
    //DIR *dir_scriere = opendir(argv[1]);
    if(dir_citire == NULL){
        printf("EROARE LA DESCHIDEREA DIRECTORULUI DAT PENTRU CITIRE.\n");
        exit(EXIT_FAILURE);
    }
    struct dirent *entry;
    while((entry = checkedReaddir(dir_citire))){
        int childGreyscale;
        int childStatistics;
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
        //cale fisier output folderOutput/nume.statistica.txt
        char outputFile_path[100] = "";
        strcat(outputFile_path,argv[2]);
        strcat(outputFile_path,"/");
        strcat(outputFile_path,entry->d_name);
        strcat(outputFile_path,".statistica.txt");
        struct stat file_info;
        if(lstat(file_path, &file_info)==-1){
            printf("Undefined behaviour.\n");
            exit(EXIT_FAILURE);
        }
        
        if(lstat(file_path, &file_info)==-1){
            printf("No such output dir.\n");
            exit(EXIT_FAILURE);
        }

        int statToBash[2], bashToMain[2];
        if(pipe(statToBash) == -1 || pipe(bashToMain) == -1){
            printf("EROARE LA DESCHIDEREA PIPE-URILOR.\n");
            exit(EXIT_FAILURE);
        }
        //file data process
        childStatistics = fork();
        if(childStatistics < 0){
            printf("EROARE.");
            exit(EXIT_FAILURE); 
        }else if(childStatistics == 0){
            if(S_ISLNK(file_info.st_mode)) {
                printFileData(file_path,outputFile_path,entry->d_name,1);
                exit(6);
            } else if (S_ISDIR(file_info.st_mode)) {
                printFileData(file_path,outputFile_path,entry->d_name,2);
                exit(5);
            } else if (S_ISREG(file_info.st_mode) && strstr(file_path, ".bmp") == NULL) {
                checkedClose(bashToMain[0]);
                checkedClose(bashToMain[1]);
                checkedClose(statToBash[0]);
                
                printFileData(file_path,outputFile_path,entry->d_name,3);
                dup2(statToBash[1],STDOUT_FILENO);

                execlp("cat","cat",file_path, NULL); 
                perror("EROARE LA TRANSMITEREA CONTINUTULUI.\n");
                exit(EXIT_FAILURE);
            } else if (S_ISREG(file_info.st_mode) && strstr(file_path, ".bmp") != NULL) {
                printFileData(file_path,outputFile_path,entry->d_name,4);
                exit(10);
            }
        }else{
            cnt++;
        }

        //greyscale process
        if (childStatistics > 0 && (S_ISREG(file_info.st_mode) && strstr(file_path, ".bmp") != NULL)){
            childGreyscale = fork();
            if(childGreyscale < 0){
                printf("EROARE.");
                exit(EXIT_FAILURE); 
            } else if(childGreyscale == 0){
                greyTones(file_path);     
                exit(3);
            }else{
                cnt++;
            }
        }
        //bash command process
        if (childStatistics > 0 && (S_ISREG(file_info.st_mode) && strstr(file_path, ".bmp") == NULL)){
            int childBash = fork();
            if(childBash < 0){
                printf("EROARE IN PROCESUL DE RULARE A SCRIPTULUI.\n");
                exit(EXIT_FAILURE);
            }else if(childBash == 0){
                checkedClose(statToBash[1]);
                checkedClose(bashToMain[0]);

                dup2(statToBash[0],STDIN_FILENO);
                dup2(bashToMain[1], STDOUT_FILENO); 
                checkedClose(statToBash[0]);
                //checkedClose(bashToMain[1]);

                execlp("/bin/bash","/bin/bash","bash1.sh",argv[3], (char *)NULL);
                checkedClose(bashToMain[1]); 
                perror("EROARE LA EXECUTIA SCRIPTULUI.\n");
                exit(EXIT_FAILURE);
            }else if(childBash > 0){
                checkedClose(statToBash[0]);
                checkedClose(bashToMain[1]);
                checkedClose(statToBash[1]);
                char scriptResult[10];
                checkedRead(bashToMain[0], scriptResult, sizeof(scriptResult) - 1);    
                scriptResult[sizeof(scriptResult) - 1] = '\0';
                propCorecte += atoi(scriptResult);
                cnt++;
                checkedClose(bashToMain[0]);
            }
        }
    } 
    closedir(dir_citire);
    closeProcesses(cnt);
    printf("S-au numarat %d propozitii corecte.\n",propCorecte);
    return 0;
}
