#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ARGS_MIN 2
#define ERROR -1

#define LECTURA 0    // file descriptor de lectura
#define ESCRITURA 1  // file descriptor de escritura


const int valor_minimo = 2;
#define TAMANIO_PIPE 2


void filtrar(int vector_pipe[2]);
int crear_listado(int valor_tope, int fds);
void enviar_listado_filtrado(int valor_actual,
                             int pipe_desde[TAMANIO_PIPE],
                             int pipe_hasta[TAMANIO_PIPE]);

int
main(int argc, char *argv[])
{
	if (argc < ARGS_MIN) {
		printf("Cantidad de argumentos invalida \n");
		return ERROR;
	}

	int valor_tope = strtol(argv[1], NULL, 10);
	if (valor_tope < valor_minimo) {
		printf("Para la ejecucion se necesita un valor mayor igual a "
		       "%d \n",
		       valor_minimo);
		return ERROR;
	};

	int padre_hijo_fds[TAMANIO_PIPE];
	if (pipe(padre_hijo_fds) < 0) {
		printf("Error al crear pipe\n");
		return ERROR;
	}

	pid_t id_fork = fork();
	if (id_fork == ERROR) {
		printf("Error al realizar el fork");
		return ERROR;
	}

	if (id_fork > 0) {  // Estamos en el padre
		close(padre_hijo_fds[LECTURA]);
		int resultado_listado =
		        crear_listado(valor_tope, padre_hijo_fds[ESCRITURA]);
		if (resultado_listado == ERROR) {
			printf("Error al intentar crear y enviar el listado de "
			       "numeros");
			return ERROR;
		}
		close(padre_hijo_fds[ESCRITURA]);  // Tras enviar el listado de numeros cierro la escritura
		pid_t resultado_wait = wait(NULL);
		if (resultado_wait == ERROR) {
			printf("Error al ejecutar el wait");
			return (ERROR);
		}
	} else {  // Soy el hijo
		filtrar(padre_hijo_fds);
	}

	return 0;
}


int
crear_listado(int valor_tope, int fds)
{
	for (int i = 2; i <= valor_tope; i++) {
		int escritura = write(fds, &i, sizeof(int));
		if (escritura == ERROR) {
			return ERROR;
		}
	}
	return 0;
}

void
filtrar(int vector_pipe[2])
{
	close(vector_pipe[ESCRITURA]);
	int primo;
	int resultado_lectura = read(vector_pipe[LECTURA], &primo, sizeof(int));
	if (resultado_lectura == ERROR) {
		printf("Error al realizar la lectura");
		return;
	}
	if (resultado_lectura == 0) {
		close(vector_pipe[LECTURA]);
		exit(0);
	}

	int hijo_nieto_fds[TAMANIO_PIPE];
	if (pipe(hijo_nieto_fds) < 0) {
		printf("Error al crear pipe\n");
		return;
	}

	printf("primo %d\n", primo);

	pid_t id_fork = fork();
	if (id_fork == ERROR) {
		printf("Error al realizar el fork");
		return;
	}

	if (id_fork > 0) {  // Estamos en el hijo
		close(hijo_nieto_fds[LECTURA]);
		enviar_listado_filtrado(primo, vector_pipe, hijo_nieto_fds);
		close(vector_pipe[LECTURA]);
		close(hijo_nieto_fds[ESCRITURA]);
		pid_t resultado_wait = wait(NULL);
		if (resultado_wait == ERROR) {
			printf("Error al ejecutar el wait");
			return;
		}
		exit(0);
	} else {
		close(vector_pipe[LECTURA]);
		close(hijo_nieto_fds[ESCRITURA]);
		filtrar(hijo_nieto_fds);
	}
}

void
enviar_listado_filtrado(int valor_actual,
                        int pipe_desde[TAMANIO_PIPE],
                        int pipe_hasta[TAMANIO_PIPE])
{
	int primo;
	int resultado_lectura;
	int resultado_escritura;

	resultado_lectura = read(pipe_desde[LECTURA], &primo, sizeof(int));
	if (resultado_lectura == ERROR) {
		printf("Error al realizar la lectura");
		return;
	}
	while (resultado_lectura > 0) {
		if ((primo % valor_actual) != 0) {
			resultado_escritura =
			        write(pipe_hasta[ESCRITURA], &primo, sizeof(int));
			if (resultado_escritura == ERROR) {
				printf("Error al realizar la escritura");
				return;
			}
		}
		resultado_lectura =
		        read(pipe_desde[LECTURA], &primo, sizeof(int));
	}
	if (resultado_lectura == ERROR) {
		printf("Error al realizar la lectura");
		return;
	}
}