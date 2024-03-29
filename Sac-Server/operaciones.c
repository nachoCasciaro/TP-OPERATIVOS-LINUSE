#include "operaciones.h"


void o_readDir(char* path, int cliente){

	int i, nodo = dameNodoDe(path);
	struct sac_file_t *node;

	if (nodo == -1){
		char* buffer = malloc(2 * sizeof(int));
		int res = -1;
		int tamanioRes = sizeof(int);
		memcpy(buffer, &tamanioRes, sizeof(int));
		memcpy(buffer + sizeof(int), &res, sizeof(int));
		send(cliente, buffer, 2* sizeof(int), 0);
	}

	node = inicioTablaDeNodos;

	pthread_rwlock_rdlock(&superLockeador); 

	char* directoriosPegoteados = string_new();

	for (i = 0; i < GFILEBYTABLE;  (i++)){
		if ((nodo==(node->bloque_padre)) & (((node->estado) == DIRECTORIO) | ((node->estado) == OCUPADO)))

			string_append(&directoriosPegoteados, (char *) &node->nombre_archivo[0]);
		    string_append(&directoriosPegoteados, ";");

			node = &node[1];
	}

	pthread_rwlock_unlock(&superLockeador); 

	// serializo directoriosPegoteados y se los envio a saccli
	char* buffer = malloc(3*sizeof(int) + strlen(directoriosPegoteados));

	int res = 0;
	int tamanioRes = sizeof(int);
	memcpy(buffer, &tamanioRes, sizeof(int));
	memcpy(buffer + sizeof(int), &res, sizeof(int));

	int tamanioDirectoriosPegoteados = strlen(directoriosPegoteados);
	memcpy(buffer + 2 * sizeof(int), &tamanioDirectoriosPegoteados, sizeof(int));
	memcpy(buffer + 3* sizeof(int), directoriosPegoteados, strlen(directoriosPegoteados));

	send(cliente, buffer, 3 * sizeof(int) + strlen(directoriosPegoteados), 0);

/*
 * FUNCIONAMIENTO ANTERIOR:
 *
   struct dirent *dp;
   char* pathNuevo = string_new();
   string_append(&pathNuevo, "/home/utnso/tp-2019-2c-Cbados/Sac-Server/miFS");
   string_append(&pathNuevo, path);
   DIR *dir = opendir(pathNuevo);
   if (!dir){
	  return;
   }
   // concateno todos los directorios
   char* directoriosPegoteados = string_new();
   while ((dp = readdir(dir)) != NULL) {
	   string_append(&directoriosPegoteados, &dp->d_name);
	   string_append(&directoriosPegoteados, ";");
   }
   // serializo directoriosPegoteados y se los envio a saccli
   char* buffer = malloc(sizeof(int) + strlen(directoriosPegoteados));
   int tamanioDirectoriosPegoteados = strlen(directoriosPegoteados);
   memcpy(buffer, &tamanioDirectoriosPegoteados, sizeof(int));
   memcpy(buffer + sizeof(int), directoriosPegoteados, strlen(directoriosPegoteados));
   send(cliente, buffer, sizeof(int) + strlen(directoriosPegoteados), 0);
   closedir(dir);
*/
}



