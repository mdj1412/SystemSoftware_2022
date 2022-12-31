#include <dirent.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/stat.h> 
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h> 
#include <pwd.h> 
// error number : 정적 메모리 위치에 저장된 오류 코드를 통해 
// 오류 상태를 보고 및 검색하기 위한 매크로를 정의
#include <errno.h>

#define MAX_PATH_LEN 1024

// 해당 파일의 mode를 출력해준다.
void fileMode(const struct stat *fileInfo);
// File Type을 확인하고 출력해준다.
void fileType(const struct stat *fileInfo);
// File Size를 출력해준다.
void fileSize(const struct stat *fileInfo);
// 재귀 함수
int mytree(int* num, int line);

// 정렬할 때 필요한 비교 함수
int compar(const struct dirent ** dirInfo1, const struct dirent ** dirInfo2);


/*

man scandir : 터미널 명령어 적으면 설명 적혀있다.

#include <dirent.h>

int scandir(const char *dirp, 
            struct dirent ***namelist,
            int (*filter)(const struct dirent *),
            int (*compar)(const struct dirent **, const struct dirent **));




const char *dirp : 파일 목록 또는 디렉토리 목록을 얻을 디렉토리에 대한 절대 path 또는 상대 path

struct dirent ***namelist : - namelist에 디렉토리에 있는 파일 및 디렉토리 목록이 저장
                            - 내부적으로 malloc() 또는 realloc()으로 할당되므로 사용후에는 반드시 free()를 해야한다. 
                            - 다중 pointer 변수를 선언해서 사용하므로 건별 free() 후 namelist 자체도 free()를 해야한다.

int (*filter)(const struct dirent *) :  - namelist에 포함시킬 것인지 여부를 판단하는 함수에 대한 pointer
                                        - 이 filter함수의 return 값이 0이면 namelist에 포함시키지 않고 0이 아니면 포함시킨다.
                                        - NULL이면 filter없이 파일 및 디렉토리 전체가 namelist에 저장된다.
                                        - . 및 .. 디렉토리도 포함되어 있다.

int (*compar)(const struct dirent **, const struct dirent **) : 
                - 데이터를 sort할 비교함수에 대한 포인터입니다.
                - 내부적으로 qsort()를 사용하므로 이 함수를 통하여 sorting합니다. 
                - 만약 이름 (struct dirent 구조체의) d_name으로 sorting하려고 한다면 이미 구현된 alphasort() 함수를 사용할 수 있습니다.
                - 만약 이 compar를 NULL로 설정하면 sorting없이 출력됩니다.





Return : 

-1
    - 오류가 발생하였으며, 상세한 오류 내용은 errno에 설정된다.

    ENOENT : dirp 디렉토리가 존재하지 않는 path이다.
    ENOMEM : 메모리 부족으로 처리되지 않았습니다.
    ENOTDIR : dirp가 존재는 하나 directory가 아닙니다.

0 이상
    - 정상적으로 처리 되었으며, namelist에 저장된 struct dirent *의 갯수가 return됩니다.

*/





int main(void) 
{
    int* num = malloc(sizeof(int) * 2);
    num[0] = 0; num[1] = 0;

    printf(".\n");
    mytree(num, 1);

    // 마지막 결과 출력
    // directories : num[0]
    // files : num[1]
    printf("\n%d directories, %d files\n", num[0], num[1]);
    return 0;
}




