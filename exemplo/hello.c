#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

# define N 15
# define NGERACOES 3

void printMatriz(float **matriz) {
    for( int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            printf("%.1f ", matriz[i][j]);
        }
        printf("\n");
    }
    printf("\n\n");
}

void zeraMatriz(float **matriz) {
    for( int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            matriz[i][j] = 0.0;
        }
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

int main(int argc, char** argv) {
    int rank, size, message_Item;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    float **matrizAtual = (float** )malloc(sizeof(float*) * (N+2));
    for (int i = 0; i < N; i++){
        matrizAtual[i] = (float*)malloc(sizeof(float) * N);
    }

    float **matrizProxima = (float** )malloc(sizeof(float*) * (N+2));
    for (int i = 0; i < N; i++){
        matrizProxima[i] = (float*)malloc(sizeof(float) * N);
    }

    zeraMatriz(matrizAtual);
    zeraMatriz(matrizProxima);

    if (rank == 0) {
        glider(matrizAtual, 1, 1);
        rPentomino(matrizAtual, 5, 10);
    }

    // vizinho superior
    int vizinhoSuperior = (rank == 0) ? size - 1 : rank--;

    // vizinho inferior
    int vizinhoInferior = (rank == size - 1) ? 0 : rank++;


    for (int rodada = 0; rodada < NGERACOES; rodada++) {
        //-------------- FRONTEIRAS ------------------------
        // envia primeira linha para vizinho superior
        MPI_Send(&(matrizAtual[1][0]), N, MPI_FLOAT, vizinhoSuperior, 0, MPI_COMM_WORLD);

        // envia última linha para vizinho inferior
        MPI_Send(&(matrizAtual[N][0]), N, MPI_FLOAT, vizinhoInferior, 0, MPI_COMM_WORLD);


        //recebe informações das bordas vizinhas
        MPI_Recv(&(matrizAtual[N + 1][0]), N, MPI_FLOAT, vizinhoInferior, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        MPI_Recv(&(matrizAtual[0][0]), N, MPI_FLOAT, vizinhoSuperior, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        printf("RODADA %d PROCESSO %d\n", rodada, rank);
        printMatriz(matrizAtual);
    }

    /*if(rank == 0){
        message_Item = 42;
        MPI_Send(&message_Item, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
        printf("Message Sent: %d\n", message_Item);
        printMatriz(matrizAtual);
    }

    else if(rank == 1){
        MPI_Recv(&message_Item, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Message Received: %d\n", message_Item);
        printMatriz(matrizAtual);
    }*/

    MPI_Finalize();
    return 0;
}