void o_getAttr(char* path, int cliente){

	struct stat *stbuf = malloc(sizeof(struct stat));
	int nodo = dameNodoDe(path), respuesta = 0;
	if (nodo < 0){
		respuesta = 1;
	}
	struct sac_file_t *node;
	memset(stbuf, 0, sizeof(struct stat));

	if (nodo == -1){
		respuesta = -1;
	}

	if(respuesta == -1){
		void* buffer = malloc( 2 * sizeof(int) );
		int tamanioRes = sizeof(int);
		memcpy(buffer, &tamanioRes, sizeof(int));
		memcpy(buffer + sizeof(int), &respuesta, sizeof(int));

		send(cliente, buffer, 2 * sizeof(int), 0);
		return;
	}

	if (strcmp(path, "/") == 0){
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
		stbuf->st_size = 0; // ??????????

		respuesta = 1;

		void* buffer = malloc( 7 * sizeof(int) + sizeof(stbuf->st_mode));

		int tamanioResp = sizeof(int);
		memcpy(buffer, &tamanioResp, sizeof(int));
		memcpy(buffer + sizeof(int), &respuesta, sizeof(int));

		int tamanioStmode = sizeof(stbuf->st_mode);
		memcpy(buffer + 2 * sizeof(int), &tamanioStmode, sizeof(int));
		memcpy(buffer + 3 * sizeof(int), &stbuf->st_mode, sizeof(stbuf->st_mode));

		int tamanioStnlink = sizeof(int);
		memcpy(buffer + 3 * sizeof(int) + sizeof(stbuf->st_mode), &tamanioStnlink, sizeof(int));
		memcpy(buffer + 4 * sizeof(int) + sizeof(stbuf->st_mode), &stbuf->st_nlink, sizeof(int));

		int tamanioEscrito = sizeof(int);
		memcpy(buffer + 5 * sizeof(int) + sizeof(stbuf->st_size), &tamanioEscrito, sizeof(int));
		memcpy(buffer + 6 * sizeof(int) + sizeof(stbuf->st_size), &stbuf->st_size, sizeof(int));

		send(cliente, buffer, 7 * sizeof(int) + sizeof(stbuf->st_mode), 0);
	}
	else{
		pthread_rwlock_rdlock(&superLockeador);

		node = inicioTablaDeNodos;

		node = &(node[nodo-1]);
		int size = 0;

		if (node->estado == 2){
			stbuf->st_mode = S_IFDIR | 0777;
			stbuf->st_nlink = 2;
			size = 4096;
			stbuf->st_mtime = node->fecha_modificacion;
			stbuf->st_ctime = node->fecha_creacion;
			stbuf->st_atime = time(NULL);
			respuesta = 0;
		} else if(node->estado == 1){
			stbuf->st_mode = S_IFREG | 0777;
			stbuf->st_nlink = 1;
			size = node->tamanio_archivo;
			stbuf->st_mtime = node->fecha_modificacion;
			stbuf->st_ctime = node->fecha_creacion;
			stbuf->st_atime = time(NULL);
			respuesta = 0;
		}

		pthread_rwlock_unlock(&superLockeador);

		if(respuesta == 0){
			//Serializo respuesta = 0, stbuf
			void* buffer = malloc( 10 * sizeof(int) + sizeof(stbuf->st_mode) + sizeof(stbuf->st_mtime) + sizeof(stbuf->st_atime)+ sizeof(stbuf->st_ctime));

			int tamanioResp = sizeof(int);
			memcpy(buffer, &tamanioResp, sizeof(int));
			memcpy(buffer + sizeof(int), &respuesta, sizeof(int));

			int tamanioStmode = sizeof(stbuf->st_mode);
			memcpy(buffer + 2 * sizeof(int), &tamanioStmode, sizeof(int));
			memcpy(buffer + 3 * sizeof(int), &stbuf->st_mode, sizeof(stbuf->st_mode));

			int tamanioStnlink = sizeof(int);
			memcpy(buffer + 3 * sizeof(int) + sizeof(stbuf->st_mode), &tamanioStnlink, sizeof(int));
			memcpy(buffer + 4 * sizeof(int) + sizeof(stbuf->st_mode), &stbuf->st_nlink, sizeof(int));

			int tamanioEscrito = sizeof(int);
			memcpy(buffer + 5 * sizeof(int) + sizeof(stbuf->st_mode), &tamanioEscrito, sizeof(int));
			memcpy(buffer + 6 * sizeof(int) + sizeof(stbuf->st_mode), &size, sizeof(int));

			int tamanioModificacion = sizeof(int);
			memcpy(buffer + 7 * sizeof(int) + sizeof(stbuf->st_mode), &tamanioModificacion, sizeof(int));
			memcpy(buffer + 8 * sizeof(int) + sizeof(stbuf->st_mode), &stbuf->st_mtime, sizeof(stbuf->st_mtime));

			int tamanioCreacion = sizeof(int);
			memcpy(buffer + 8 * sizeof(int) + sizeof(stbuf->st_mode) + sizeof(stbuf->st_mtime), &tamanioCreacion, sizeof(int));
			memcpy(buffer + 9 * sizeof(int) + sizeof(stbuf->st_mode) + sizeof(stbuf->st_mtime), &stbuf->st_ctime, sizeof(stbuf->st_ctime));

			int tamanioAcceso = sizeof(int);
			memcpy(buffer + 9 * sizeof(int) + sizeof(stbuf->st_mode) + sizeof(stbuf->st_mtime) + sizeof(stbuf->st_ctime), &tamanioAcceso, sizeof(int));
			memcpy(buffer + 10 * sizeof(int) + sizeof(stbuf->st_mode) + sizeof(stbuf->st_mtime) + sizeof(stbuf->st_ctime), &stbuf->st_atime, sizeof(stbuf->st_atime));


			send(cliente, buffer, 10 * sizeof(int) + sizeof(stbuf->st_mode) + sizeof(stbuf->st_mtime) + sizeof(stbuf->st_atime)+ sizeof(stbuf->st_ctime) , 0);
		}
	}

/*
 * FUNCIONAMIENTO ANTERIOR:
 *
	char* path = string_new();
	string_append(&path, "/home/utnso/tp-2019-2c-Cbados/Sac-Server/miFS");
	string_append(&path, nombre);
	if( access( path, F_OK ) != -1 ) {
		// file exists
		struct stat file_info;
		//Serializo ok = 1, file_info (mode y nlink)
		void* buffer = malloc( 7 * sizeof(int) + sizeof(file_info.st_mode));
		int ok = 1;
		int tamanioOk = sizeof(int);
		memcpy(buffer, &tamanioOk, sizeof(int));
		memcpy(buffer + sizeof(int), &ok, sizeof(int));
		lstat(path, &file_info);
		int tamanioStmode = sizeof(file_info.st_mode);
		memcpy(buffer + 2 * sizeof(int), &tamanioStmode, sizeof(int));
		memcpy(buffer + 3 * sizeof(int), &file_info.st_mode, sizeof(file_info.st_mode));
		int tamanioStnlink = sizeof(int);
		memcpy(buffer + 3 * sizeof(int) + sizeof(file_info.st_mode), &tamanioStnlink, sizeof(int));
		memcpy(buffer + 4 * sizeof(int) + sizeof(file_info.st_mode), &file_info.st_nlink, sizeof(int));
		int tamanioEscrito = sizeof(int);
		memcpy(buffer + 5 * sizeof(int) + sizeof(file_info.st_size), &tamanioEscrito, sizeof(int));
		memcpy(buffer + 6 * sizeof(int) + sizeof(file_info.st_size), &file_info.st_size, sizeof(int));
		send(cliente, buffer, 7 * sizeof(int) + sizeof(file_info.st_mode), 0);
	} else {
	    // file doesn't exist
		//Serializo ok = 0
		void* buffer = malloc( 2 * sizeof(int) );
		int ok = 0;
		int tamanioOk = sizeof(int);
		memcpy(buffer, &tamanioOk, sizeof(int));
		memcpy(buffer + sizeof(int), &ok, sizeof(int));
		send(cliente, buffer, 2 * sizeof(int), 0);
	}
*/
}


