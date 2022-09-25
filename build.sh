MODULO=$1

echo ""
echo "========================================================"
echo "============ COMIENZO INSTALACION DE SHARED ============"
echo "========================================================"
echo ""

# Compilar los shared
(cd shared/Debug && make clean && make all)

# Desinstalar shared del sistema
sudo rm -f /usr/lib/libshared.so
sudo rm -rf /usr/include/shared
echo "Desinstalo shared del sistema"

# Instalo libshared.so
sudo cp -u shared/Debug/libshared.so /usr/lib
echo "Instalo libshared.so"

# Instalo los .h de shared
(
cd shared/src &&
H_SRCS=$(find . -iname "*.h" | tr '\n' ' ') &&
sudo cp --parents -u $H_SRCS /usr/include
)
echo "Instalo los .h de shared"

echo ""
echo "========================================================"
echo "=========== INSTALACION DE SHARED COMPLETADA ==========="
echo "========================================================"
echo ""

# Compilar modulo que se pasa...

echo ""
echo "========================================================"
echo "=========== COMIENZO COMPILACION DE [$MODULO] ==========="
echo "========================================================"
echo ""

(cd $MODULO/Debug && make clean && make all)

echo ""
echo "========================================================"
echo "============= MODULO [$MODULO] COMPILADO ============="
echo "========================================================"
echo ""