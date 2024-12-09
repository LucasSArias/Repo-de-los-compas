#ifndef INTERRUPCIONES_H_
#define INTERRUPCIONES_H_

#include "bibliotecas.h"

typedef struct{
  op_code reason;
  uint32_t tid;
  uint32_t pid;
} t_interrupcion;

void* recibir_interrupciones_confirmadas(void *args);
t_interrupcion *recibir_interrupciones_confirmadas_cpu();

#endif /* INTERRUPCIONES_H_ */