int o_open(char* path){

	/*
	 * FUNCIONAMIENTO ANTERIOR:
	 *
	 * char* pathAppend = string_new();
	string_append(&pathAppend, "/home/utnso/tp-2019-2c-Cbados/Sac-Server/miFS");
	string_append(&pathAppend, path);
	int respuesta;
	if( access( pathAppend, F_OK ) != -1 ) {
	    // file exists
		respuesta = 1;
	} else {
	    // file doesn't exist
		respuesta = 0;
	}
	return respuesta;
	*/

	return 0;
}



int o_rmdir(char* path){

	int miNodo = dameNodoDe(path), respuesta = 0;
	if (miNodo == -1){
		return -1;
	}
	struct sac_file_t *nodo;
	nodo = &(inicioTablaDeNodos[-1]);
	nodo = &(nodo[miNodo]);

	eliminarRecursivamente(miNodo);

	pthread_rwlock_wrlock(&superLockeador);
	nodo->estado = BORRADO;
	pthread_rwlock_unlock(&superLockeador);

	return respuesta;
}



void eliminarRecursivamente(int miNodo){
	// Chequea si el directorio esta vacio.
	for (int i=0; i < 1024 ; i++){
		if (((&inicioTablaDeNodos[i])->estado != BORRADO) & ((&inicioTablaDeNodos[i])->bloque_padre == miNodo)) {
			if( (&inicioTablaDeNodos[i])->estado == OCUPADO ){
				//eliminas el archivo
				struct sac_file_t* file_data, *nodoo;
				ENABLE_DELETE_MODE;
				file_data = &(inicioTablaDeNodos[i - 1]);
				pthread_rwlock_wrlock(&superLockeador);
				eliminarNodos(file_data, 0, 0);
				DISABLE_DELETE_MODE;
				nodoo = &(inicioTablaDeNodos[-1]);
				nodoo = &(nodoo[i+1]);
				nodoo->estado = BORRADO;
				pthread_rwlock_unlock(&superLockeador);
			}
			if( (&inicioTablaDeNodos[i])->estado == DIRECTORIO ){
				struct sac_file_t *nodo;
				nodo = &(inicioTablaDeNodos[-1]);
				nodo = &(nodo[i+1]);
				eliminarRecursivamente(i + 1);
				pthread_rwlock_wrlock(&superLockeador);
				nodo->estado = BORRADO;
				pthread_rwlock_unlock(&superLockeador);

			}
		}
	}
}


