#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


#ifndef NARGS
#define NARGS 4
#endif

#define ARGS_MIN 2
#define ERROR -1
#define RESULTADOOK 0

#define final_nulo 1
#define espacio_comando 1


int leer_argumentos(char **argumentos, int tamanio_paquete);
int ejecutar_argumentos(char **argumentos, int cantidad_argumentos);
void liberar_memoria(char **argumentos, int cantidad_argumentos);
void liberar_posicion_memoria(char **argumentos, int posicion);


int
main(int argc, char *argv[])
{
	if (argc < ARGS_MIN) {
		printf("Cantidad de argumentos invalida");
		return ERROR;
	}

	char *argumentos[espacio_comando + NARGS + final_nulo] = { NULL };
	argumentos[0] = argv[1];  // Convencion de execvp

	return leer_argumentos(argumentos, NARGS);
}

/*
 *Funcion encargada de leer los argumentos recibidos por linea de comandos
 *Recibe el vector donde se guardan los argumentos y la cantidad de argumentos
 *que se van a empaquetar en cada llamado a la ejecucion
 * Devuelve -1 en caso de algun error. 0 Si la lectura y ejecucion es correcta
 */
int
leer_argumentos(char **argumentos, int tamanio_paquete)
{
	size_t tamanio = 0;
	int cantidad_argumentos = 1;
	int resultado_ejecucion = 0;

	if (argumentos[cantidad_argumentos] != NULL) {
		free(argumentos[cantidad_argumentos]);  // Liberar la memoria si ya estaba asignada
		argumentos[cantidad_argumentos] = NULL;
	}
	int linea_leida =
	        getline(&argumentos[cantidad_argumentos], &tamanio, stdin);
	if (linea_leida == ERROR) {
		printf("Error en la lectura de argumentos\n");
		return ERROR;
	}

	while (linea_leida != ERROR && resultado_ejecucion == RESULTADOOK) {
		argumentos[cantidad_argumentos][linea_leida] =
		        '\0';  // Quito el salto de linea si lo hay

		if (cantidad_argumentos == tamanio_paquete) {
			resultado_ejecucion =
			        ejecutar_argumentos(argumentos,
			                            cantidad_argumentos);
			cantidad_argumentos = 0;
		}

		cantidad_argumentos++;

		if (argumentos[cantidad_argumentos] != NULL) {
			free(argumentos[cantidad_argumentos]);  // Liberar la memoria si ya estaba asignada
			argumentos[cantidad_argumentos] = NULL;
		}

		linea_leida =
		        getline(&argumentos[cantidad_argumentos], &tamanio, stdin);
	}

	if (resultado_ejecucion != RESULTADOOK) {
		return ERROR;
	}

	if (linea_leida == ERROR) {
		liberar_posicion_memoria(argumentos, cantidad_argumentos);
	}

	if (cantidad_argumentos != 1) {
		resultado_ejecucion =
		        ejecutar_argumentos(argumentos, cantidad_argumentos);
	}

	return resultado_ejecucion;
}

/*
 *Funcion encargada de ejecutar los argumentos recibidos
 *Recibe el vector donde se guardan los argumentos y la cantidad de argumentos
 *a ejecutar
 * Devuelve -1 en caso de algun error. 0 Si su ejecucion es correcta
 */
int
ejecutar_argumentos(char **argumentos, int cantidad_argumentos)
{
	pid_t id_fork = fork();
	if (id_fork == ERROR) {
		printf("Error al realizar el fork\n");
		return ERROR;
	}
	if (id_fork > 0) {  // Estamos en el caso padre
		pid_t resultado_wait = wait(NULL);
		if (resultado_wait == ERROR) {
			printf("Error al ejecutar el wait\n");
			return (ERROR);
		}
		liberar_memoria(argumentos, cantidad_argumentos);
		return RESULTADOOK;
	} else {
		int ejecucion = execvp(argumentos[0], argumentos);
		if (ejecucion == ERROR) {
			printf("Error al intentar ejecutar el comando\n");
			return ERROR;
		}
		return RESULTADOOK;
	}
}


/*
 *Funcion que libera la memoria asignada a los argumentos que ya fueron ejecutados
 *Ademas "limpia" el array dejando sus valores nulos
 *Recibe el array de caracteres con los argumentos y la cantidad de estos
 */
void
liberar_memoria(char **argumentos, int cantidad_argumentos)
{
	for (int i = 1; i < cantidad_argumentos + 1; i++) {
		free(argumentos[i]);
		argumentos[i] = NULL;
	}
}


/*
 *Funcion que recibe una posicion de memoria para liberarla.
 *Esta funcion surge porque el getline hace un alloc aunque falle
 *Recibe el vector y la posicion a liberar.
 */
void
liberar_posicion_memoria(char **argumentos, int posicion)
{
	free(argumentos[posicion]);
	argumentos[posicion] = NULL;
}