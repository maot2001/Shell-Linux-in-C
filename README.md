# Shell-Linux-in-C

El proyecto esta estructurado de la siguiente manera:
1- Declaracion de librerias, metodos, const char* y variables globales que fueron utilizadas.
2- Metodo main.
3- Metodos previamente nombrados.

1)
-Se utilizan librerias y const char que fueron necesarias dentro de los metodos que creamos.
-Las variables globales fueron necesarias debido al flujo del programa o la comunicacion entre metodos de distante ejecucion.

2)
-El flujo del programa esta dado por la inicializacion de algunas variables globales; luego se entra a un ciclo que es mantenido por una de estas variables y en donde se realizan tres acciones: lectura, procesamiento para casos especiales y ejecucion(dentro de la ejecucion ubicamos el proceso de separar caracteres, pero es perfectamente retirable de aqui).

3)Metodos importantes:
-Execute: En este se separan los # y las pipes, conformandose asi la cadena de comandos y en base a la ocurrencia de las pipes se decide que accion tomar. Para cualquier caso (excepto los metodos exit y cd cuando no hay pipe) la ejecucion del comando se lleva a cabo por hijos del proceso inicial utilizando las funciones fork y execvp. En el caso de que existan pipes, por cada pipe que existe hay 2 procesos que separa, por tanto si hay n pipes existen (n + 1) comandos a ejecutar, utilizando matrices para guardar estas pipes, array para los pid_t y tensores para los comandos se pueden ejecutar los casos de pipe y multipipe de la misma forma.
-Linked: Aqui se analizan las salidas y entradas de cada comando, cuando se encuentran se cambian utilizando la funcion dup2(), pero no es necesario dup() porque esto solo se hace dentro de procesos hijos del inicial.
-Quitlinks: Aqui se eliminan los carecteres de redireccionar entradas y salidas, asi como la ubicacion de a donde se quiere llevar a cabo ya que si se pasan en execvp pueden ser tomados como argumentos cuando no lo son, esto solo se llama dentro de Linked.
-Work: Este es el metodo que trabaja los comandos, separando los que fueron implementados por nosotros de los que no, siempre que se entra a el, es desde un proceso hijo.
Exsten otros metodo como Parser(encargado del procesamiento de casos especiales), Again y Againcomp(encargados del caso again), First(analisis y eliminacion de espacios iniciales), History(ubicar los comandos en el historial), Spliter(para cortar un char* por el caracter que se envie), Cd y Reroot(encargados de la redireccion de directorio principal), pero de estos no vemos una gran descripcion que dar, en su mayoria son funcionalidades pequeñas o puramente de trabajo con char*.

En resumen el flujo del programa pasa por:
-Inicializacion de variables.
-Recibir el/los comando(s).
-Analizar casos especiales (again, espacio inicial).
-Eliminacion de mensajes y separacion por pipes.
-Analisis de cada comando interno a una pipe por separado (o del comando sin pipes) por los procesos hijos.
-Redireccion de salidas y entradas.
-Ejecucion del comando.
-En caso de no ser "exit" el comando en cuestion, regresar a recibir.