/*
int o_rmdir_2(char* path){
	DIR *d = opendir(path);
	size_t path_len = strlen(path);
	int r = -1;
	if (d) {
		struct dirent *p;
		r = 0;
		while (!r && (p = readdir(d))) {
			int r2 = -1;
			char *buf;
			size_t len;
			// Skip the names "." and ".." as we don't want to recurse on them.
			if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
				continue;
			}
			len = path_len + strlen(p->d_name) + 2;
			buf = malloc(len);
			if (buf) {
				struct stat statbuf;
				snprintf(buf, len, "%s/%s", path, p->d_name);
				if (!stat(buf, &statbuf)) {
					if (S_ISDIR(statbuf.st_mode)) {
						r2 = o_rmdir_2(buf);
					} else {
						r2 = unlink(buf);
					}
				}
				free(buf);
			}
			r = r2;
		}
		closedir(d);
	}
	if (!r) {
		r = rmdir(path);
	}
	return r;
}
*/



int o_unlink(char* pathC){

	struct sac_file_t* file_data;

	int node = dameNodoDe(pathC);

	ENABLE_DELETE_MODE;

	file_data = &(inicioTablaDeNodos[node - 1]);

	pthread_rwlock_wrlock(&superLockeador);
	eliminarNodos(file_data, 0, 0);
	pthread_rwlock_unlock(&superLockeador);

	DISABLE_DELETE_MODE;

	return o_rmdir2(pathC);

/*
	 * FUNCIONAMIENTO ANTERIOR
	 *
	 * int ok;
	char* path = string_new();
	string_append(&path, "/home/utnso/tp-2019-2c-Cbados/Sac-Server/miFS");
	string_append(&path, pathC);
	if( access( path, F_OK ) != -1 ) {
	    // file exists
		// entonces lo elimino
		FILE *fp1;
		int status = remove(path);
		if (status == 0){
		    printf("%s file deleted successfully.\n", path);
			ok = 1;
		}
		else{
		    printf("Unable to delete the file\n");
		    perror("Following error occurred");
		    ok = 0;
		 }
	} else {
	    // file doesn't exist
		ok = 0;
	}
	return ok;
	*/
}


