//
// Created by ryan on 01/03/2022.
//
#include <stdio.h>
#include "stdlib.h"
#include "queue.h"

struct queue {
    struct queue *next; // ponteiro para o próximo da fila
    int content; // conteúdo
};

Queue* push(Queue *queue, int content) {
    // printf("entrou na fila com o PID: %d\n", content);
    Queue *new = (Queue *) malloc(sizeof(Queue));
    new->content = content;
    // printf("como ta na fila: %d\n", new->content);
    new->next = NULL;

    //na primeira entrada a queue vai sempre estar vazia 
    if (is_empty(queue)==1) { // fila vazia
        queue = new;
    } else {
        Queue *p;
        Queue *anterior;

        for (p = queue; p != NULL; p = p->next) { // procura o final da fila
            anterior = p;
        } 
        anterior->next = new;
    }
    //print_queue(queue);
    return queue;
}

Queue *pop(Queue *queue) {
    Queue *new = queue;
    queue = new->next;
    free(new);

    return queue;
}

int first(Queue *pQueue) {
    return pQueue->content;
}

int is_empty(Queue *queue) {
    return queue == NULL ? 1 : 0;
}
