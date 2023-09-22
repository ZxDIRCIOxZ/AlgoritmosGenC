#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
 
#define TAMANIO_POBLACION 100
//limites de funcion
#define LIMITE_INFERIOR_FUNC -20
#define LIMITE_SUPERIOR_FUNC 20
//definicion de la precision
#define PRECISION 3
//definicion del individio mediante una estructura
typedef struct ind_t
{
    //definicion del cromosoma
    int *cromosoma;
    //definicion de variables para los genes
    double x;
    // variable de aptitud
    double aptitud;
    //arreglo para almacenar a los padres del individuo
    int padres[2];
    //variables para identificacion de parametros de lugra de mutacion y cruce
    int lugar_mutacion;
    int lugar_cruce;
} Individuo;
 
//creacion de conmutadores para decinion de padres y 
Individuo *padres;
Individuo *descendencia;
//variable global al individuo elitista
Individuo el_mejor;
 
double * ruleta;/*V3*/
 
//variable del tamaño del cromosoma
unsigned tamanio_cromosoma;
 
//se declara una variablle global como parametro del algoritmo genetico (V4)
double probabilidad_cruza;
 
double probabilidad_mutacion;
 
unsigned max_generaciones;
 
unsigned padre_seleccionado;
unsigned madre_seleccionada;
 
//declarar la funcion para obtener los parametros del algoritmo
void obtenerParametros()
{
    printf("\nNumero maximo de generaciones: "); scanf("%u", &max_generaciones);
    printf("\nProbabilidad de cruza: "); scanf("%lf", &probabilidad_cruza);
    printf("\nProbabilidad de mutacion: "); scanf("%lf", &probabilidad_mutacion);
}
 
//funcion que permite reservar memoria para las poblaciones
void asignarMemoria()
{
    //define numeros de bytes que se usaran a partir del tamaño por el tamaño de poblacion
    unsigned bytes_requeridos = sizeof(Individuo) * TAMANIO_POBLACION;
    padres = (Individuo *)malloc (bytes_requeridos);
    descendencia = (Individuo *)malloc (bytes_requeridos);
 
    //para la resrva de memoria para los cromosomas se debe iterar sobre los elementos de la poblacion
    //los numeros de bits que se usaran para cada cromosoma de la funcion a evaluar
    int i;
    //definicion del tamaño del cromosoma usando la formula
    tamanio_cromosoma = ceil(log2((LIMITE_SUPERIOR_FUNC - LIMITE_INFERIOR_FUNC) * pow(10, PRECISION)));
    
    /*definicion de los bytes requeridos tomando el tamaño del cromosoma
    bytes_requeridos = tamanio_cromosoma * sizeof(int);*/
 
    //solicitar la memoria para los cromosomas tomando en cuenta el tamaño 
    for (i = 0; i < TAMANIO_POBLACION; i++)
    {
        padres[i].cromosoma = (int *)malloc(bytes_requeridos);
        descendencia[i].cromosoma = (int *)malloc(bytes_requeridos);
 
        //asignar valores iniciales
        padres[i].x = descendencia[i].x = RAND_MAX;
        //se busca minimizar la optimizacion asignandole un valor muy grande
        padres[i].aptitud = descendencia[i].aptitud = 0;
        //aptitud(valores entre 0 y 1 a ambos padres)
        padres[i].padres[0] = descendencia[i].padres[0] = -1;
        //asignar el valor(-1) que esta fuera
        padres[i].padres[1] = descendencia[i].padres[1] = -1;
        padres[i].lugar_cruce = descendencia[i].lugar_cruce = -1;
        //para indicar que no tienen padres 
        padres[i].lugar_mutacion = descendencia[i].lugar_mutacion = -1;
    }
    el_mejor.cromosoma = (int *) calloc(tamanio_cromosoma, sizeof(int));
    ruleta = (double *) malloc(sizeof(double) * TAMANIO_POBLACION);
}
 
//funcion que genera los aleatorios sobre un intervalo a - b
double aleatorioDoble(double a, double b)
{
    return (b - a) * (rand() / (double)RAND_MAX) + a;
    //con esto se obtiene 
}
 
//para asegurar el proceso de la cracion de la primer generacion sea aleatoria se requiere de una simulacion del azar se usa la funcion volado()
int volado(double probabilidad)
//esta funcion pedira la probabilidad
{
    if (aleatorioDoble(0, 1) < probabilidad)
         return 0;
    else
         return 1;
}
 
