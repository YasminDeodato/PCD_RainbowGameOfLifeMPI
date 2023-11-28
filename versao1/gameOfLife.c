#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

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

    int N=10, nRodadas=3;

    //divisão de domínio
    int nRowsLocal = N/size;
    if (rank == size - 1) {
        nRowsLocal += N % size;
    }

    // linha de bordo "fantasma"
    int nRowsLocalWithGhost = nRowsLocal + 2;
    // int nColwsLocalWithGhost = N + 2;

    // criar matriz local
    float **matrizAtual = (float** )malloc(sizeof(float*) * nRowsLocalWithGhost);
    for (int i = 0; i < n; i++){
        matrizAtual[i] = (float*)malloc(sizeof(float) * nColwsLocalWithGhost);
    }

    float **matrizProxima = (float** )malloc(sizeof(float*) * nRowsLocalWithGhost);
    for (int i = 0; i < n; i++){
        matrizAtual[i] = (float*)malloc(sizeof(float) * nColwsLocalWithGhost);
    }

    //popular a matriz com dados iniciais
    for(int row=1; row < nRowsLocalWithGhost -1; row++) {
        for(int col=0; col < N; col++) {
            matrizAtual[row][col] = rand() % 2;
        }
    }

    // vizinho superior
    int vizinhoSuperior = (rank == 0) ? size - 1 : rank--;

    // vizinho inferior
    int vizinhoInferior = (rank == size - 1) ? 0 : rank++;


    for (int rodada = 0; rodada < nRodadas; rodada++) {
        //-------------- FRONTEIRAS ------------------------
        // envia primeira linha para vizinho superior
        MPI_Send(&matrizAtual[1][0], N, MPI_FLOAT, vizinhoSuperior, 0, MPI_COMM_WORLD);

        // envia última linha para vizinho inferior
        MPI_Send(&matrizAtual[nRowsLocal][0], N, MPI_FLOAT, vizinhoInferior, 0, MPI_COMM_WORLD);


        //recebe informações das bordas vizinhas
        MPI_Recv(&matrizAtual[nRowsLocal + 1][0], N, MPI_FLOAT, vizinhoInferior, 0, MPI_COMM_WORLD);

        MPI_Recv(&matrizAtual[0][0], N, MPI_FLOAT, vizinhoSuperior, 0, MPI_COMM_WORLD);
        
        // display current grid on screen
        if (rank != 0) { 
            for (int row = 1; row <= nRowsLocal; row++) {
                MPI_Send(&matrizAtual[row][1], N, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
            }
        } 

        int vivosLocais = 0;
        //-------------- ATUALIZAR GRID ------------------------
        for (int i=1; i < nRowsLocal; i++) {
            for (int j=0; j < N; j++) {
                matrizProxima[i][j] = verificarNovoEstadoCelula(matrizAtual, i, j);

                if (matrizProxima[i][j] > 0.0) vivosLocais++;

            }
        }

        //----------- COPIA MATRIZ -----------
        for (int i=1; i < nRowsLocal; i++) {
            for (int j=0; j < N; j++) {
                matrizAtual[i][j] = matrizProxima[i][j];
            }
        }

        int totalVivosGeracao = 0;
        MPI_Reduce(&vivosLocais, &totalVivosGeracao, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        // Print the total live count for each generation (only done by rank 0)
        if (rank == 0) {
            printf("Generation %d: Total live cells across all processes: %d\n", rodada, totalVivosGeracao);
        }
    }


    MPI_Finalize();
    return 0;
}