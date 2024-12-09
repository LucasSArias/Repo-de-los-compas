#include "filesystem.h"

int main(int argc, char *argv[])
{

	/* LEER ARCHIVO CONFIG, el path por default se lee desde .vscode/launch.json */
	load_config_from_file(validate_and_get_path(argc, argv));
    
    /*---------------Iniciar estructuras Filesystem---------------*/
    iniciar_estructuras_filesystem();

	/*---------*********---------*/	
    iniciar_servidor_filesystem();

    return 0;
}