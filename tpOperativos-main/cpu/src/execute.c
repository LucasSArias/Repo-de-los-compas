#include "execute.h"

void setear_registro(e_registro registro, uint32_t valor)
{
    switch (registro)
    {
    case PC:
        contexto_ejecucion->pc = valor;
        break;
    case AX:
        contexto_ejecucion->ax = valor;
        break;
    case BX:
        contexto_ejecucion->bx = valor;
        break;
    case CX:
        contexto_ejecucion->cx = valor;
        break;
    case DX:
        contexto_ejecucion->dx = valor;
        break;
    case EX:
        contexto_ejecucion->ex = valor;
        break;
    case FX:
        contexto_ejecucion->fx = valor;
        break;
    case GX:
        contexto_ejecucion->gx = valor;
        break;
    case HX:
        contexto_ejecucion->hx = valor;
        break;
    default:
        log_error(cpu_logger, "Registro no reconocido");
        exit(1);
    }
}

void leer_memoria(e_registro registro_datos, e_registro registro_direccion)
{
    uint32_t direccion_logica = obtener_direccion_logica(registro_direccion);
    uint32_t direccion_fisica = traducir_direccion_logica(direccion_logica);
    if(direccion_fisica == -1){
        //TODO: SEGMENTATION FAULT, QUE HAY QUE HACER?
        hay_segmentation_fault = true;
        log_error(cpu_logger, "Segmentation fault: Direccion fisica invalida. Direccion logica: %d", direccion_logica);
        return;
    }
    pedir_read_a_memoria(direccion_fisica);
    uint32_t valor = recibir_valor_read_mem();
    setear_registro(registro_datos, valor);
}

void escribir_memoria(e_registro registro_datos, e_registro registro_direccion)
{
    uint32_t valor = obtener_valor_de_registro(registro_datos);
    uint32_t direccion_logica = obtener_direccion_logica(registro_direccion);
    uint32_t direccion_fisica = traducir_direccion_logica(direccion_logica);
    if(direccion_fisica == -1){
        //TODO: SEGMENTATION FAULT, QUE HAY QUE HACER?
        hay_segmentation_fault = true;
        log_error(cpu_logger, "Segmentation fault: Direccion fisica invalida. Direccion logica: %d", direccion_logica);
        return;
    }
    pedir_escribir_a_memoria(valor, direccion_fisica);

    int resultado = recibir_entero(conexion_memoria);
    if(resultado != OK){
        log_error(cpu_logger, "No se pudo escribir en la memoria");
        exit(EXIT_FAILURE);
    }
}

void sumar_registros(e_registro registro_destino, e_registro registro_origen)
{
    uint32_t valor_origen = obtener_valor_de_registro(registro_origen);
    uint32_t valor_destino = obtener_valor_de_registro(registro_destino);
    valor_destino += valor_origen;
    setear_registro(registro_destino, valor_destino);
}

void restar_registros(e_registro registro_destino, e_registro registro_origen)
{
    uint32_t valor_origen = obtener_valor_de_registro(registro_origen);
    uint32_t valor_destino = obtener_valor_de_registro(registro_destino);
    valor_destino -= valor_origen;
    setear_registro(registro_destino, valor_destino);
}

void loggear_registro(e_registro registro)
{
    uint32_t valor = obtener_valor_de_registro(registro);
    log_info(cpu_logger, "Valor del registro %s: %d", enum_names_registros[registro], valor);
}

void execute_jnz(e_registro registro, uint32_t instruccion)
{
    if (obtener_valor_de_registro(registro) != 0)
    {
        contexto_ejecucion->pc = instruccion;
    } else 
    {
        contexto_ejecucion->pc = contexto_ejecucion->pc + 1;
    }
        
}

int obtener_valor_de_registro(e_registro registro)
{
    switch (registro)
    {
    case PC:
        return contexto_ejecucion->pc;
    case AX:
        return contexto_ejecucion->ax;
    case BX:
        return contexto_ejecucion->bx;
    case CX:
        return contexto_ejecucion->cx;
    case DX:
        return contexto_ejecucion->dx;
    case EX:
        return contexto_ejecucion->ex;
    case FX:
        return contexto_ejecucion->fx;
    case GX:
        return contexto_ejecucion->gx;
    case HX:
        return contexto_ejecucion->hx;
    default:
        log_error(cpu_logger, "Registro no reconocido");
        exit(1);
    }
}
