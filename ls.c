#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>

#define ERROR -1
#define TAMANIOSTRING 512

// Función de comparación para qsort
int
comparar_nombres(const void *a, const void *b)
{
	return strcmp(*(const char **) a, *(const char **) b);
}

// Función para obtener nombres de archivos/directorios en un directorio
// y ordenarlos alfabéticamente
char **
obtener_nombres_ordenados(char *directorio, size_t *tamanio)
{
	DIR *dir;
	struct dirent *entrada;

	// Para el ordenamiento alfabetico
	char **nombres = NULL;

	dir = opendir(directorio);
	if (!dir) {
		perror("Error al abrir el directorio");
		exit(ERROR);
	}

	while ((entrada = readdir(dir))) {
		nombres = realloc(nombres, (*tamanio + 1) * sizeof(char *));
		nombres[*tamanio] = strdup(entrada->d_name);
		(*tamanio)++;
	}

	closedir(dir);

	// Ordeno los nombres alfabeticament
	qsort(nombres, *tamanio, sizeof(char *), comparar_nombres);

	return nombres;
}

// Lista los contenidos del directorio con la informacion
// Recibe el path del directorio, y un contador para conocer la cantidad de
// archivos Soporta paths absolutos

void
listar_contenidos(char *directorio, int *contador)
{
	size_t tamanio = 0;
	char **nombres = obtener_nombres_ordenados(directorio, &tamanio);
	char ruta_completa[TAMANIOSTRING];

	// Imprimir los resultados ordenados
	for (size_t i = 0; i < tamanio; i++) {
		snprintf(ruta_completa,
		         sizeof(ruta_completa),
		         "%s/%s",
		         directorio,
		         nombres[i]);
		struct stat info;
		if (stat(ruta_completa, &info) == -1) {
			perror("Error al obtener información del "
			       "archivo/directorio");
			exit(ERROR);
		}
		printf("%-30s\t\t", nombres[i]);

		// Imprimo el tipo (directorio "D", enlace simbólico "L" (link), archivo "-")
		struct stat infolink;  // POR SI ES LINK
		if (lstat(ruta_completa, &infolink) == 0) {
			if (S_ISLNK(infolink.st_mode)) {
				printf("L ");
				char buf[TAMANIOSTRING];
				ssize_t len = readlink(ruta_completa,
				                       buf,
				                       sizeof(buf) - 1);
				if (len != -1) {
					buf[len] = '\0';
					printf("-> %s\t", buf);
				}
			}
		}
		if (!S_ISLNK(infolink.st_mode)) {
			if (S_ISDIR(info.st_mode)) {
				printf("D\t\t");
			} else {
				printf("-\t\t");
			}
		}

		// Imprimo los permisos en formato numérico *Ver nota al pie*
		printf("%o\t\t",
		       (unsigned int) info.st_mode &
		               (S_IRWXU | S_IRWXG | S_IRWXO));

		// Imprimo uid del usuario
		struct passwd *pw = getpwuid(info.st_uid);
		if (pw != NULL) {
			printf("%s\n", pw->pw_name);
		} else {
			perror("Error al obtener el nombre del usuario");
			exit(ERROR);
		}
	}

	// Libero la memoria
	for (size_t i = 0; i < tamanio; i++) {
		free(nombres[i]);
	}
	free(nombres);

	*contador = tamanio;
}

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Uso: %s <directorio>\n", argv[0]);
		exit(ERROR);
	}

	printf("%-30s%1s%6s%s\n", "Nombre\t\t", "Tipo\t\t", "Permisos\t", "Usuario");
	int contador = 0;
	listar_contenidos(argv[1], &contador);
	printf("Total: %d\n", contador);

	return 0;
}


/*
La representación numérica utiliza la base octal y asigna valores específicos a cada combinación de permisos.

Read (r) = 4: Si se tiene permiso de lectura, se asigna el valor 4.
Write (w) = 2: Si se tiene permiso de escritura, se asigna el valor 2.
Execute (x) = 1: Si se tiene permiso de ejecución, se asigna el valor 1.
Ahora, para representar todas las combinaciones posibles de permisos, sumamos estos valores. Por ejemplo:

Permiso de lectura (r) = 4

Permiso de escritura (w) = 2

Permiso de ejecución (x) = 1

Permiso de lectura y escritura (rw) = 4 + 2 = 6

Permiso de lectura y ejecución (rx) = 4 + 1 = 5

Permiso de escritura y ejecución (wx) = 2 + 1 = 3

Permiso de lectura, escritura y ejecución (rwx) = 4 + 2 + 1 = 7


Como por ejemplo:

755: Propietario tiene permisos de lectura, escritura y ejecución (7), el grupo
y otros tienen permisos de lectura y ejecución (5). 644: Propietario tiene permisos
de lectura y escritura (6), el grupo y otros solo tienen permiso de lectura (4).

*/