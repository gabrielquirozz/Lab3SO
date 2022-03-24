#include <pthread.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h>  
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <omp.h>
#include <stdbool.h>

int sudoku[9][9];
bool Pflag = true;


void *columnValidator(){  

    int numeros[10]={0};
    int i, j, m, b, k, l;
    int a =0;
    printf("Thread de ejecucion de columnas: %d\n", syscall(SYS_gettid));
    omp_set_nested(true);
    omp_set_num_threads(9); 
    #pragma omp parallel for private(a) //schedule(dynamic)
    for(i =0; i<9; i++){
        printf("En la revision de columnas el thread en ejecucion es: %d\n", syscall(SYS_gettid)); 
        for ( j=0; j<9; j++){
            numeros[sudoku[j][i]]++;
        }
        for(k = 1; k<=9; k++){
            if(numeros[k]!=1){
                Pflag = false;
            }
        }
        for(l=1; l<=9;l++){
            numeros[l]=0;
        }
    }
    pthread_exit(0);
}

bool rowValidator(){
    
    bool banderaR = true;
    int numeros[10]={0};
    int i, j, m, b, k, l;
    omp_set_nested(true);
    omp_set_num_threads(9); 
    int a = 0;
    #pragma omp parallel for private(a) schedule(dynamic)
    for(i =0; i<9; i++){

        for (j=0; j<9; j++){
            numeros[sudoku[i][j]]++;
        }
        for(k = 1; k<=9; k++){
            if(numeros[k]!=1){
                banderaR = false;
            }
        }
        for(l=1; l<=9;l++){
            numeros[l]=0;
        }
    }
    return(banderaR);
}


bool threeXthreeValidator(){

    int numeros[10]={0};
    int count=0;
    int i, j, m, b, k;
    int a = 0;
    bool bandera = true;

    omp_set_nested(true); 
    omp_set_num_threads(9); 
    #pragma omp parallel for private(a) schedule(dynamic)
    for(m=0;m<9;m+=3){
        for(b=0;b<9;b+=3){
            for(i=m;i<m+3;i++){
                for(j=b;j<b+3;j++){
                    numeros[sudoku[i][j]]++;
                }
            }
            count++;
            for( k=1;k<=9;k++){
                if(numeros[k]!=1){
                    bandera = false;
                }
            }
            for(k=1;k<=9;k++){
                numeros[k]=0;
            }
        }
    }
    return bandera;
}


int main(int argc, char* argv[]){ 
    omp_set_nested(true);
    omp_set_num_threads(1); 
    int i, j, m, b, k;


    int fd = open(argv[1], O_RDONLY, 0666); 
    struct stat s;
    int status = fstat (fd, & s);
    int size = s.st_size;
    char *f = (char *) mmap (0, size, PROT_READ, MAP_PRIVATE, fd, 0);

    for ( i = 0; i < 81; i++)
    {
        sudoku[i / 9][i % 9] = f[i] - '0';
    }

    pid_t parent_pid = getpid();

    if (fork() == 0) {
        char p_pid[20];
        sprintf(p_pid, "%d", (int) parent_pid);
        execlp("ps","ps","-p", p_pid, "-lLf", NULL);
    }else{
        pthread_t thread1;
        pthread_create(&thread1, NULL, columnValidator, NULL);
        pthread_join(thread1, NULL);
        printf("En el main trabaja el thread numero: %d\n", syscall(SYS_gettid));
        wait(NULL);

        bool k1= rowValidator();
        bool k2= threeXthreeValidator();
        
        if (Pflag && k2 ){
            printf("Sudoku validado!\n");
        }else{
            printf("Sudoku no validado!\n");
        }

        if(fork()==0){
            char p_pid[20];
            sprintf(p_pid, "%d", (int) parent_pid);
            printf("Antes de terminar el estado de este proceso y sus threads es:\n");
            execlp("ps","ps","-p", p_pid, "-lLf", NULL);
        }else{
            wait(NULL);
            return (0);

        }

    }

}
