#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

float *mat1; // primeira matriz de entrada
float *mat2; // segunda matriz de entrada
float *saida_concorrente; // matriz de saída para a versão concorrente
float *saida_sequencial; // matriz de saída para a versão sequencial
int nthreads; // número de threads
int linhas_mat1, colunas_mat1, colunas_mat2; // dimensões das matrizes

typedef struct {
    int id; // identificador do elemento que a thread irá processar
    int linhas_mat1, colunas_mat1, colunas_mat2; // dimensões das matrizes
} tArgs;

// função que as threads executarão
void *tarefa(void *arg) {
    tArgs *args = (tArgs *)arg;

    int linhas_por_thread = args->linhas_mat1 / nthreads;
    int inicio = args->id * linhas_por_thread;
    int fim = (args->id == nthreads - 1) ? args->linhas_mat1 : inicio + linhas_por_thread;

    for (int i = inicio; i < fim; i++) {
        for (int j = 0; j < args->colunas_mat2; j++) {
            for (int k = 0; k < args->colunas_mat1; k++) {
                saida_concorrente[i * args->colunas_mat2 + j] += mat1[i * args->colunas_mat1 + k] * mat2[k * args->colunas_mat2 + j];
            }
        }
    }
    pthread_exit(NULL);
}

// função para multiplicação sequencial de matrizes
void multiplicacao_sequencial() {
    for (int i = 0; i < linhas_mat1; i++) {
        for (int j = 0; j < colunas_mat2; j++) {
            float soma_produto = 0;
            for (int k = 0; k < colunas_mat1; k++) {
                soma_produto += mat1[i * colunas_mat1 + k] * mat2[k * colunas_mat2 + j];
            }
            saida_sequencial[i * colunas_mat2 + j] = soma_produto;
        }
    }
}

