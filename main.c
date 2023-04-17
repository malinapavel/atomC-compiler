#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "library_compiler.h"

int main(int argc, char **argv) {

    if (argc != 2){
        printf("Please provide a file name");
        exit(-1);
    }


    int source_fd = open(argv[1], O_RDONLY);
    if (source_fd < 0){
        printf("Could not open file");
        exit(-1);
    }


    struct stat st;
    int file_size;
    if(stat(argv[1], &st)){
        printf("Could not process file stats");
        exit(-1);
    }
    else file_size = st.st_size;


    char buffer[file_size * sizeof(char) - 1];
    int bytes_read;
    if( (bytes_read = read(source_fd, buffer, file_size * sizeof(char))) < 0 ){
        printf("Could not read file contents");
        exit(-1);
    }

    if(bytes_read < file_size)  buffer[bytes_read] = '\0';
    else buffer[file_size] = '\0';

    //printf("%s", buffer);
    int lexical = next_token(buffer);
    int syntactical = unit();
    //printf("unit value: %d \n", syntactical);

    if(lexical !=0 && syntactical != 0){
        printf("%s\n", "\n~ The code was successfully analyzed!");
    }


    if(close(source_fd) < 0){
        printf("Could not close file");
        exit(-1);
    }

}
