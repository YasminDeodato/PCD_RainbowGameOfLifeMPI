/*
* Programacao Concorrente e Distribuida
* Rainbow Game of Life com MPI
* Helio Didzec Junior
* Yasmin Beatriz Deodato
*/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

# define N 2048
# define NGERACOES 200

int contaCelula(float **matriz, int x, int y){
    int contador = 0;

    if (((x > 0) && (x < N-1)) && ((y > 0) && (y < N-1))) {
        for(int i = x-1 ; i <= x+1; i++){
            for(int j = y-1; j <= y+1; j++){
                if (matriz[i][j] > 0.0) contador += 1;
            }
        }
    } else {
        for(int i = x-1 ; i <= x+1; i++){
            for(int j = y-1; j <= y+1; j++){
                if ((i < 0) && (j < 0)) {
                    if (matriz[N-1][N-1] > 0.0) contador += 1;
                } else if((i < 0) && (j > N-1)){
                    if (matriz[N-1][0] > 0.0) contador += 1;
                } else if ((i > N-1) && (j > N-1)) {
                    if (matriz[0][0] > 0.0) contador += 1;
                } else if((i > N-1) && (j < 0)){
                    if (matriz[0][N-1] > 0.0) contador += 1;
                } else if (i < 0){
                    if (matriz[N-1][j] > 0.0) contador += 1;
                } else if(j < 0){
                    if (matriz[i][N-1] > 0.0) contador += 1;
                } else if(i > N-1){
                    if (matriz[0][j] > 0.0) contador += 1;
                } else if(j > N-1){
                    if (matriz[i][0] > 0.0) contador += 1;
                } else {
                    if (matriz[i][j] > 0.0) contador += 1;
                }
            }
        }
    }
    if (matriz[x][y] > 0.0) contador -= 1;


    return contador;
}

float mediaCelula(float **matriz, int x, int y){
    float contador = 0.0;

    if (((x > 0) && (x < N-1)) && ((y > 0) && (y < N-1))) {
        for(int i = x-1 ; i <= x+1; i++){
            for(int j = y-1; j <= y+1; j++){
                contador += matriz[i][j];
            }
        }
    } else {
        for(int i = x-1 ; i <= x+1; i++){
            for(int j = y-1; j <= y+1; j++){
                if ((i < 0) && (j < 0)) {
                    contador += matriz[N-1][N-1];
                } else if((i < 0) && (j > N-1)){
                    contador += matriz[N-1][0];
                } else if ((i > N-1) && (j > N-1)) {
                    contador += matriz[0][0];
                } else if((i > N-1) && (j < 0)){
                    contador += matriz[0][N-1];
                } else if (i < 0){
                    contador += matriz[N-1][j];
                } else if(j < 0){
                    contador += matriz[i][N-1];
                } else if(i > N-1){
                    contador += matriz[0][j];
                } else if(j > N-1){
                    contador += matriz[i][0];
                } else {
                    contador += matriz[i][j];
                }
            }
        }
    }
    contador -= matriz[x][y];


    return (contador/8.0);
}

float verificarNovoEstadoCelula(float **matriz, int i, int j) {
    int vivos = contaCelula(matriz, i, j);

    //qualquer celula morta com 3 (tres) vizinhos torna-se viva;
    if ((matriz[i][j] == 0.0) && (vivos == 3)) {
        return mediaCelula(matriz, i, j);
    //qualquer celula viva com 2 (dois) ou 3 (tres) vizinhos deve sobreviver;
    } else if ((matriz[i][j] > 0.0) && ((vivos == 2) || (vivos == 3))) {
        return 1.0;
    }
    //qualquer outro caso, celulas vivas devem morrer e celulas ja mortas devem continuar mortas.
    else {
       return 0.0;
    }
}

void glider(float **matriz, int x, int y) {
    matriz[x  ][y+1] = 1.0;
    matriz[x+1][y+2] = 1.0;
    matriz[x+2][y] = 1.0;
    matriz[x+2][y+1] = 1.0;
    matriz[x+2][y+2] = 1.0;
}

void rPentomino(float **matriz, int x, int y) {
    matriz[x+1][y  ] = 1.0;
    matriz[x  ][y+1] = 1.0;
    matriz[x+1][y+1] = 1.0;
    matriz[x+2][y+1] = 1.0;
    matriz[x  ][y+2] = 1.0;
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);

    //divisao de dominio
    int nLinhasLocal = N/size;
    if (rank == size - 1) {
        nLinhasLocal += N % size;
    }

    // linha de bordo "fantasma"
    int nLinhasLocalComFronteira = nLinhasLocal + 2;

    // alocar matrizes
    float **matrizAtual = (float** )malloc(sizeof(float*) * nLinhasLocalComFronteira);
    for (int i = 0; i < nLinhasLocalComFronteira; i++){
        matrizAtual[i] = (float*)malloc(sizeof(float) * N);
    }

    float **matrizProxima = (float** )malloc(sizeof(float*) * nLinhasLocalComFronteira);
    for (int i = 0; i < nLinhasLocalComFronteira; i++){
        matrizProxima[i] = (float*)malloc(sizeof(float) * N);
    }

    //popular a matriz com dados iniciais
    for(int row=1; row < nLinhasLocalComFronteira; row++) {
        for(int col=0; col < N; col++) {
            matrizAtual[row][col] = 0;
        }
    }
    // o processo 0 eh responsavel pela parte da tabela com o glider e rPentomino
    if (rank == 0) {
        glider(matrizAtual, 1, 1);
        rPentomino(matrizAtual, 10, 30);
    }

    // vizinho superior
    int vizinhoSuperior = (rank == 0) ? size - 1 : rank-1;

    // vizinho inferior
    int vizinhoInferior = (rank == size - 1) ? 0 : rank+1;


    for (int geracao = 0; geracao < NGERACOES; geracao++) {
        // envia primeira linha para vizinho superior
        MPI_Send(&matrizAtual[1][0], N, MPI_FLOAT, vizinhoSuperior, 0, MPI_COMM_WORLD);
        // envia última linha para o processo anterior
        MPI_Send(&matrizAtual[nLinhasLocal][0], N, MPI_FLOAT, vizinhoInferior, 1, MPI_COMM_WORLD);

        // recebe a primeira linha da matriz a partir da fronteira do vizinho superior
        MPI_Recv(&matrizAtual[0][0], N, MPI_FLOAT, vizinhoSuperior, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // recebe a ultima linha da matriz a partir da fronteira do vizinho inferior
        MPI_Recv(&matrizAtual[nLinhasLocal+1][0], N, MPI_FLOAT, vizinhoInferior, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


        // atualiza grid e conta numero de vivos locais
        int vivosLocais = 0;
        for (int i=1; i < nLinhasLocal+1; i++) {
            for (int j=0; j < N; j++) {
                matrizProxima[i][j] = verificarNovoEstadoCelula(matrizAtual, i, j);

                if (matrizProxima[i][j] > 0.0) vivosLocais++;

            }
        }

        // copia matriz anterior para atual
        for (int i=1; i < nLinhasLocal+1; i++) {
            for (int j=0; j < N; j++) {
                matrizAtual[i][j] = matrizProxima[i][j];
            }
        }

        // soma o numero de vivos locais de todos os processos
        int totalVivosGeracao = 0;
        MPI_Reduce(&vivosLocais, &totalVivosGeracao, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            printf("Geracao n %d - Celulas vivas: %d\n", geracao+1, totalVivosGeracao);
        }
    }

    free(matrizAtual);
    free(matrizProxima);

    MPI_Finalize();
    return 0;
}