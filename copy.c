#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define ERROR -1
#define TAMANIOBUFFER 4096


void
copy(char *destino, char *origen)
{
	// Archivo de origen
	int fd_origen = open(destino, O_RDONLY);
	if (fd_origen == -1) {
		perror("Error al abrir el archivo de origen");
		exit(ERROR);
	}
	// Archivo de destino
	int fd_destino =
	        open(origen, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd_destino == -1) {
		perror("Error al crear el archivo de destino");
		close(fd_origen);
		exit(ERROR);
	}

	// Info sobre el archvio de origen
	struct stat info_origen;
	if (fstat(fd_origen, &info_origen) == -1) {
		perror("Error al obtener información del archivo de origen");
		close(fd_origen);
		close(fd_destino);
		exit(ERROR);
	}
	// Ajusto el tamaño del archivo de destino para que sea el tamaño del archivo de origen
	if (ftruncate(fd_destino, info_origen.st_size) == -1) {
		perror("Error al establecer el tamaño del archivo de destino");
		close(fd_origen);
		close(fd_destino);
		exit(ERROR);
	}

	char *map_origen = mmap(
	        NULL, info_origen.st_size, PROT_READ, MAP_SHARED, fd_origen, 0);
	char *map_destino = mmap(
	        NULL, info_origen.st_size, PROT_WRITE, MAP_SHARED, fd_destino, 0);

	if (map_origen == MAP_FAILED || map_destino == MAP_FAILED) {
		perror("Error al mapear archivos a memoria");
		close(fd_origen);
		close(fd_destino);
		exit(ERROR);
	}

	// Copio los datos de la región de memoria de origen a la de destino
	memcpy(map_destino, map_origen, info_origen.st_size);

	// Desmapeo las regiones de memoria
	if (munmap(map_origen, info_origen.st_size) == -1 ||
	    munmap(map_destino, info_origen.st_size) == -1) {
		perror("Error al desmapear archivos de memoria");
		close(fd_origen);
		close(fd_destino);
		exit(ERROR);
	}

	// Si todo sale correctamente, cierra los archivos y retorna
	close(fd_origen);
	close(fd_destino);
}

/*
 *Funciona con paths relativos pero tambien con absolutos, aunque puede haber un
 *error de permisos dependiendo donde se quiera copiar Yo lo solucione o bien,
 *cambiando el path a donde se quiere copiar, o bien haciendo sudo ./copy ...
 */
int
main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "ERROR \nForma de uso: %s <ruta_origen> <ruta_destino>\n", argv[0]);
		exit(ERROR);
	}

	copy(argv[1], argv[2]);

	return 0;
}