int main(int argc, char *argv[]) {
    pthread_t *tid; // identificador das threads no sistema
    tArgs *args; // identificadores locais e dimensões
    double inicio, fim, tempo_sequencial_inicio, tempo_concorrente, tempo_sequencial_saida, tempo_sequencial, parte_paralela, aceleracao,
    eficiencia; // tomada de tempo

    // Inicio do tempo de inicialização
    GET_TIME(inicio); 

    if (argc < 5) {
        printf("Digite: %s <Arquivo matriz1> <Arquivo matriz2> <Arquivo de saída concorrente> <Arquivo de saída sequencial> <Numero de threads>\n", argv[0]);
        return 1;
    }

    char *arquivo_mat1 = argv[1];
    char *arquivo_mat2 = argv[2];
    char *arquivo_saida_concorrente = argv[3];
    char *arquivo_saida_sequencial = argv[4];
    nthreads = atoi(argv[5]);

    // Abre os arquivos de entrada
    FILE *arq_mat1 = fopen(arquivo_mat1, "rb");
    FILE *arq_mat2 = fopen(arquivo_mat2, "rb");

    if (arq_mat1 == NULL || arq_mat2 == NULL) {
        printf("Erro ao abrir os arquivos de entrada.\n");
        return 2;
    }

    // Lê as dimensões das matrizes de entrada
    fread(&linhas_mat1, sizeof(int), 1, arq_mat1);
    fread(&colunas_mat1, sizeof(int), 1, arq_mat1);
    fread(&colunas_mat2, sizeof(int), 1, arq_mat2);

    // Aloca memória para as matrizes
    mat1 = (float *)malloc(sizeof(float) * linhas_mat1 * colunas_mat1);
    mat2 = (float *)malloc(sizeof(float) * colunas_mat1 * colunas_mat2);
    saida_concorrente = (float *)calloc(linhas_mat1 * colunas_mat2, sizeof(float));
    saida_sequencial = (float *)calloc(linhas_mat1 * colunas_mat2, sizeof(float));

    if (mat1 == NULL || mat2 == NULL || saida_concorrente == NULL || saida_sequencial == NULL) {
        printf("Erro ao alocar memória para as matrizes.\n");
        return 3;
    }

    // Lê as matrizes de entrada dos arquivos
    fread(mat1, sizeof(float), linhas_mat1 * colunas_mat1, arq_mat1);
    fread(mat2, sizeof(float), colunas_mat1 * colunas_mat2, arq_mat2);

    // Fecha os arquivos de entrada
    fclose(arq_mat1);
    fclose(arq_mat2);

    // Marcador do final da inicialização
    GET_TIME(fim);

    // Calculando o tempo de inicialização
    tempo_sequencial_inicio = fim - inicio;
    printf("O tempo de inicialização do programa foi de %lf\n", tempo_sequencial_inicio);

    // Inicio do tempo de execução do programa sequencial
    GET_TIME(inicio);

    // Multiplicação sequencial das matrizes
    multiplicacao_sequencial();

    // Marcador do final do programa sequencial
    GET_TIME(fim);

    // Calculando o tempo de processamento sequencial
    tempo_sequencial = fim - inicio;
    printf("O tempo de processamento sequencial é de %lf\n", tempo_sequencial);

    // Parte concorrente
    GET_TIME(inicio);

    // alocação das estruturas
    tid = (pthread_t *)malloc(sizeof(pthread_t) * nthreads);
    if (tid == NULL) {
        puts("ERRO--malloc");
        return 4;
    }
    args = (tArgs *)malloc(sizeof(tArgs) * nthreads);
    if (args == NULL) {
        puts("ERRO--malloc");
        return 4;
    }

    // criação das threads
    for (int i = 0; i < nthreads; i++) {
        (args + i)->id = i;
        (args + i)->linhas_mat1 = linhas_mat1;
        (args + i)->colunas_mat1 = colunas_mat1;
        (args + i)->colunas_mat2 = colunas_mat2;
        if (pthread_create(tid + i, NULL, tarefa, (void *)(args + i))) {
            puts("ERRO--pthread_create");
            return 5;
        }
    }

    // espera pelo término das threads
    for (int i = 0; i < nthreads; i++) {
        pthread_join(*(tid + i), NULL);
    }

    // Marcador do final do programa concorrente
    GET_TIME(fim);

    // Calculando o tempo de processamento concorrente
    tempo_concorrente = fim - inicio;
    printf("O tempo de processamento concorrente é de %lf\n", tempo_concorrente);

    // Inicio do tempo de finalização 
    GET_TIME(inicio);

    // Abre os arquivos de saída
    FILE *arq_saida_concorrente = fopen(arquivo_saida_concorrente, "wb");
    FILE *arq_saida_sequencial = fopen(arquivo_saida_sequencial, "wb");

    if (arq_saida_concorrente == NULL || arq_saida_sequencial == NULL) {
        printf("Erro ao abrir os arquivos de saída.\n");
        return 6;
    }

    // Escreve as dimensões da matriz de saída nos arquivos
    fwrite(&linhas_mat1, sizeof(int), 1, arq_saida_concorrente);
    fwrite(&colunas_mat2, sizeof(int), 1, arq_saida_concorrente);
    fwrite(&linhas_mat1, sizeof(int), 1, arq_saida_sequencial);
    fwrite(&colunas_mat2, sizeof(int), 1, arq_saida_sequencial);

    // Escreve a matriz de saída para a versão concorrente no arquivo
    fwrite(saida_concorrente, sizeof(float), linhas_mat1 * colunas_mat2, arq_saida_concorrente);

    // Escreve a matriz de saída para a versão sequencial no arquivo
    fwrite(saida_sequencial, sizeof(float), linhas_mat1 * colunas_mat2, arq_saida_sequencial);

    // Fecha os arquivos de saída
    fclose(arq_saida_concorrente);
    fclose(arq_saida_sequencial);

    // Liberação da memória
    free(mat1);
    free(mat2);
    free(saida_concorrente);
    free(saida_sequencial);
    free(args);
    free(tid);

    // Marcador do final da finalização
    GET_TIME(fim);

    // Calculando o tempo de processamento concorrente
    tempo_sequencial_saida = fim - inicio;
    printf("O tempo de finalização é de %lf\n", tempo_sequencial_saida);

    // Calculo da aceleração
    parte_paralela = tempo_concorrente/nthreads;

    aceleracao = (tempo_sequencial_inicio + tempo_sequencial + tempo_sequencial_saida)/(parte_paralela + tempo_sequencial_inicio + tempo_sequencial_saida);
    printf("O ganho estimado foi de %lf\n", aceleracao);

    // Calculo da eficiencia
    eficiencia = aceleracao/tempo_concorrente;
    printf("A eficiência foi de %lf\n", eficiencia);

    return 0;
}