int o_rmdir2(char* path){

	int miNodo = dameNodoDe(path), i, respuesta = 0;
	if (miNodo == -1){
		return -1;
	}
	struct sac_file_t *node;

	pthread_rwlock_wrlock(&superLockeador);

	node = &(inicioTablaDeNodos[-1]);

	node = &(node[miNodo]);

	// Verifica si el directorio esta vacio
	for (i=0; i < 1024 ; i++){
		if (((&inicioTablaDeNodos[i])->estado != BORRADO) & ((&inicioTablaDeNodos[i])->bloque_padre == miNodo)) {
			respuesta = -1;
			goto finalizar;
		}
	}

	node->estado = BORRADO;

	finalizar:
	pthread_rwlock_unlock(&superLockeador);

	return respuesta;

/*
 * FUNCIONAMIENTO ANTERIOR:
 *
	char* path = string_new();
	string_append(&path, "/home/utnso/tp-2019-2c-Cbados/Sac-Server/miFS");
	string_append(&path, pathC);
	int retorno = o_rmdir_2(path);
	return retorno;
*/
}


int o_rename(char* pathAntiguo, char* nuevoPath){
	if (dameNodoDe(pathAntiguo) == -1){
		return -1;
	}

	char* nuevaRuta = malloc(strlen(nuevoPath) + 1);
	char* nuevoNombre = malloc(GFILENAMELENGTH + 1);
	char* tofree1 = nuevaRuta;
	char* tofree2 = nuevoNombre;
	dividirRuta(nuevoPath, &nuevaRuta, &nuevoNombre);
	int old_node = dameNodoDe(pathAntiguo), nuevoNodoPadre = dameNodoDe(nuevaRuta);

	pthread_rwlock_wrlock(&superLockeador);

	strcpy((char *) &(inicioTablaDeNodos[old_node - 1].nombre_archivo[0]), nuevoNombre);
	inicioTablaDeNodos[old_node -1].bloque_padre = nuevoNodoPadre;

	pthread_rwlock_unlock(&superLockeador);

	free(tofree1);
	free(tofree2);

	return 0;
}


int o_create(char* path){

	if (dameNodoDe(path) != -1){
		return 1;
	}

	int miNodo, i, respuesta;
	int bloqueLibre;
	struct sac_file_t *nodo;
	char *nombre = malloc(strlen(path) + 1), *nom_to_free = nombre;
	char *directorioMadre = malloc(strlen(path) + 1), *dir_to_free = directorioMadre;
	char *bloqueDeDatos;

	dividirRuta(path, &directorioMadre, &nombre);

	if (strcmp(directorioMadre, "/") == 0) {
		miNodo = 0;
	} else if ((miNodo = dameNodoDe(directorioMadre)) < 0){
		return 1;
	}

	nodo = inicioTablaDeNodos;

	pthread_rwlock_wrlock(&superLockeador);

	// Busca nodo libre
	for (i = 0; (nodo->estado != 0) & (i <= TAMANIO_TABLA_DE_NODOS); i++) {
		nodo = &(inicioTablaDeNodos[i]);
	}

	if (i > TAMANIO_TABLA_DE_NODOS){
		respuesta = -EDQUOT;
		goto finalizar;
	}

	nodo->estado = OCUPADO;
	strcpy((char*) &(nodo->nombre_archivo[0]), nombre);
	nodo->tamanio_archivo = 0;
	nodo->bloque_padre = miNodo;
	nodo->bloques_indirectos[0] = 0;
	nodo->fecha_creacion = nodo->fecha_modificacion = time(NULL);

	bloqueLibre = obtenerBloqueLibre();

	agregarBloqueLibre(nodo, bloqueLibre);

	bloqueLibre -= (HEADER + TAMANIO_TABLA_DE_NODOS + TAMANIO_BITMAP);
	bloqueDeDatos = (char*) &(inicioBloquesDeDatos[bloqueLibre]);

	memset(bloqueDeDatos, '\0', TAMANIO_BLOQUE);
	respuesta = 0;

	finalizar:
	free(nom_to_free);
	free(dir_to_free);

	pthread_rwlock_unlock(&superLockeador);

	return respuesta;

	/*
	 * FUNCIONAMIENTO ANTERIOR
	 *
	 * int ok;
	char* path = string_new();
	string_append(&path, "/home/utnso/tp-2019-2c-Cbados/Sac-Server/miFS");
	string_append(&path, pathC);
	if( access( path, F_OK ) != -1 ) {
	    // file exists
		ok = 0;
	} else {
	    // file doesn't exist
		ok = 1;
		FILE *fp1;
		fp1 = fopen (path, "w");
		fclose(fp1);
	}
	return ok;
	*/
}


