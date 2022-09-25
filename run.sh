MODULO=$1
CONFIG=$2

bash build.sh $MODULO

echo ""
echo "========================================================"
echo "========== COMIENZO EJECUCION DE [$MODULO] =========="
echo "========================================================"
echo ""

# Ejecuto el modulo pasado por parametro y pasandole todos sus parametros que se
# recibieron por consola
# Ej:
# bash play.sh consola arg1 arg2
# bash play.sh kernel
# 
# $@ = consola arg1 arg2
# ${@:2} = arg1 arg2
(cd $MODULO/Debug && ./$MODULO "../../config/$CONFIG/$MODULO.config" "../../config/$CONFIG/program.txt")