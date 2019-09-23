#include "conexionCli.h"

struct sockaddr_in serverAddress;
struct sockaddr_in serverAddresssacServer;


void conectarseASacServer(){
	sacServer = socket(AF_INET, SOCK_STREAM, 0);
	serverAddresssacServer.sin_family = AF_INET;
	serverAddresssacServer.sin_port = htons(8003);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	int conectado = connect(sacServer, (struct sockaddr *) &serverAddresssacServer, sizeof(serverAddresssacServer));

	if( conectado == -1){
		exit(-1);
	}

	//close(sacServer);
}

void levantarConfigFile(config* pconfig){
	t_config* configuracion = leer_config();

	pconfig->ip = config_get_int_value(configuracion, "IP");
	pconfig->puerto = config_get_int_value(configuracion, "LISTEN_PORT");

}

t_config* leer_config() {
	return config_create("sacCli_config");
}


t_log * crear_log() {
	return log_create("fuse.log", "fuse", 1, LOG_LEVEL_DEBUG);
}
