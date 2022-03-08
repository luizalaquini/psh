#ifndef PROC_H
#define PROC_H

/* Tipo opaco */
typedef struct processos Processos;

// GETs and SETs

void setMatriz (Processos *proc, char ***matriz);
char ***getMatriz (Processos *proc);

void setQtdProcs (Processos *proc, int qtd);
int getQtdProcs (Processos *proc);

// FUNÇÕES

/* Inicializa estrutura com vetor de comandos e número de comandos presentes no vetor(inteiro) 
* INPUTs: Quantidade de processos e vetor de processos.
* OUTPUTs: Estrutura.
* Pré-condição: Vetor de processos deve existir.
* Pós-condição: Estrutura existe. 
*/
Processos *initProc(int qtd_p, char ***matriz);

/* Libera memória alocada pra matriz de processos 
* INPUTs: Estrutura que contém a matriz a ser liberada. 
* OUTPUTs: -
* Pré-condição: Matriz deve existir na estrutura. 
* Pós-condição: Matriz não existe mais na estrutura.
*/
void destroy_matrix(Processos *proc);

/* Libera memória alocada para um processo na matriz
* INPUTs: Processo a ser liberado.
* OUTPUTs: -
* Pré-condição: Processo deve existir.
* Pós-condição: Processo não existe mais.
*/
void destroy_proc(char **processo);

#endif /* PROC_H */