int o_truncate(char* path, int espacioNuevo){

	if (espacioNuevo < 0) return -1;
	int nodoMadre = dameNodoDe(path);
	if (nodoMadre == -1) return -1;
	struct sac_file_t *node;
	int res = 0;

	pthread_rwlock_wrlock(&superLockeador);

	node = inicioTablaDeNodos;

	node = &(node[nodoMadre-1]);

	if (espacioNuevo > node->tamanio_archivo){
		// reservar los nodos correspondientes
		res = darleEspacioNuevo(node, (espacioNuevo - node->tamanio_archivo));
		if (res != 0) goto finalizar;

	} else {
		// borrar los nodos hasta ESE punto
		int punteroAEliminar;
		int data_to_delete;

		settearPosicion(&punteroAEliminar, &data_to_delete, 0, espacioNuevo);

		res = eliminarNodos(node, punteroAEliminar, data_to_delete);
		if(res != 0) goto finalizar;
	}

	node->tamanio_archivo = espacioNuevo;

	finalizar:

	pthread_rwlock_wrlock(&superLockeador);

	return res;
}



int o_mkdir(char* path){

	int nodoMadre, i, respuesta = 0;
	struct sac_file_t *node;
	char *nombre = malloc(strlen(path) + 1), *nom_to_free = nombre;
	char *direccionMadre = malloc(strlen(path) + 1), *dir_to_free = direccionMadre;

	if (dameNodoDe(path) != -1){
		return 1;
	}

	dividirRuta(path, &direccionMadre, &nombre);

	if (strcmp(direccionMadre, "/") == 0){
		nodoMadre = 0;
	} else if ((nodoMadre = dameNodoDe(direccionMadre)) < 0){
		return 1;
	}

	node = inicioTablaDeNodos;

	pthread_rwlock_wrlock(&superLockeador);

	for (i = 0; (node->estado != 0) & (i <= TAMANIO_TABLA_DE_NODOS); i++){
		node = &(inicioTablaDeNodos[i]);
	}

	if (i > TAMANIO_TABLA_DE_NODOS){
		respuesta = 1;
		goto finalizar;
	}

	node->estado = DIRECTORIO;
	strcpy((char*) &(node->nombre_archivo[0]), nombre);
	node->tamanio_archivo = 0;
	node->bloque_padre = nodoMadre;
	respuesta = 0;

	finalizar:
	free(nom_to_free);
	free(dir_to_free);

	pthread_rwlock_unlock(&superLockeador);

	return respuesta;

/*
 * FUNCIONAMIENTO ANTERIOR
 *
 * int ok;
	struct stat sb;
	char* folder = string_new();
	string_append(&folder, "/home/utnso/tp-2019-2c-Cbados/Sac-Server/miFS");
	string_append(&folder, path);
	if (stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode)) {
	     // folder exists
		ok = 0;
	} else {
		// folder doesn't exist
		int check;
		check = mkdir(folder, 0700);
		// check if directory is created or not
		if (!check){
		    printf("Directory created\n");
			ok = 1;
		}
		else {
		   printf("Unable to create directory\n");
		   ok = 0;
		}
	}
	return ok;
*/
}


