#!/bin/bash
echo "Ejecutando pruebas"
# Limpiar sockets
echo "Limpiando sockets"
kill $(lsof -t -i:8003)
kill $(lsof -t -i:8006)
kill $(lsof -t -i:8002)
kill $(lsof -t -i:8007)

# Variables
PRUEBA0=".config"
PRUEBA1="_prueba_planificacion.config"
PRUEBA2="_prueba_race_condition.config"
PRUEBA3="_prueba_fijas.config"
PRUEBA4="_prueba_dinamicas.config"
PRUEBA5="_prueba_fibonacci.config"
PRUEBA6="_prueba_stress.config"

FILENAME1="PLANI_PROC"
FILENAME2="RECURSOS_MUTEX_PROC"
FILENAME3="MEM_FIJA_BASE"
FILENAME4="MEM_DINAMICA_BASE"
FILENAME5="PRUEBA_FS"
FILENAME6="THE_EMPTINESS_MACHINE"

TAM_PROCESO1=32
TAM_PROCESO2=32
TAM_PROCESO3=12
TAM_PROCESO4=128
TAM_PROCESO5=8
TAM_PROCESO6=16

# Check if a numeric argument is provided
if [ -z "$1" ]; then
    echo "Please provide a test number (0-6)."
    exit 1
fi

# Select variables based on input number
case "$1" in
    0)
        PRUEBA=$PRUEBA0
        FILENAME=""
        TAM_PROCESO=0
        ;;
    1)
        PRUEBA=$PRUEBA1
        FILENAME=$FILENAME1
        TAM_PROCESO=$TAM_PROCESO1
        ;;
    2)
        PRUEBA=$PRUEBA2
        FILENAME=$FILENAME2
        TAM_PROCESO=$TAM_PROCESO2
        ;;
    3)
        PRUEBA=$PRUEBA3
        FILENAME=$FILENAME3
        TAM_PROCESO=$TAM_PROCESO3
        ;;
    4)
        PRUEBA=$PRUEBA4
        FILENAME=$FILENAME4
        TAM_PROCESO=$TAM_PROCESO4
        ;;
    5)
        PRUEBA=$PRUEBA5
        FILENAME=$FILENAME5
        TAM_PROCESO=$TAM_PROCESO5
        ;;
    6)
        PRUEBA=$PRUEBA6
        FILENAME=$FILENAME6
        TAM_PROCESO=$TAM_PROCESO6
        ;;
    *)
        echo "Invalid test number. Please provide a number between 0 and 6."
        exit 1
        ;;
esac

CONFIG=".config"

MODULO_FILESYSTEM="filesystem"
MODULO_MEMORIA="memoria"
MODULO_KERNEL="kernel"
MODULO_CPU="cpu"

RUTA_FILESYSTEM="../filesystem/"
RUTA_MEMORIA="../memoria/"
RUTA_KERNEL="../kernel/"
RUTA_CPU="../cpu/"

#ACA SE DEFINE QUE SE VA A EJECUTAR
#PRUEBA=$PRUEBA1
#FILENAME=$FILENAME1
#TAM_PROCESO=$TAM_PROCESO1

cd ../filesystem/
make clean && make release
./bin/filesystem "${RUTA_FILESYSTEM}${MODULO_FILESYSTEM}${PRUEBA}" &
sleep 2
cd ../memoria/
make clean && make release
./bin/memoria "${RUTA_MEMORIA}${MODULO_MEMORIA}${PRUEBA}" &
sleep 2
cd ../cpu/
make clean && make release
./bin/cpu "${RUTA_CPU}${MODULO_CPU}${CONFIG}" &
sleep 2
cd ../kernel/
make clean && make release
./bin/kernel "${RUTA_KERNEL}${MODULO_KERNEL}${PRUEBA}" "${FILENAME}" "${TAM_PROCESO}" &