//funcion para inicializar la primer generacion
void crearPrimerGeneracion()
{
    //itera sobre los padres
    int i, j;
    for(i = 0; i < TAMANIO_POBLACION; i++)
    {
        for(j = 0; j < tamanio_cromosoma; j++)
            padres[i].cromosoma[j] = volado(0.5);
    }
}
/*codigo agregado a la segunda version para calcular la aptitud (Version 2)*/
double binarioAreal(int * cromosoma)
{
    int i;
    double aux = 0.0;
    for (i = tamanio_cromosoma - 1; i >= 0; i--)
        aux += (double) pow(2, tamanio_cromosoma -i -1);
    return LIMITE_INFERIOR_FUNC + ((aux * (LIMITE_SUPERIOR_FUNC - LIMITE_INFERIOR_FUNC))/(pow(2, tamanio_cromosoma)-1));
}
 
void evaluarFuncionObjetivo(Individuo * individuo)
{
    individuo -> x = binarioAreal(individuo -> cromosoma);
    individuo -> aptitud = 1 / pow(individuo -> x, 2);
}
 
void evaluarPoblacion(Individuo * poblacion)
{
    int i;
    for(i = 0; i < TAMANIO_POBLACION; i++)
        evaluarFuncionObjetivo(&poblacion[i]); //& operador de acceso
}
 
/*version 3*/
void actualizarRuleta(Individuo * poblacion)
{
    int i;
    double suma_aptitudes = 0.0;
    for(i = 0; i < TAMANIO_POBLACION; i++)
        suma_aptitudes += poblacion[i].aptitud;
    for(i = 0; i < TAMANIO_POBLACION; i++)
        ruleta[i] = poblacion[i].aptitud / suma_aptitudes;
}
 
unsigned seleccionPorRuleta()
{
    double r = aleatorioDoble(0, 1);
    double suma = 0.0;
    int i, individuo_actual;
    for(i = TAMANIO_POBLACION; suma < r; i++)
    {
        individuo_actual = i % TAMANIO_POBLACION;
        suma += ruleta[individuo_actual];
    }
    return i;
}
 
//recibe como argumento a los dos padres y a los dos hijos
void cruza(Individuo * padre, Individuo * madre, Individuo * hijo1, Individuo * hijo2)
{
    //aplicar proceso probabilistico
    int i; //variable para iterador
    if(volado(probabilidad_cruza))
    {
        //se elige un punto al azar para el punto cruza
        unsigned p = (unsigned) aleatorioDoble(1, tamanio_cromosoma - 2);
 
        // inicio ciclo
        for(i = 0; i <= p; i++) //itera desde 0 hasta que i sea igual o menor al padre
        {
            //se implementa el paso de los primeros alelos del padre
            hijo1 -> cromosoma[i] = padre -> cromosoma[i];
            //se implementa el paso de los primeros alelos de la madre
            hijo2 -> cromosoma[p + i] = madre -> cromosoma[i];
        }
 
        //uso de la parte que sobra del cromosoma del padre
        for(i = p + 1; i < tamanio_cromosoma; i++)
        {
            //se implementa el paso de los ultimos alelos de la madre
            hijo1 -> cromosoma[i] = madre -> cromosoma[i];
            //se implementa el paso de los ultimos alelos del padre
            hijo2 -> cromosoma[i - p - 1] = padre -> cromosoma[i];
        }
        //si se realiza la cruza, indicar en que lugar se realizara
        hijo1 -> lugar_cruce = hijo2 -> lugar_cruce = p;
    }
    //en caso de que no se realice la cruza se realiza una copia identica de los padres
    else
    {
        for(i = 0; i < tamanio_cromosoma; i++)
        {
            hijo1 -> cromosoma[i] = padre -> cromosoma[i];
            hijo2 -> cromosoma[i] = madre -> cromosoma[i];
        }
        //como no hubo cruza se indica con valor invalido
        hijo1 -> lugar_cruce = hijo2 -> lugar_cruce = -1;
        hijo1 -> padres[0] = hijo2 -> padres[0] = 0;
        hijo2 -> padres[1] = hijo2 -> padres[1] = 0;
    }
}
 
