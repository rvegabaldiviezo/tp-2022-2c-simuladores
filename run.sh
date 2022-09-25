MODULO=$1

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
(cd $MODULO/Debug && ./$MODULO ${@:2})