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

echo "=========== INSTALACION DE SHARED COMPLETADA ==========="

# Compilar modulos...

(cd consola/Debug && make clean && make all)
(cd kernel/Debug && make clean && make all)
#(cd cpu/Debug && make clean && make all)
#(cd memoria/Debug && make clean && make all)