//recibe como argumento al individuo en el que se aplicara la mutacion
void mutacion(Individuo * individuo)
{
    //si hay probabilidad de mutacion se eliger un alelo al azar
    if(volado(probabilidad_mutacion))
    {
        //asignar un lugar de mutacion a partir del valor devuelto por la funcion aleatoria doble
        unsigned m = (unsigned) aleatorioDoble(0, tamanio_cromosoma - 1);
        //una vez que se tiene el lugar de mutacion se hace cambio de valor
        individuo -> cromosoma[m] = 1 - individuo -> cromosoma[m];
        //indicar el lugar de mutacion el individuo
        individuo -> lugar_mutacion = m;
    }
    //en caso de que no haya mutacion se indica el valor invalido
    else
    {
        individuo -> lugar_mutacion = - 1;
    } 
}
 
//funcion elitista
void elitismo()
{
    unsigned mejor_padre;
    //guardado de numero de referencia a la posicion de los dos peores hijos
    unsigned peor_hijo1, peor_hijo2;
    //iterar sobre los individuos de toda la poblacion para encontrar a los peores hijos
    int i;
    //inicializar a los peores hijos con unvalor inicial en 0
    mejor_padre = peor_hijo1 = peor_hijo2 = 0;
    //el mejor individuo(elitista) sera el primer padre
    el_mejor = padres[i];
    //se recorre con toda la poblacion
    for(i = 0; i < TAMANIO_POBLACION; i ++)
    {
        //identificar los peores hijos
        if(descendencia[i].aptitud < descendencia[peor_hijo1].aptitud)
            peor_hijo1 = i;
        else if(descendencia[i].aptitud < descendencia[peor_hijo2].aptitud)
            peor_hijo2 = i;
        //identificar al individuo elitista
        if(padres[i].aptitud > padres[mejor_padre].aptitud)
            mejor_padre = i;
    }
    descendencia[peor_hijo1] = padres[mejor_padre];
    descendencia[peor_hijo2] = padres[mejor_padre];
}
 
void imprimirCromosoma(Individuo * individuo)
{
    int i;
    for(i = 0; i < tamanio_cromosoma; i++)
    {
        if(i == individuo -> lugar_mutacion)
            printf("(");
            printf("%d", individuo -> cromosoma[i]);
        if(i == individuo -> lugar_mutacion)
            printf("(");
        if(i == individuo -> lugar_cruce)
            printf("/");
    }
}
 
void imprimirDetallesPoblacion(Individuo * poblacion)
{
    int i;
    printf("\n\n----------------------------------------\n");
    printf(" #\tCromosoma\tx\tAptitud\tPadres");
    printf("\n\n----------------------------------------\n");
 
    for(i = 0; i < TAMANIO_POBLACION; i++)
    {
        printf("\n%03d ", i + 1);
        imprimirCromosoma(&poblacion[i]);
        printf(" %.3f\t%.3f\t(%d,%d)", poblacion[i].x, poblacion[i].aptitud, poblacion[i].padres[0], poblacion[i].padres[1]);
    }
}
 
//declarar la funcion main y su retorno en 0
int main()
{
    obtenerParametros();
    //inicializa el generador de numeros aleatorio tomando como parametro de generacion el paso del tiempo
    srand((long)time(NULL));
    //solicitar la memoria para la estructura de nuestros individuos
    asignarMemoria();
    //empieza la implementacion del algoritmo genetico y se crea la primer generacion
    crearPrimerGeneracion();
    evaluarPoblacion(padres); /*valores de cromosomas, bits (V2)*/
 
    Individuo * ayudante_temporal;
 
    int generacion, i;
    for(generacion = 0; generacion < max_generaciones; generacion++)
    {
        actualizarRuleta(padres);
        imprimirDetallesPoblacion(padres);
        for(i = 0; i < TAMANIO_POBLACION - 1; i += 2)
        {
            padre_seleccionado = seleccionPorRuleta();
            madre_seleccionada = seleccionPorRuleta();
            cruza(&padres[padre_seleccionado], &padres[madre_seleccionada], &descendencia[i], &descendencia[i + 1]);
            mutacion(&descendencia[i]);
            mutacion(&descendencia[i + 1]);
            evaluarFuncionObjetivo(&descendencia[i]);
            evaluarFuncionObjetivo(&descendencia[i + 1]);
        }
 
        elitismo();
        ayudante_temporal = padres;
        padres = descendencia;
        descendencia = ayudante_temporal;
        printf("\n\n\nTermino con exito generacion %d\n", generacion + 1);
    }
 
    free(padres);
    free(descendencia);
    free(ruleta);
 
    return 0;
}