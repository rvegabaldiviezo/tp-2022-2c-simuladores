MODULO=$1

CYAN='\033[0;36m'
RED='\033[0;31m'
NC='\033[0m'

echo -e ""
echo -e "========================================================"
echo -e "============ COMIENZO INSTALACION DE ${CYAN}shared${NC} ============"
echo -e "========================================================"
echo -e ""

# Compilar los shared
(cd shared/Debug && make clean && make all)

if [ $? -ne 0 ]; then
    echo -e "${RED}Fallo la compilacion de las shared${NC}"
    exit 1
fi

# Desinstalar shared del sistema
sudo rm -f /usr/lib/libshared.so
sudo rm -rf /usr/include/shared

# Instalo libshared.so
sudo cp -u shared/Debug/libshared.so /usr/lib

# Instalo los .h de shared
(
cd shared/src &&
H_SRCS=$(find . -iname "*.h" | tr '\n' ' ') &&
sudo cp --parents -u $H_SRCS /usr/include
)
echo -e "Instalo los .h de shared"

echo -e ""
echo -e "--------{ INSTALACION DE ${CYAN}shared${NC} FINALIZADA }----------"
echo -e ""

# Compilar modulo que se pasa...

echo -e ""
echo -e "========================================================"
echo -e "=========== COMIENZO COMPILACION DE ${CYAN}$MODULO${NC} ==========="
echo -e "========================================================"
echo -e ""

(cd $MODULO/Debug && make clean && make all)
if [ $? -ne 0 ]; then
    echo -e "${RED}Fallo la compilacion del modulo $MODULO${NC}"
    exit 1
fi

echo -e ""
echo -e "--------------{ MODULO ${CYAN}$MODULO${NC} COMPILADO }--------------"
echo -e ""