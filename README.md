# 🌈 Rainbow Game Of Life MPI

Repositório para códigos da atividade 3 da disciplina de Programação Concorrente e Distribuída, ministrada no 2sem/2023, no ICT-Unifesp.

### Alunos
Hélio Didzec Júnior \
Yasmin Beatriz Deodato

### Informações

#### 0️⃣ Compilar
```bash
mpicc prog.c -o prog
```
#### 1️⃣ Executar
```bash
mpirun -np 5 prog
mpirun --allow-run-as-root --oversubscribe -np 5 prog

mpicc rainbowGameOfLifeMPI.c -o game
sudo time mpirun -np 4 ./game
```