int o_read(char* path, int size, int offset, char* buffer){

	unsigned int nodo = dameNodoDe(path), bloque_punteros, num_bloque_datos;
	unsigned int bloqueABuscar;
	struct sac_file_t *node;
	ptrGBloque *punteroABloqueDD;
	char *bloqueDeDatos;
	size_t tamanioQueMeQuedaPorLeer = size;
	int respuesta;

	if (nodo == -1){
		return -1;
	}

	node = inicioTablaDeNodos;
	node = &(node[nodo-1]);

	pthread_rwlock_rdlock(&superLockeador);

	if(node->tamanio_archivo <= offset){
		respuesta = 0;
		goto finalizar;
	} else if (node->tamanio_archivo <= (offset+size)){
		tamanioQueMeQuedaPorLeer = size = ((node->tamanio_archivo)-(offset));
	}
		// Recorre todos los punteros en el bloque de la tabla de nodos
		for (bloque_punteros = 0; bloque_punteros < BLOQUESINDIRECTOS; bloque_punteros++){

			// Chequea el offset y lo acomoda para leer lo que realmente necesita
			if (offset > TAMANIO_BLOQUE * 1024){
				offset -= (TAMANIO_BLOQUE * 1024);
				continue;
			}

			bloqueABuscar = (node->bloques_indirectos)[bloque_punteros];
			bloqueABuscar -= (GFILEBYBLOCK + TAMANIO_BITMAP + TAMANIO_TABLA_DE_NODOS);
			punteroABloqueDD =(ptrGBloque *) &(inicioBloquesDeDatos[bloqueABuscar]);

			// Recorre el bloque de punteros correspondiente.
			for (num_bloque_datos = 0; num_bloque_datos < 1024; num_bloque_datos++){

				// Chequea el offset y lo acomoda para leer lo que realmente necesita
				if (offset >= TAMANIO_BLOQUE){
					offset -= TAMANIO_BLOQUE;
					continue;
				}

				bloqueABuscar = punteroABloqueDD[num_bloque_datos]; 	// Ubica el nodo de datos correspondiente. Relativo al nodo 0: Header.
				bloqueABuscar -= (GFILEBYBLOCK + TAMANIO_BITMAP + TAMANIO_TABLA_DE_NODOS);	// Acomoda el nodo, haciendolo relativo al bloque de datos.
				bloqueDeDatos = (char *) &(inicioBloquesDeDatos[bloqueABuscar]);

				// Corre el offset hasta donde sea necesario para poder leer lo que quiere.
				if (offset > 0){
					bloqueDeDatos += offset;
					offset = 0;
				}

				if (tamanioQueMeQuedaPorLeer < TAMANIO_BLOQUE){
					//buf = malloc(4); //todo malloc de 4 para ejemplo de "jaja"
					//buf = malloc(strlen(data_block) +1);

					memcpy(buffer, bloqueDeDatos, tamanioQueMeQuedaPorLeer);
					buffer = &(buffer[tamanioQueMeQuedaPorLeer]);
					tamanioQueMeQuedaPorLeer = 0;
					break;
				} else {
					//buf = malloc(4); //todo malloc de 4 para ejemplo de "jaja"
					//buf = malloc(strlen(data_block) +1);

					memcpy(buffer, bloqueDeDatos, TAMANIO_BLOQUE);
					tamanioQueMeQuedaPorLeer -= TAMANIO_BLOQUE;
					buffer = &(buffer[TAMANIO_BLOQUE]);
					if (tamanioQueMeQuedaPorLeer == 0) break;
				}

			}

			if (tamanioQueMeQuedaPorLeer == 0) break;
		}
		respuesta = size;

		finalizar:
		pthread_rwlock_unlock(&superLockeador);


	return respuesta;

	/*
	 * FUNCIONAMIENTO ANTERIOR
	 *
	FILE *f;
	//open the file for write operation
	if( (f=fopen(path,"rb")) == NULL){
		//if the file does not exist print the string
		printf("No se pudo abrir el archivo");
	}
	fseek(f, offset, SEEK_SET);
	if( fread(texto, size, 1, f) == 0){
		printf("No leyo");
	}
	fclose(f);
*/
}




