/** Cria um arquivo binario com N números inteiros positivos e com a quantidade de 
    primos que o arquivo possui. Além disso, imprime esse valor total no terminal 
    Uso: ./geraValores <nome_do_arquivo> <quantidade_de_numeros>**/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

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

int gera_arquivo(const char *filename, int N) {
    FILE *file = fopen(filename, "wb");  //"wb" - escrita binária
    if (!file) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    // Contador de números primos
    int num_primos = 0;

    for (int i = 0; i < N; ++i) {
        int num = rand() % 1000000 + 1; // Gera números entre 1 e 1000000
        if (ehPrimo(num)) {
            num_primos++;
        }
        fwrite(&num, sizeof(int), 1, file); // Escrita binária
    }

    // Escreve o número de primos no arquivo
    fwrite(&num_primos, sizeof(int), 1, file);

    fclose(file);
    return num_primos;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <nome_do_arquivo> <quantidade_de_numeros>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Nome do arquivo fornecido pelo usuário
    const char *filename = argv[1]; 
    // Quantidade de números fornecido pelo usuário
    int N = atoi(argv[2]); 

    // Caso de erro: quando o número N digitado é igual ou menor que 0
    if (N <= 0) {
        fprintf(stderr, "A quantidade de números deve ser um inteiro positivo.\n");
        exit(EXIT_FAILURE);
    }

    int num_primos = gera_arquivo(filename, N);

    // Imprime a quantidade de números primos que o arquivo possui
    printf("Total de números primos encontrados: %d\n", num_primos);
    
    return 0;
}
