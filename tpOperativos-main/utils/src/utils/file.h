#ifndef FILE_UTILS_H_
#define FILE_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>

#define SEPARATOR_PATH "/"

t_list *leer_archivo_por_linea(char *file_path, t_log *logger);

#endif /* FILE_UTILS_H_ */