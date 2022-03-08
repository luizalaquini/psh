#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "psh.h"
#include "queue.h"
#include <fcntl.h>

#define TRUE 1
#define PSH_TOK_BUFSIZE 64
#define PSH_TOK_DELIMITADOR " \t\r\n\a"
#define PSH_ARGS_NUMBER 10

// Variáveis Globais
int pipePidVaccinated[2];
int pipe_last_proc[2];
int pgid_vacinados_global = -1;
int is_first_vaccinated_proc = 0;
Queue *queue_unvaccinated = NULL;

// Declara funções auxiliares que vão funcionar somente aqui dentro de "psh.c":

/*Mata todos os processos do grupo de NÃO VACINADOS
* INPUTs: Sinal.
* OUTPUTs: -
* Pré-requitos: Devem existir processos não vacinados.
* Pós-requisitos: Processos não vacinados não existem mais.
*/
void kill_unvaccinated(int signal);

/* Mata todos os processos do grupo de VACINADOS
* INPUTs: Sinal.
* OUTPUTs: -
* Pré-requitos: Devem existir processos vacinados.
* Pós-requisitos: Processos vacinados não existem mais.
*/
void kill_vaccinated(int signal);

/* Recebe um dos 3 sinais (SIGINT, SIGQUIT ou SIGTSTP) e imprime mensagem 
* INPUTs: Um dos 3 sinais.
* OUTPUTs: Mensagem no terminal.
* Pré-requitos: Sinal deve ser capturado.
* Pós-requisitos: -
*/
void handle_sigvacinado(int sig);

/* Recebe matriz de processos e retorna verdadeiro na presença do operador ">"
* INPUTs: Matriz de processos.
* OUTPUTs: Verdadeiro ou falso.
* Pré-requitos: Matriz de processos deve existir.
* Pós-requisitos: -
*/
int exist_output_token(char **proc);

/* Recebe uma string com processo + operador + arquivo de redirecionamento e limpa a string deixando somente o processo
* Exemplo: "ls > saida.txt" passa a ser somente "ls" 
* INPUTs: String e nome do arquivo de redirecionamento.
* OUTPUTs: Nome do processo.
* Pré-requitos: String deve ser diferente de NULL.
* Pós-requisitos: -
*/
char **clean_proc(char **proc, char **file);

/* Redireciona saída da psh para arquivo recebido como parâmetro (nome)
* INPUTs: Nome do arquivo de redirecionamento.
* OUTPUTs: -
* Pré-requitos: Arquivo deve existir.
* Pós-requisitos: -
*/
void psh_redirect(char* file);

/* Imprime mensagem com desenho do Gandalf
* INPUTs: -
* OUTPUTs: Mensagem.
* Pré-requitos: -
* Pós-requisitos: -
*/
void print_gandalf();

/* Transforma um vetor de palavras em uma matriz de comandos
* INPUTs: Quantidade de processos, vetor de palavras e total de palavras do vetor.
* OUTPUTs: Estrutura de processos.
* Pré-requitos: Vetor de palavras deve existir e ser diferente de NULL.
* Pós-requisitos: Matriz de comandos existe.
*/
Processos *convert_tokens_to_procs(int bufsize, int total_words, char *const *tokens, int qtd_procs);

/* Encapsula as intruções para execução dos processos VACINADOS
* INPUTs: Matriz de processos vacinados, quantidade de processos e index.
* OUTPUTs: -
* Pré-requitos: Matriz deve existir.
* Pós-requisitos: -
*/
void launch_vaccinated_proc(char **const *matriz, int i , int qtd_procs);

/* Encapsula as intruções para execução dos processos NÃO VACINADOS
* INPUTs: Matriz de processos não vacinados.
* OUTPUTs: -
* Pré-requitos: Matriz deve existir.
* Pós-requisitos: -
*/
void launch_unvaccinated_proc(char **const *matriz);

//================ PRINCIPAIS FUNÇÕES ================ 