int mytree(int* num, int line) {
    // 파일 정보 구조체
    struct stat fileInfo;
    // 시스템 정보 구조체 (포인터)
    struct passwd * userInfo;
    // Directory 구조체 (포인터)
    DIR * dirp;
    // Directory Entry (포인터)
    struct dirent * dirInfo;


    // 현재 디렉토리 위치
    char cwd[MAX_PATH_LEN + 1];
    memset(cwd, '\0', 1);

    // 현재 작업 디렉토리의 경로를 얻고, 이를 메모리 공간에 저장
    if (getcwd(cwd, MAX_PATH_LEN) == NULL) {
        perror("getcwd() error!");
        exit(-1);
    }

    
    // 현재 디렉토리의 entry 갯수
    int numOfDirent = 0;

    // Directory Entry 갯수 확인
    dirp = opendir(cwd);
    while ((dirInfo = readdir(dirp)) != NULL) {
        // 숨김파일 생략
        if (strncmp(&dirInfo->d_name[0], ".", 1) == 0) continue;
        // 숨김파일이 아닐 경우, directory entry의 갯수 1 증가
        else { numOfDirent++; }
    }


    // 정렬

    // directory entry 정렬 목록
    struct dirent** namelist;
    // return : directory entry 갯수
    int count;

    // if ((count = scandir(cwd, &namelist, NULL, alphasort)) == -1) {
    //     fprintf(stderr, "%s Directory 조회 오류: %s\n", cwd, strerror(errno));
    //     return 1;
    // }
    if ((count = scandir(cwd, &namelist, NULL, compar)) == -1) {
        fprintf(stderr, "%s Directory 조회 오류: %s\n", cwd, strerror(errno));
        return 1;
    }
    



    // 중간 확인
    // printf("Current work directory: %s\n", cwd);
    // printf("numOfDirent: %d\n", numOfDirent);






    // 현재 경로에서 directory pointer 뽑아오기 (정렬로 인한 수정)

    //dirp = opendir(cwd);
    //while ((dirInfo = readdir(dirp)) != NULL) {
    for (int i = 0; i < count; i++) {
        // 숨긴 파일 생략
        //if (strncmp(&dirInfo->d_name[0], ".", 1) == 0) continue;
        if (strncmp(&(namelist[i]->d_name)[0], ".", 1) == 0) continue;

        // while문 돌아갈 때마다 초기화
        int lineCheck = line;
        // 하위 디렉토리 때문에 설정 
        int step = 2;
        // 몇 개의 directory entry가 남았는지...
        numOfDirent--;

        // 출력 전 나무 그리기
        while (lineCheck != 1) {
            // 6칸
            if (lineCheck % 2 == 1) { printf("│     "); }
            else { printf("      "); }
            lineCheck /= 2;
            step *= 2;
        }

        // fileInfo를 가져오기 위한 설정
        int r = 0;
        char tmp[MAX_PATH_LEN + 1];
        memcpy(tmp, cwd, sizeof(cwd));
        strcat(tmp, "/");
        //strcat(tmp, dirInfo->d_name);
        strcat(tmp, namelist[i]->d_name);
        r = stat(tmp, &fileInfo);
        if (r == -1) {
            perror("stat() error!");
            exit(-1);
        }

        // passwd 구조체에 불러오기
        userInfo = getpwuid(fileInfo.st_uid);



        // 1. 나무 그리기
        // 6칸
        if (numOfDirent != 0) { printf("├───  "); }
        else { printf("└───  "); }
        
        // 2. Inode 넘버
        //printf("[%8llu ", dirInfo->d_ino);
        printf("[%8lu ", namelist[i]->d_ino);

        // 3. 디바이스 넘버
        printf("%6ld ", fileInfo.st_dev);

        // 4. Directory(d) or Regular File(-)
        fileType(&fileInfo);

        // 4. 권한
        fileMode(&fileInfo);

        // 5. 소유자
        printf(" %s", userInfo->pw_name);

        // 6. 파일 사이즈
        fileSize(&fileInfo);

        // 7. 파일 이름
        printf("%s\n", namelist[i]->d_name);



        // Directory File 확인 !
        if (S_ISDIR(fileInfo.st_mode)) {
            num[0]++;

            // 하위 디렉토리로 이동
            if (chdir(tmp) == -1) { 
                perror("chdir() error!"); 
                exit(-1);
            }

            if (numOfDirent != 0) { mytree(num, line + step); }
            else { mytree(num, line + step - step / 2); }
        }
        // Regular File 확인 !
        else if (S_ISREG(fileInfo.st_mode)) { num[1]++; }
        else { perror("directory X && regular X"); exit(-1); }
    }



    // free
    for (int i = 0; i < count; i++) { free(namelist[i]); }
    free(namelist);



    // 해당 경로의 directory를 닫아준다.
    closedir(dirp);
    return 0;
}








