#include"utils.h"

t_log* logger;

int iniciar_servidor(void)
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	//assert(!"no implementado!");

	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	//El primer daot es el nombre de dominio o simplemente
	//la IP, mism caso en el puerto, es o un puerto dado
	//como string o el nombre de un servicio en cuyo caso segun wikipedia 
	//esta cosa buscara en algun archivo del sistema para ver como resolver ese asunto
	//hints es que tipo de servicio queremos. En este caso
	//es un socket que recibe streams de data, que es pasivo
	//y que usa el protocolo IP4
	//el ultimo valor es el servidor respuesta que tiene los datos
	//que solicitamos mas la data que necesite para funcionar
	//la funcion retorna algo que no sea 0 si dio un fallo
	int fallo = getaddrinfo(NULL, PUERTO, &hints, &servinfo);
	if (fallo)
		log_error(logger, "No pudimos registrar el servicio");

	// Creamos el socket de escucha del servidor
	// Esta funcion crea un socket con los datos de coneccion
	// solicitados
	socket_servidor = socket(servinfo->ai_family,
				servinfo->ai_socktype,
				servinfo->ai_protocol);
	// Asociamos el socket a un puerto
	//Hace que el socket pase a etar operacional
	//se usa reuseport para poder conectar y reconectar facil
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	//bind es para reclamar el puerto asignado como propio
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	//listen deja al servidor esperando por conecciones 
	//con somaxcon para que pueda recibir tantas como pueda
	listen(socket_servidor, SOMAXCONN);
	// Escuchamos las conexiones entrantes

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	//assert(!"no implementado!");

	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);

	log_info(logger, "Se conecto un cliente!");
	log_info(logger, "Socket cliente = <%d>", socket_cliente);

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}
