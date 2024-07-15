#include<sys/stat.h>
#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<stdint.h>

char filename[] = "t.txt";

FILE *file = NULL;

char buffer[6];

int openFile(){
    file = fopen(filename, "rb");
    if(file == NULL){
        perror("open error file");
        return -1;
    }
}

int main(){
    openFile();

    int byte_read;

    while ((byte_read = fread(buffer, sizeof(uint8_t), 2, file))>0)
    {
        printf("字节数，%d\n", byte_read);
        buffer[5] = '\0';
        printf("%s\n", buffer);
    }
    return 0;
}