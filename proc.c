#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

#define ERROR -1
#define RESULTADOOK 0
#define TAMANIOSTRING 512

void
ps()
{
	DIR *dir;
	struct dirent *entrada;

	// Busco el directorio /proc
	dir = opendir("/proc");
	if (dir == NULL) {
		perror("Error al abrir /proc");
		exit(ERROR);
	}

	char *rutacmdl;
	char cmdline[TAMANIOSTRING];
	char proc_path[TAMANIOSTRING];

	while ((entrada = readdir(dir))) {
		if (isdigit(entrada->d_name[0])) {  // Busco entrada->d_name, que en caso de ser un proceso, sera el pid

			// Armo la ruta al archivo
			snprintf(proc_path,
			         sizeof(proc_path),
			         "/proc/%s",
			         entrada->d_name);  // EN

			// Estaba teniendo un warning extranio, que surgia por
			// poder tener direcciones mas grandes en tamanio, al parecer esta es la solucion
			int len = snprintf(NULL, 0, "%s/comm", proc_path);
			rutacmdl = malloc(len + 1);  // +1 para el carácter nulo

			if (rutacmdl == NULL) {
				perror("Error al asignar memoria");
				closedir(dir);
				exit(ERROR);
			}

			snprintf(rutacmdl, len + 1, "%s/comm", proc_path);

			// Abro el archivo cmdline
			FILE *cmdl_file = fopen(rutacmdl, "r");

			if (!cmdl_file) {
				perror("Error. No existe el archivo cmdline");
				closedir(dir);
				free(rutacmdl);
				exit(ERROR);
			} else {
				if (fgets(cmdline, sizeof(cmdline), cmdl_file)) {
					size_t largo = strcspn(cmdline, "\n");

					// Elimino el salto de línea al final, si existe
					if (largo > 0) {
						cmdline[largo] = '\0';
					}

					// Imprimo PID y comando
					printf("%5s %s \n",
					       entrada->d_name,
					       cmdline);
				}

				fclose(cmdl_file);
			}
			free(rutacmdl);
		}
	}

	// Cierro el dir
	closedir(dir);
}

int
main()
{
	printf("  PID COMMAND\n");
	ps();
	return 0;
}
