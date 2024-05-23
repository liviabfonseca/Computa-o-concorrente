#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_NUM 10000000 //Número máximo a ser considerado primo

// Variáveis globais
sem_t slotCheio, slotVazio;  //Semaforos para sincronização por condição
sem_t mutexGeral; //Semaforo para sincronização entre produtores e consumidores

int *buffer;
int tam_buffer;
int in = 0, out = 0;
int *cont_primo;
int num_consumidores;
int total_primos_arquivo = 0; //Numero total de primos no arquivo

// Função para verificar se um número é primo
int ehPrimo(long long int n) {
    int i;
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (i = 3; i <= sqrt(n); i += 2)
        if (n % i == 0) return 0;
    return 1;
}

// Função para inserir um elemento no buffer
void Insere(int item) {
    sem_wait(&slotVazio); //aguarda slot vazio para inserir
    sem_wait(&mutexGeral); //exclusão mútua
    buffer[in] = item;
    in = (in + 1) % tam_buffer;
    sem_post(&mutexGeral);
    sem_post(&slotCheio); // Sinaliza um slot cheio
}

// Função para retirar um elemento do buffer
int Retira() {
    int item;
    sem_wait(&slotCheio); // Aguarda slot cheio para retirar
    sem_wait(&mutexGeral); //Exclusão mutua
    item = buffer[out];
    out = (out + 1) % tam_buffer;
    sem_post(&mutexGeral);
    sem_post(&slotVazio); // Sinaliza um slot vazio
    return item;
}

// Produtor
void *produtor(void *arg) {
    char *filename = (char *)arg;
    FILE *file = fopen(filename, "rb");  //"rb" - leitura binária
    if (!file) {
        perror("Erro ao abrir o arquivo de entrada");
        exit(EXIT_FAILURE);
    }

    int num;
    while (fread(&num, sizeof(int), 1, file) == 1) {
        Insere(num);
    }

    fclose(file);

    // Sinalizar fim da produção para consumidores
    for (int i = 0; i < num_consumidores; i++) {
        Insere(-1);
    }

    pthread_exit(NULL);
}

// Consumidor
void *consumidor(void *arg) {
    int id = *(int *)(arg);
    free(arg);

    int primos = 0;
    int num;

    while (1) {
        num = Retira();
        if (num == -1) break; //Verifica o termino 

        if (ehPrimo(num)) {
            primos++;
        }
    }

    cont_primo[id] = primos;
    printf("Thread consumidora %d encontrou %d números primos.\n", id, primos);

    pthread_exit(NULL);
}

// Função para ler o número total de primos do arquivo
int lerTotalPrimos(const char *filename) {
    FILE *file = fopen(filename, "rb");  //"rb" - leitura binária
    if (!file) {
        perror("Erro ao abrir o arquivo de entrada");
        exit(EXIT_FAILURE);
    }

    // Ponteiro do arquivo no final para ler o total de num primos
    fseek(file, -sizeof(int), SEEK_END);
    int total_primos;
    fread(&total_primos, sizeof(int), 1, file);
    fclose(file);
    return total_primos;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <arquivo_de_entrada> <quantidade_de_threads_consumidoras> <tamanho_do_buffer>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *filename = argv[1]; // arquivo de entrada
    num_consumidores = atoi(argv[2]); // Quantidade de threads consumidoras
    tam_buffer = atoi(argv[3]); // Tamanho do buffer

    buffer = malloc(tam_buffer * sizeof(int));
    if (!buffer) {
        perror("Erro ao alocar memória para o buffer");
        exit(EXIT_FAILURE);
    }

    cont_primo = calloc(num_consumidores, sizeof(int));
    if (!cont_primo) {
        perror("Erro ao alocar memória para cont_primo");
        exit(EXIT_FAILURE);
    }

    sem_init(&mutexGeral, 0, 1); // Binário
    sem_init(&slotCheio, 0, 0); // Contador
    sem_init(&slotVazio, 0, tam_buffer); // Contador

    total_primos_arquivo = lerTotalPrimos(filename);

    pthread_t prod_thread;

    // Verificando o retorno da função PTHREAD_CREATE
    if (pthread_create(&prod_thread, NULL, produtor, filename) != 0) {
        perror("Erro ao criar thread de produtor");
        exit(EXIT_FAILURE);
    }

    pthread_t cons_threads[num_consumidores];
    int *cons_ids[num_consumidores];

    for (int i = 0; i < num_consumidores; i++) {
        cons_ids[i] = malloc(sizeof(int));
        *cons_ids[i] = i;

        //Verificando o retorno da função PTHREAD_CREATE
        if (pthread_create(&cons_threads[i], NULL, consumidor, cons_ids[i]) != 0) {
            perror("Erro ao criar thread de consumidor");
            exit(EXIT_FAILURE);
        }
    }

    pthread_join(prod_thread, NULL);

    for (int i = 0; i < num_consumidores; i++) {
        pthread_join(cons_threads[i], NULL);
    }

    //Descobrindo a quantidade total de números primos e a thread vencedora
    int total_primos = 0;
    int max_primos = 0;
    int ganhadora = -1;
    for (int i = 0; i < num_consumidores; i++) {
        total_primos += cont_primo[i];
        if (cont_primo[i] > max_primos) {
            max_primos = cont_primo[i];
            ganhadora = i;
        }
    }

    // Printa qual é a thread vencedora
    puts("");
    printf("-> A thread vencedora é a: %d com %d primos encontrados.\n", ganhadora, max_primos);


    // Comparando o número total de primos encontrados com o número lido do arquivo
    puts("");
    printf("Total de números primos encontrados: %d\n", total_primos);
    if (total_primos == total_primos_arquivo) {
        printf("O número total de primos encontrados corresponde ao total de números primos no arquivo.\n");
    } else {
        printf("O número total de primos encontrados NÃO corresponde ao total de números primos no arquivo.\n");
    }

    free(buffer);
    free(cont_primo);
    sem_destroy(&mutexGeral);
    sem_destroy(&slotCheio);
    sem_destroy(&slotVazio);

    return 0;
}