#ifndef PSH_H
#define PSH_H

#include "proc.h"
#include "queue.h"

/* LEITURA DA LINHA DE COMANDO
* Input(s): -
* Output(s): Linha lida 
* Pré-condição: Inicialização da shell e entrada no loop de funcionamento.
* Pós-condição: A linha de comando foi lida e retornada. Caso contrário é exibida uma mensagem de erro.
*/
char *psh_read_line(void);

/* SEPARAÇÃO DOS COMANDOS A PARTIR DA LINHA LIDA 
* Input(s): Linha lida
* Output(s): Vetor de strings contendo em cada posição um comando 
* Pré-condição: Linha lida deve existir
* Pós-condição: Vetor de comandos existe
*/
Processos *psh_split_line(char *line);

int psh_launch(Processos *proc);

/* EXECUCÃO DOS COMANDOS A PARTIR DO VETOR DE STRINGS GERADO PELA FUNCAO "psh_split_line"
* Input(s): Linha lida
* Output(s): Vetor de strings contendo em cada posição um comando 
* Pré-condição: Linha lida deve existir
* Pós-condição: Vetor de comandos existe
*/
int psh_execute(Processos *args);

/* LOOP DE FUNCIONAMENTO DA SHELL
* Input(s): -
* Output(s): -
* Pré-condição: Inicialização da shell.
* Pós-condição: A shell inicializou e rodou.
*/
void psh_loop(void);

#endif /* PSH_H */
