# TP Sistemas Operativos  - 2do Cuatrimestre 2022

| DESCRIPCION   |
|------------------|
| El objetivo del trabajo práctico consiste en desarrollar una solución que permita la simulación de un sistema distribuido, donde planificamos los procesos, resolvemos las  peticiones al sistema y administramos la memoria. Esta simulacion nos permite aplicar de forma práctica los conceptos teóricos dictados en la cursada. |
 
![Arquitectura del sistema](/shared/arquitecturaDelTP.png) 

|         TEMAS               |          LINK                   |
|------------------------|--------------------|   
| Programación en C  | [Buenas Prácticas de C](https://docs.utnso.com.ar/guias/programacion/buenas-practicas.html#buenas-practicas-de-c), [Introducción al Lenguaje C](https://docs.utnso.com.ar/primeros-pasos/lenguaje-c.html#introduccion-al-lenguaje-c), [Makefile - Informática I - FRBA - UTN](https://www.youtube.com/watch?v=A35l4jXBvEY&ab_channel=InformaticaI-UTN-FRBA) ,[Funciones en Lenguaje C - Informática I - FRBA - UTN](https://www.youtube.com/watch?v=aciC4izEiCo&ab_channel=InformaticaI-UTN-FRBA) ,[Estructuras y uniones - Informática I - FRBA - UTN](https://www.youtube.com/watch?v=Tw8PmTRuU_Q&ab_channel=InformaticaI-UTN-FRBA), [Variables y constantes en Lenguaje C - Informática I - FRBA - UTN](https://www.youtube.com/watch?v=DlG2K674O1E&ab_channel=InformaticaI-UTN-FRBA), [makefile : cómo construirlo](https://www.youtube.com/watch?v=0XlVyZAfQEM&t=1066s&ab_channel=WhileTrueThenDream), [Funciones de orden superior de las commons con más parámetros](https://www.youtube.com/watch?v=1kYyxZXGjp0&list=PL6oA23OrxDZDSe0ziMJ7iE-kPq9PdonPx&index=16&ab_channel=UTNSO) |
| Tests unitarios en C  | [Unit Testing con CSpec](https://docs.utnso.com.ar/guias/herramientas/cspec.html#unit-testing-con-cspec), [C - Unit Testing Introduction with Google Test](https://www.youtube.com/watch?v=BwO07hUzFNQ&ab_channel=KrisJordan) |
| Biblioteca compartida en C | [Crear una Biblioteca Compartida desde Eclipse 2020 + GCC Internals](https://www.youtube.com/watch?v=A6dhc9cCI18&ab_channel=UTNSO), [C - Shared Library](https://www.youtube.com/watch?v=Aw9kXFqWu_I&list=PL6oA23OrxDZDSe0ziMJ7iE-kPq9PdonPx&index=3&ab_channel=UTNSO) |
| Sockets  | [3ra Charla 1c 2019 - Sockets](https://www.youtube.com/watch?v=V0KFn9w62sY&ab_channel=UTNSO), [Sockets TCP. Linux. C: Ejemplo servidor secuencial.Breve teoría](https://www.youtube.com/watch?v=zFHjKCVwS48&t=1s&ab_channel=WhileTrueThenDream) |
| Serialización | [3ra Charla 1c 2019 - Serializacion](https://www.youtube.com/watch?v=GnuurOt8yqE&ab_channel=UTNSO), [Serialización](https://www.youtube.com/watch?v=gXr-zawbhIY&list=PLSwjRgubz0MaiiBb426tJxQoyIikVsNWK&index=7&ab_channel=LaCajadeUTN)|
| POSIX Threads | [Hilos y mutex en C usando pthread](https://www.youtube.com/watch?v=gl8FQU3VEzU&ab_channel=UTNSO),   [3ra Charla 1c 2019 - Threads](https://www.youtube.com/watch?v=G8PD6wauMeY&t=1770s&ab_channel=UTNSO), [Programar en C con Hilos pthreads](https://www.youtube.com/watch?v=NAKrOZCcJ4A&t=208s&ab_channel=WhileTrueThenDream) |
| Concurrencia | [video](), [Mutex. Sincronización de hilos. Programar en C](https://www.youtube.com/watch?v=faZEhIHdJx8&t=12s&ab_channel=WhileTrueThenDream)|
| Semáforos | [video]() , [3ra Charla 1c 2019 - Threads](https://www.youtube.com/watch?v=G8PD6wauMeY&t=1770s&ab_channel=UTNSO)|
| Gestión de memoria | [video](), [3ra Charla 1c 2019 - Valgrind](https://www.youtube.com/watch?v=knRei6OBU4Q&ab_channel=UTNSO) | 
| Segmentación Paginada |  [video]() |
| Planificación de procesos |  [Planificación - 2C2020](https://www.youtube.com/watch?v=SQsC7bwt3_c&ab_channel=UTNSO), [UTN 20 - SO: Procesos y Planificacion](https://www.youtube.com/watch?v=iOZLnOXQxVE&ab_channel=Snoopy4k),  [UTN 20 - SO, Teoria: Planificación de CPU ALgoritmos](https://www.youtube.com/watch?v=4J7hEXekn4M&ab_channel=Snoopy4k) |
| Multiprogramación |  [Multiprogramación, Gestión de memoria, Procesos](https://www.youtube.com/watch?v=oeuGAxxovxs&ab_channel=TelesensesSenses) |
| Multiprocesamiento | [UTN 20 - Sstemas Operativos, Teoria: Multicomputadoras y Sistemas Distribuidos](https://www.youtube.com/watch?v=yaKKhdeQ7FU&ab_channel=Snoopy4k) |
| Sistema de archivos |  [video]() |
| Bash Scripting |  [Guía de uso de Bash](https://docs.utnso.com.ar/guias/consola/bash.html#guia-de-uso-de-bash) |
| Automatización de deploy | [Guía de despliegue de TP](https://docs.utnso.com.ar/guias/herramientas/deploy.html#guia-de-despliegue-de-tp) |

## GRUPO: Simuladores

| Apellido y Nombre | GitHub user | Módulos a cargo |
|-------------------|-------------|-----------------|
| BERGES, Santiago   | [@sberges](https://www.github.com/sberges) | cpu |
| FRIED, Alan  | [@alu654](https://www.github.com/alu654) | memoria |
| RAMOS, Julieta | [@julietaramos](https://www.github.com/julietaramos) | cpu |
| ROBLES, Lautaro  | [@LautaroRobles](https://www.github.com/LautaroRobles) | consola, kernel, memoria | 
| VEGA B., Ramon Angel  | [@rvegabaldiviezo](https://www.github.com/rvegabaldiviezo) | kernel | 

## ENUNCIADO 

|       TP             |     LINK          |
|----------------------|-------------------|
| Gran Ejemplo de Creación de Kernels | [GECK](https://docs.google.com/document/d/1xYmkJXRRddM51fQZfxr3CEuhNtFCWe5YU7hhvsUnTtg/edit)|

## DEPLOY
#### Primero instalar las commons ejecutando
```bash
./commons
```
#### Segundo compilar los modulos
```bash
./buildall
```
#### Luego ejecutar los modulos
```bash
./run [modulo] [config]
```
## TESTING
#### Para probar muchos modulos a la vez (config es opcional)
```bash
./multirun [modulo1] [config]? [program?] [modulo2] [config]? [program?]...
```
Esto abre multiples terminales y ejecuta los modulos en el orden indicado usando la configuracion indicada (si no se pasa configuracion ejecuta los modulos con la config default)

Ejemplo: Abrir **cpu** despues **kernel** y por ultimo **consola** usando la config **default**
```bash
./multirun cpu kernel consola
```
Ejemplo: Abrir **memoria** y despues **cpu** usando la config **base1**
```bash
./multirun memoria cpu
```
