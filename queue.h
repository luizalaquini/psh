#ifndef TRABALHOSO_QUEUE_H
#define TRABALHOSO_QUEUE_H

/* Tipo Opaco */
typedef struct queue Queue;

/* INSERE NO FIM DA FILA
* INPUTs: Conteúdo e fila onde deve ser inserido.
* OUTPUTs: Fila.
* Pré-condição: Fila deve existir e conteúdo deve ser diferente de NULL.
* Pós-condição: Fila modificada.
*/
Queue *push(struct queue *queue, int content);

/* REMOVE O PRIMEIRO DA FILA 
* INPUTs: Fila de onde vai se remover o conteúdo.
* OUTPUTs: Fila com conteúdo removido.
* Pré-condição: Fila deve existir e possuir pelo menos um conteúdo.
* Pós-condição: Fila modificada.
*/
Queue *pop(Queue *queue);

/* RETORNA O PRIMEIRO DA FILA 
* INPUTs: Fila a ser analisada.
* OUTPUTs: Primeiro da fila. 
* Pré-condição: Fila deve existir e ser diferente de NULL.
* Pós-condição: -
*/
int first(Queue *pQueue);

/* RETORNA VERDADEIRO SE A FILA ESTIVER VAZIA
* INPUTs: Fila a ser analisada. 
* OUTPUTs: Verdadeiro ou Falso.
* Pré-condição: Fila deve existir.
* Pós-condição: -
*/
int is_empty(Queue *queue);

#endif /* TRABALHOSO_QUEUE_H */