void fileSize(const struct stat *fileInfo) {
    int size = fileInfo->st_size;

    if (size < 1000) {
        printf("%8d]  ", size);
    } 
    else {
        double ssize = size;
        ssize /= 1000;
        printf("%7.1fK]  ", ssize);
    }
}



void fileType(const struct stat *fileInfo) { 
    if (S_ISREG(fileInfo->st_mode)) {
        printf("-");
    } 
    else if (S_ISDIR(fileInfo->st_mode)) {
        printf("d");
    } 
    else if (S_ISLNK(fileInfo->st_mode)) {
        printf("l");
    } 
    // else if (S_ISSOCK(fileInfo->st_mode)) {
    //     printf("s");
    // } 
    // else if (S_ISFIFO(fileInfo->st_mode)) {
    //     printf("p");
    // } 
    // else if (S_ISCHR(fileInfo->st_mode)) {
    //     printf("c");
    // } 
    // else if (S_ISBLK(fileInfo->st_mode)) {
    //     printf("b");
    // }
    // else if (S_TYPEISMQ(fileInfo->st_mode)) {
    //     printf("");
    // } 
    // else if (S_TYPEISSEM(fileInfo->st_mode)) {
    //     printf("");
    // } 
    // else if (S_TYPEISSHM(fileInfo->st_mode)) {
    //     printf("");
    // }
    else { 
        perror("Directory X && Regular X"); 
        exit(-1); 
    }
}






void fileMode(const struct stat *fileInfo)
{
    // User
    if (S_IRUSR & fileInfo->st_mode) { 
        // printf("%d", S_IRUSR & fileInfo->st_mode);
        printf("r"); 
    } 
    else { 
        // printf("%d", S_IRUSR & fileInfo->st_mode);
        printf("-"); 
    }

    if (S_IWUSR & fileInfo->st_mode) { 
        // printf("%d", S_IWUSR & fileInfo->st_mode);
        printf("w"); 
    } 
    else { 
        // printf("%d", S_IWUSR & fileInfo->st_mode);
        printf("-"); 
    }

    if (S_IXUSR & fileInfo->st_mode) { 
        // printf("%d", S_IXUSR & fileInfo->st_mode);
        printf("x"); 
    }
    else { printf("-"); }

    // Group
    if (S_IRGRP & fileInfo->st_mode) {
        // printf("%d", S_IRGRP & fileInfo->st_mode);
        printf("r"); 
    } 
    else { 
        // printf("%d", S_IRGRP & fileInfo->st_mode);
        printf("-"); 
    }

    if (S_IWGRP & fileInfo->st_mode) { 
        // printf("%d", S_IWGRP & fileInfo->st_mode);
        printf("w" ); 
    }
    else { 
        // printf("%d", S_IWGRP & fileInfo->st_mode);
        printf("-"); 
    }

    if (S_IXGRP & fileInfo->st_mode) { 
        // printf("%d", S_IXGRP & fileInfo->st_mode);
        printf("x"); 
    }
    else { 
        // printf("%d", S_IXGRP & fileInfo->st_mode);
        printf("-"); 
    }

    // Others
    if (S_IROTH & fileInfo->st_mode) { 
        // printf("%d", S_IROTH & fileInfo->st_mode);
        printf("r"); 
    } 
    else { 
        // printf("%d", S_IROTH & fileInfo->st_mode);
        printf("-"); 
    }

    if (S_IWOTH & fileInfo->st_mode) { 
        // printf("%d", S_IWOTH & fileInfo->st_mode);
        printf("w"); 
    }
    else { 
        // printf("%d", S_IWOTH & fileInfo->st_mode);
        printf("-"); 
    }

    if (S_IXOTH & fileInfo->st_mode) { 
        // printf("%d", S_IXOTH & fileInfo->st_mode);
        printf("x"); 
    } 
    else { 
        // printf("%d", S_IXOTH & fileInfo->st_mode);
        printf("-"); 
    }
}


int compar(const struct dirent ** dirInfo1, const struct dirent ** dirInfo2) 
{
    return strcmp((*dirInfo1)->d_name, (*dirInfo2)->d_name);
}

