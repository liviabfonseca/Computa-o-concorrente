/*
Programa auxiliar para gerar um vetor de floats 
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 1000 //valor maximo dos elementos do vetor

int main(int argc, char *argv[]) {
    long int n; //qtde de elementos do vetor que serao gerados
    float elem; //valor gerado para incluir no vetor
    int fator=1; //fator multiplicador para gerar números negativos
    float maior, menor; //maior e menor elemento da sequencia que sera gerada
    
    //valida e recebe os valores de entrada
    if(argc < 2) {
       printf("Use: %s <numero de elementos (maior que 0)> \n", argv[0]);
       return 1;
    }
    n = atol(argv[1]);
    if(n<1) {
       printf("ERRO: o numero de elementos deve ser maior que 0 \n");
       return 2;
    }	    

    //semente para gerar numeros randomicos
    srand((unsigned)time(NULL));

    //imprime a qtde de elementos do vetor (inteiro)
    printf("%ld\n", n);
    
    //gera o primeiro elemento e inicializa maior e menor
    elem = (rand() % MAX)/3.0 * fator;
    maior = menor = elem;
    printf("%f ", elem);
    //gera e imprime os elementos restantes separado por espaco em branco
    for(int i=0; i<n-1; i++) {
        elem = (rand() % MAX)/3.0 * fator;
        printf("%f ", elem);
        fator*=-1;
	if(elem > maior)
            maior = elem;
        if(elem < menor)
            menor = elem;
    }
    //imprime maior e menor no final
    printf("\n%f %f\n", maior, menor);

    return 0;
}
