MODULO=$1
DEFAULT_CONFIG="default"
CONFIG=${2:-$DEFAULT_CONFIG}

CYAN='\033[0;36m'
RED='\033[0;31m'
NC='\033[0m'

bash build.sh $MODULO
if [ $? -ne 0 ]; then
    exit 1
fi

echo -e ""
echo -e "========================================================"
echo -e "============ COMIENZO EJECUCION DE ${CYAN}$MODULO${NC} ============="
echo -e "========================================================"
echo -e ""

# Ejecuto el modulo pasado por parametro y pasandole todos sus parametros que se
# recibieron por consola
# Ej:
# bash play.sh consola arg1 arg2
# bash play.sh kernel
# 
# $@ = consola arg1 arg2
# ${@:2} = arg1 arg2
(cd $MODULO/Debug && ./$MODULO "../../config/$CONFIG/$MODULO.config" "../../config/$CONFIG/program.txt")

echo -e ""
echo -e "----------{ EJECUCION DE ${CYAN}$MODULO${NC} FINALIZADA }-----------"
echo -e ""