int o_write(char* path, int size, int offset, char* buf){

	int nodo = dameNodoDe(path);
	if (nodo == -1){
		loguearError(" - NO se pudo hacer el WRITE en SacServer\n");
	}
	int nodoLibre;
	struct sac_file_t *node;
	char *bloqueDeDatos;
	size_t tamanio = size, tamanioArchivo, espacioEnBloque, offsetEnBloque = offset % TAMANIO_BLOQUE;
	off_t off = offset;
	int *n_pointer_block = malloc(sizeof(int)), *n_data_block = malloc(sizeof(int));
	ptrGBloque *punteroABloque;
	int res = size;

	node = &(inicioTablaDeNodos[nodo-1]);
	tamanioArchivo = node->tamanio_archivo;

	if ((tamanioArchivo + size) >= THELARGESTFILE){
		loguearError(" - NO se pudo hacer el WRITE en SacServer\n");
	}

	pthread_rwlock_wrlock(&superLockeador);

	while (tamanio != 0){

		espacioEnBloque = TAMANIO_BLOQUE - (tamanioArchivo % TAMANIO_BLOQUE);
		if (espacioEnBloque == TAMANIO_BLOQUE){
			(espacioEnBloque = 0);
			// bloque lleno.
		}
		if (tamanioArchivo == 0){
			espacioEnBloque = TAMANIO_BLOQUE;
			/* archivo recien creado */
		}

		if ((off >= (tamanioArchivo + espacioEnBloque)) & (tamanioArchivo != 0)){

			// Si no hay espacio en el disco, retorna error.
			if (bitmap_free_blocks == 0){
				loguearError(" - NO se pudo hacer el WRITE en SacServer porque no hay espacio en el disco\n");
				res = -1;
				goto finalizar;
			}

			nodoLibre = obtenerBloqueLibre();
			if (nodoLibre < 0){
				goto finalizar;
			}

			res = agregarBloqueLibre(node, nodoLibre);
			if (res != 0){
				goto finalizar;
			}

			nodoLibre -= (HEADER + TAMANIO_TABLA_DE_NODOS + TAMANIO_BITMAP);
			bloqueDeDatos = (char*) &(inicioBloquesDeDatos[nodoLibre]);

			espacioEnBloque = TAMANIO_BLOQUE;

		} else {
			settearPosicion(n_pointer_block, n_data_block, tamanioArchivo, off);

			*n_pointer_block = node->bloques_indirectos[*n_pointer_block];
			*n_pointer_block -= (HEADER + TAMANIO_TABLA_DE_NODOS + TAMANIO_BITMAP);
			punteroABloque = (ptrGBloque*) &(inicioBloquesDeDatos[*n_pointer_block]);
			*n_data_block = punteroABloque[*n_data_block];
			*n_data_block -= (HEADER + TAMANIO_TABLA_DE_NODOS + TAMANIO_BITMAP);
			bloqueDeDatos = (char*) &(inicioBloquesDeDatos[*n_data_block]);
		}

		if (tamanio >= TAMANIO_BLOQUE){
			for(int m = 0; m < TAMANIO_BLOQUE; m++){
					bloqueDeDatos[m] = buf[m];

			}
			//memcpy(bloqueDeDatos, buf, BLOCKSIZE);
			if ((node->tamanio_archivo) <= (off)) tamanioArchivo = node->tamanio_archivo += TAMANIO_BLOQUE;
			buf += TAMANIO_BLOQUE;
			off += TAMANIO_BLOQUE;
			tamanio -= TAMANIO_BLOQUE;
			offsetEnBloque = 0;
		} else if (tamanio <= espacioEnBloque){ /*Hay lugar suficiente en ese bloque pra escribir el resto del archivo */
			memcpy(bloqueDeDatos + offsetEnBloque, buf, tamanio);
			if (node->tamanio_archivo <= off) tamanioArchivo = node->tamanio_archivo += tamanio;
			else if (node->tamanio_archivo <= (off + tamanio)) tamanioArchivo = node->tamanio_archivo += (off + tamanio - node->tamanio_archivo);
			tamanio = 0;
		} else { /* No hay lugar suficiente, llena el bloque y vuelve a buscar uno nuevo */
			memcpy(bloqueDeDatos + offsetEnBloque, buf, espacioEnBloque);
			tamanioArchivo = node->tamanio_archivo += espacioEnBloque;
			buf += espacioEnBloque;
			off += espacioEnBloque;
			tamanio -= espacioEnBloque;
			offsetEnBloque = 0;
		}
	}

		node->fecha_modificacion= time(NULL);

		res = size;

		finalizar:
		pthread_rwlock_unlock(&superLockeador);
		return res;

/*
 * FUNCIONAMIENTO ANTERIOR:
 *
	FILE *f;
	f = fopen(path,"wb");
	if(f){
		fseek(f, offset, SEEK_SET);
		fwrite(buffer, 1, size, f);
	}
	else{
		printf("write : no se pudo abrir el archivo");
	}
	fclose(f);
*/
}