char *psh_read_line(void) {
    // Inicializando variáveis necessárias
    char *line = NULL;
    size_t bufsize = 0;

    // Casos de erro
    if (getline(&line, &bufsize, stdin) == -1){
        if (feof(stdin)) {
            exit(EXIT_SUCCESS); // EOF
        }
        else  {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }

    // Se tudo correto, a linha é lida e retornada
    return line;
}

Processos *psh_split_line(char *line) {

    if (strcmp(line, "\n") == 0) return NULL;

    // Inicializando e alocando variáveis necessárias
    int bufsize = PSH_TOK_BUFSIZE;
    int total_words = 0;
    char **tokens = malloc(bufsize * sizeof(char*)); // matriz contendo em cada posição as palavras separadas por espaço
    char *token;

    if (!tokens) { // Caso erro de alocação
        fprintf(stderr, "psh: allocation error (tokens)\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, PSH_TOK_DELIMITADOR); // divisão

    int qtd_procs = 1; // conta o numero de processos por linha (max 5);

    while (token != NULL) {
        // Organização das instruções no vetor de strings
        tokens[total_words] = token;
        if(strcmp(token, ";") == 0){
            qtd_procs++; // incremento no num de processos ao encontrar um ";"
        }
        total_words++;

        if (total_words >= bufsize) {  // Necessidade de realocar aumentando a quantidade de caracteres
            // no vetor de strings para as instruções
            bufsize += PSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) { // caso de erro de realocação
                fprintf(stderr, "psh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, PSH_TOK_DELIMITADOR);
    }

    return convert_tokens_to_procs(bufsize, total_words, tokens, qtd_procs);
}

Processos *convert_tokens_to_procs(int bufsize, int total_words, char *const *tokens, int qtd_procs) {
    char ***matriz_proc = (char ***) malloc(qtd_procs * sizeof(char**)); // matriz contendo em cada posição um comando
    char* token;
 
    char **proc = malloc(PSH_ARGS_NUMBER * sizeof(char*));
    int j = 0; //contador de palavras em um processo
    int k = 0; //contador de processos do vetor de processos
    for (int i = 0; i < total_words; ++i) {
        if (strcmp(tokens[i], ";") != 0) {
            token = (char *) malloc(bufsize * sizeof(char));
            strcpy(token, tokens[i]);
            proc[j++] = token;
        } else {
            matriz_proc[k++] = proc;
            proc = malloc(PSH_ARGS_NUMBER * sizeof(char*));
            j = 0;
        }
    }
    matriz_proc[k] = proc;

    return initProc(qtd_procs, matriz_proc);
}

int psh_launch(Processos *proc) {
    char ***matriz = getMatriz(proc);

    int qtd_procs = getQtdProcs(proc); // tamanho do vetor de referências à processos / tamanho de uma referência de getQtdProcs(proc)

    if (qtd_procs > 1) { // Para grupo de processos VACINADOS
        for (int i = 0; i < qtd_procs; ++i) {
            launch_vaccinated_proc(matriz, i, qtd_procs);
        }
    } else { // Para grupo de processos NÃO VACINADOS 
        launch_unvaccinated_proc(matriz);
    }

    return TRUE;
}

void launch_vaccinated_proc(char **const *matriz, int i, int qtd_procs) {
    pid_t pid;
    
    if (matriz[i] != NULL) {
    pid_t pgid_vacinados = 0;
        pid = fork();
        if (pid == -1) perror("psh");  // Caso erro de fork
        else if (pid == 0) { // Código do processo FILHO

            if(is_first_vaccinated_proc == 1 && i==0){ // entra apenas no 1º processo filho vacinado, para que o pgid dos vacinados seja o id deste processo
                close(pipePidVaccinated[0]);
                pgid_vacinados = getpid();
                write(pipePidVaccinated[1], &pgid_vacinados, sizeof(pgid_vacinados));
                close(pipePidVaccinated[1]);
            } else { // resto dos filhos apenas le do proprio
                read(pipePidVaccinated[0], &pgid_vacinados, sizeof(pgid_vacinados));
                write(pipePidVaccinated[1], &pgid_vacinados, sizeof(pgid_vacinados));
                close(pipePidVaccinated[0]);
                close(pipePidVaccinated[1]);
            }

            if (i == qtd_procs - 1) {
                int is_last_proc = 1;
                close(pipe_last_proc[0]);
                write(pipe_last_proc[1], &is_last_proc, sizeof(int));
                close(pipe_last_proc[1]);
            }
            setpgid(getpid(), pgid_vacinados);

            //Ignorando sinais para processos não vacinados
            signal(SIGINT, SIG_IGN); /* Ctrl + C */
            signal(SIGQUIT, SIG_IGN); /* Ctrl + \ */
            signal(SIGTSTP, SIG_IGN); /* Ctrl + Z */

            if (execvp(matriz[i][0], matriz[i]) == -1) {
                perror("psh");
            }

        } else {//pai
            /*Tentamos fazer um pipe para ler o pgid do grupo de vacinados para o pai, ele funciona, passa os processos vacinados para foreground e a bash para back. Mas, os pipes feitos a seguir interferem no nosso pipe para definir o pgid dos proximos processos vacinados. Não tivemos tempo para consertar esse erro. */

            // if (i == qtd_procs - 1) {
            //     int last_proc_signals = 0;
            //     int pgid;
            //     close(pipe_last_proc[1]);
            //     // while(last_proc_signals != 1) {
            //         read(pipe_last_proc[0], &last_proc_signals, sizeof(int));
            //     // }
                
            //     printf("LAST PROC SIGNALS: %d\n", last_proc_signals);
            //     if (last_proc_signals == 1) {
            //         read(pipePidVaccinated[0], &pgid, sizeof(int));
            //         write(pipePidVaccinated[1], &pgid, sizeof(int));
            //         close(pipePidVaccinated[0]);
            //         close(pipePidVaccinated[1]);
            //         pgid_vacinados_global = pgid;
            //         printf("VACINADOS GLOBAL: %d\n", pgid_vacinados_global);
            //     }
            //     close(pipe_last_proc[0]);
            // }

        }
    }
}

void launch_unvaccinated_proc(char **const *matriz) {
    pid_t pid;
    char **processo_n_vacinado = matriz[0];
    if (processo_n_vacinado != NULL) {

        //int status;
        pid = fork();
        if (pid == -1) perror("psh");  // Caso erro de fork
        else if (pid == 0) { // Código do processo FILHO
            setpgid(getpid(), getpid()); // o próprio id do processo será o seu id do grupo

            int exist = exist_output_token(processo_n_vacinado);

            char *file = NULL;
            //Quando há um redirecionamento de saida no processo, precisamos retirar o simbolo para redirecionar e o arquivo de saida
            if(exist){
                char **processo_redirecionar = clean_proc(processo_n_vacinado, &file);
                psh_redirect(file);
                if (execvp(processo_redirecionar[0], processo_redirecionar) == -1) {
                    perror("psh");
                }
                destroy_proc(processo_redirecionar);
            } else { // execução para todo processo sem redirecionamento
                if (execvp(processo_n_vacinado[0], processo_n_vacinado) == -1) {
                    perror("psh");
                }
            }


            exit(EXIT_FAILURE);
        } else { // Código do processo PAI
            //printf("pid filho nao vacinado: %d\n", pid);
            queue_unvaccinated =  push(queue_unvaccinated, pid);
        }
    }
}

int psh_execute(Processos *args) {

    if (args  == NULL) { // Caso comando "vazio"
        return 1;
    }

    return psh_launch(args);
}

void psh_loop(void) {
    char *line;
    Processos *args; //matriz 3
    int statusLoop;

    if(pipe(pipePidVaccinated) < 0){
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    if(pipe(pipe_last_proc) < 0){
        perror("pipe");
        exit(EXIT_FAILURE);
    }

   // Lida com sinais e imprime mensagens
   signal(SIGINT,  handle_sigvacinado); /* Ctrl + C */
   signal(SIGQUIT, handle_sigvacinado); /* Ctrl + \ */
   signal(SIGTSTP, handle_sigvacinado); /* Ctrl + Z */
   signal(SIGUSR1,  handle_sigvacinado); 

    do {
        int status;

        // VE SE ALGUM FILHO MORREU DE SIGUSR1
        waitpid(-1, &status, WNOHANG);

        if (WTERMSIG(status) == SIGUSR1){
            //printf("ENTREI COMO SIGUSR1!!\n");
            kill_unvaccinated(SIGUSR1);
            kill_vaccinated(SIGUSR1);
            print_gandalf(); // imprime o gandalf
        }

        printf("psh> ");
        line = psh_read_line(); // leitura da linha

        args = psh_split_line(line); // separa instruções

        if(args == NULL) continue;
        
        //Definindo o pid do primeiro comando do grupo de vacinados como pgid do grupo
        if(getQtdProcs(args) > 1){
            is_first_vaccinated_proc++;
        }

        //COMANDO TERM:
        if (getQtdProcs(args) == 1 && (strcmp(getMatriz(args)[0][0], "term")==0)){
            kill_unvaccinated(SIGTERM);
            kill_vaccinated(SIGTERM);
            exit(1);
        }

        //COMANDO FG:
        if (getQtdProcs(args) == 1 
            && (strcmp(getMatriz(args)[0][0], "fg")==0) 
            && pgid_vacinados_global != -1){
            //printf("sou o pgid dos vacs: %d\n", pgid_vacinados_global);
            tcsetpgrp(0, pgid_vacinados_global); // coloca o grupo de processos vacinados em foreground

            sleep(30); //30 segundos de espera

            signal(SIGTTOU, SIG_IGN); // ignora SIGTTOU
            tcsetpgrp(0, getppid()); // coloca o grupo de processos vacinados em background
            signal(SIGTTOU, SIG_DFL); // SIGTTOU volta para default

            free(line);
            destroy_matrix(args);
            continue;
        
        }

        statusLoop = psh_execute(args); // executa instruções
        

        free(line);
        destroy_matrix(args);
        args = NULL;

    } while (statusLoop);
}

//================ FUNÇÕES AUXILIARES ================ 


void kill_unvaccinated(int signal) {
    while (!is_empty(queue_unvaccinated)) {
        int pid_unvaccinated = first(queue_unvaccinated);
        queue_unvaccinated = pop(queue_unvaccinated);
        //print_queue(queue_unvaccinated);
        kill(pid_unvaccinated, signal);
        //printf("matou o %d\n", pid_unvaccinated);
    }
}

void kill_vaccinated(int signal) {
    int pid_vaccinated;
    close(pipePidVaccinated[1]);
    read(pipePidVaccinated[0], &pid_vaccinated, sizeof(int));
    close(pipePidVaccinated[0]);
    //printf("PID VACCINATED PARA MATAR:%d\n", pid_vaccinated);
    killpg(pid_vaccinated, signal);
}

void handle_sigvacinado(int sig) {
    printf("Estou vacinada… desista!!\n");
    printf("psh> ");
    fflush(stdout);
}

int exist_output_token(char **proc) {
    // echo ; echo2 > output.txt ; echo 3
    char *find_char = NULL;
    int j = 0;
    while ((find_char = proc[j]) != NULL){
        if (strcmp(find_char, ">") == 0) {
            return 1;
        }
        j++;
    }
    return 0;
}

char **clean_proc(char **proc, char **file) {
    int sizeof_proc = 0;

    while(proc[sizeof_proc] != NULL) sizeof_proc++;

    int sizeof_newproc = sizeof_proc - 2;
    char **str_return = (char **) malloc(sizeof_newproc * sizeof(char *));

    for (int j = 0; j < sizeof_newproc; j++) str_return[j] = strdup(proc[j]);
    *file = strdup(proc[sizeof_proc - 1]);

    return str_return;
}

void psh_redirect(char* file){
    int fd = open(file,O_WRONLY|O_CREAT|O_TRUNC,0644); //fopen ("saida.txt", "w");

    if((fd == -1) | (dup2(fd, STDOUT_FILENO) == -1)){
        perror("psh");
        exit(EXIT_FAILURE);
    }
}

void print_gandalf(){
    printf(
            "                       ,---.\n"
            "                       /    |\n"
            "                      /     |\n"
            "                     /      |\n"
            "                    /       |\n"
            "               ___,'        |\n"
            "             <  -'          :\n"
            "              `-.__..--'``-,_\\_\n"
            "                 |o/ ` :,.)_`>\n"
            "                 :/ `     ||/)\n"
            "                 (_.).__,-` |\\ \n"
            "                 /( `.``   `| :\n"
            "                 \'`-.)  `  ; ;\n"
            "                 | `       /-<\n"
            "                 |     `  /   `.\n"
            " ,-_-..____     /|  `    :__..-'\\ \n"
            "/,'-.__\\  ``-./ :`      ;       \\ \n"
            "`\\ `\\  `\\  \\ :  (   `  /  ,   `. \\ \n"
            "  \\` \\   \\   |  | `   :  :     .\\ \\ \n"
            "   \\ `\\_  ))  :  ;     |  |      ): :\n"
            "  (`-.-'\\ ||  |\\ \\   ` ;  ;       | |\n"
            "   \\-_   `;;._   ( `  /  /_       | |\n"
            "    `-.-.// ,'`-._\\__/_,'         ; |\n"
            "       \\:: :     /     `     ,   /  |\n"
            "        || |    (        ,' /   /   |\n"
            "        ||                ,'   /    |\n"
            "________ Unfortunately all process died!________\n"
            "___ Vaccination should be a social contract!____\n"
            "____Cooperation was the morally right choice!___\n"
    );
}