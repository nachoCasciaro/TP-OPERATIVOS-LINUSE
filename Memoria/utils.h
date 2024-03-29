
#ifndef UTILS_H_
#define UTILS_H_
#define EXIT_FAILURE 1

#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include "muse.h"

int iniciar_conexion(char*, int);
void levantarConfigFile();
void arrancarMemoria();
t_config* leer_config(void);
t_log * crear_log();
void loguearInfo(char* texto);
void atenderMuseInit(int socket);
void atenderMuseClose(int socket);
void atenderMuseAlloc(int socket);
void atenderMuseFree(int socket);
void atenderMuseGet(int socket);
void atenderMuseCopy(int socket);
void atenderMuseMap(int socket);
void atenderMuseSync(int socket);
void atenderMuseUnmap(int socket);

typedef struct {
	char *ip;
	int puerto;
	int tamanio_memoria;
	int tamanio_pag;
	int tamanio_swap;
} config;

t_log* logger;
pthread_t hiloLevantarConexion;
config* pconfig;

int client_socket[30];

#endif /* UTILS_H_ */
