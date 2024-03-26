#include <stdio.h>
#include <pthread.h>

#define N 10 //Número de elementos do vetor
#define NTHREADS 4 //Número de threads

int vetor[N]; //Vetor de entrada
int resultado[N];

//Função para inicializar o vetor com números sequenciais de 1 a N
void inicializa_vetor() {
    for (int i = 0; i < N; i++) {
        vetor[i] = i + 1;
    }
}

//Função que eleva o vetor ao quadrado
void *quadrado(void *arg) {
    int thread_id = *((int *)arg);
    int i;
    int inicio;
    int fim;

    //Calculando quantos elementos cada thread deverá processar
    int elementos_por_thread = N/NTHREADS;

    inicio = thread_id * elementos_por_thread;
    fim = inicio + elementos_por_thread;

    //Quando tivermos uma divisão com resto, os elementos restantes da divisão são processados pela última thread
    if (thread_id == NTHREADS - 1)
        fim = N;

    //Looping que eleva os números ao quadrado
    for (i = inicio; i < fim; i++) {
        resultado[i] = vetor[i] * vetor[i];
    }
    pthread_exit(NULL);
}

//Função que verifica se o resultado calculado está correto
int verifica_resultado() {
    for (int i = 0; i < N; i++) {
        if (resultado[i] != (vetor[i] * vetor[i])) {
            return 0; // Retorna 0 se o resultado estiver incorreto
        }
    }
    return 1; //Retorna 1 se o resultado estiver correto
}

int main(void) {
    pthread_t tid[NTHREADS]; // Identificador da thread no sistema
    int ident[NTHREADS]; // Identificador local da thread

    // Inicializando o vetor
    inicializa_vetor();

    // Criando as threads
    for (int i = 0; i < NTHREADS; i++) {
        ident[i] = i;
        if (pthread_create(&tid[i], NULL, quadrado, (void *)&ident[i])) {
            printf("ERRO -- pthread_create\n");
        }
    }

    // Aguardando o término da execução das threads criadas
    for (int i = 0; i < NTHREADS; i++) {
        if (pthread_join(tid[i], NULL)) {
            printf("ERRO -- pthread_create\n");
        }
    }

    // Verificando se os valores finais do vetor estão corretos
    if (verifica_resultado()) {
        printf("O resultado está correto\n");
    } 
    else {
        printf("O resultado está incorreto\n");
    }

    // Imprimindo o resultado
    printf("Vetor selecionado: ");
    for (int i = 0; i < N; i++) {
        printf("%d ", vetor[i]);
    }

    printf("\nVetor ao quadrado: ");
    for (int i = 0; i < N; i++) {
        printf("%d ", resultado[i]);
    }
    printf("\n");

    return 0;
}