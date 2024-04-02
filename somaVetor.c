#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX 100 //Número máximo de threads permitidas

//Struct para armazenar os argumentos da função executada pelas threads
typedef struct {
    int id; //ID da thread
    float *vetor; //Ponteiro para o vetor de floats
    long int elementos_por_thread; //Tamanho do bloco de elementos que a thread irá processar
    float *soma_parcial; //Ponteiro para a soma parcial dos elementos processados pela thread
} ThreadArgs;

//Função que soma os elementos do vetor
void *somaElementos(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    long int inicio = args->id * args->elementos_por_thread;
    long int fim = inicio + args->elementos_por_thread;

    //Soma parcial dos elementos
    float soma = 0.0;

    //Looping que soma os números do bloco
    for (long int i = inicio; i < fim; i++) {
        soma += args->vetor[i];
    }

    //Atualiza a soma parcial
    *(args->soma_parcial) = soma;
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    //Validação da entrada
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <arquivo de entrada> <numero de threads>\n", argv[0]);
        return 1;
    }

    //Abre o arquivo de entrada
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo de entrada.\n");
        return 2;
    }

    //Lê o número de elementos do vetor
    long int num_elementos;
    fscanf(file, "%ld", &num_elementos);

    //Aloca dinamicamente memória para armazenar um vetor
    float *vetor = (float *)malloc(num_elementos * sizeof(float));

    //Verifica se a alocação de memória deu certo
    if (vetor == NULL) {
        fprintf(stderr, "Erro de alocação de memória.\n");
        fclose(file);
        return 3;
    }
    //Lê os elementos do vetor a partir do arquivo
    for (long int i = 0; i < num_elementos; i++) {
        fscanf(file, "%f", &vetor[i]);
    }

    //Fecha o arquivo
    fclose(file);

    //Número de threads informado pelo usuário
    int NTHREADS = atoi(argv[2]);
    if (NTHREADS < 1 || NTHREADS > MAX) {
        fprintf(stderr, "O número de threads deve ser um valor entre 1 e %d.\n", MAX);
        free(vetor);
        return 4;
    }

    //Calcula o tamanho do bloco de elementos que cada thread irá processar
    long int elementos_por_thread = (num_elementos + NTHREADS - 1) / NTHREADS;

    //Aloca espaço para armazenar os IDs e threads
    pthread_t threads[NTHREADS];
    ThreadArgs args[NTHREADS];
    float soma_parcial[NTHREADS];

    //Cria as threads
    for (int i = 0; i < NTHREADS; i++) {
        args[i].id = i;
        args[i].vetor = vetor;
        args[i].elementos_por_thread = elementos_por_thread;
        args[i].soma_parcial = &soma_parcial[i];
        pthread_create(&threads[i], NULL, somaElementos, (void *)&args[i]);
    }

    //Aguarda o término das threads, acumula as somas parciais e imprime o valor total
    float somaTotal = 0.0;
    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(threads[i], NULL);
        somaTotal += soma_parcial[i];
    }
    printf("A soma total é de: %f\n", somaTotal);

    //Verifica se a soma total está correta e imprime o valor
    float somaEsperada = 0.0;
    for (long int i = 0; i < num_elementos; i++) {
        somaEsperada += vetor[i];
    }
    printf("A soma esperada é de: %f\n", somaEsperada);

    //Libera a memória
    free(vetor);

    return 0;
}