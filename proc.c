#include <stdio.h>
#include <stdlib.h>
#include "proc.h"

// ESTRUTURA

struct processos {
    char*** matriz; // vetor de processos (lista de comandos)
    int qtd_procs; // número de processos na matriz
};

// SETs e GETs

void setQtdProcs (Processos *processos, int qtd){
    processos->qtd_procs = qtd;
}

int getQtdProcs (Processos *processos){
    return processos->qtd_procs;
}

void setMatriz (Processos *processos, char ***matriz){
    processos->matriz = matriz;
}

char ***getMatriz (Processos *processos){
    return processos->matriz;
}

// FUNÇÕES

Processos *initProc(int qtd_p, char ***matriz){
    Processos *new = (Processos *) malloc(sizeof(Processos)); // aloca

    //recebe parâmetros
    new->qtd_procs = qtd_p;
    new->matriz = matriz;

    return new;
}

void destroy_matrix(Processos *processos) {
    char*** matriz = processos->matriz;
    for (int i = 0; i < processos->qtd_procs; i++) {
        destroy_proc(matriz[i]);
    }
    free(processos);
}

void destroy_proc(char ** processo) {
    int j = 0;
    char* palavra = processo[j];
    while(palavra != NULL) {
        free(palavra);
        j++;
        palavra = processo[j];
    }
    //free(processo); // dando double free
}