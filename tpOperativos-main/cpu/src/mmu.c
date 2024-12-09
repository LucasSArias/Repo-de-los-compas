#include "mmu.h"

static bool _es_valido_direccion_fisica(uint32_t direccion_fisica);

uint32_t obtener_direccion_logica(e_registro registro_direccion)
{
    return obtener_valor_de_registro(registro_direccion);
}

uint32_t traducir_direccion_logica(uint32_t direccion_logica)
{
    uint32_t direccion_fisica = direccion_logica + contexto_ejecucion->base;
    if (_es_valido_direccion_fisica(direccion_fisica))
    {
        return direccion_fisica;
    }
    else
    {
        return -1;
    }
}

bool _es_valido_direccion_fisica(uint32_t direccion_fisica)
{
    return direccion_fisica >= contexto_ejecucion->base && direccion_fisica <= contexto_ejecucion->limite;
}