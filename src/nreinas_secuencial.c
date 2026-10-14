/**************************************************************************/
/* N-Queens Solutions  ver3.1               takaken July/2003             */
/**************************************************************************/
#include <stdio.h>
#include <stdlib.h>


/* Time in seconds from some point in the past */
double dwalltime();

#define  MAXSIZE  24
#define  MINSIZE   2

int  SIZE, SIZEE;
int  BOARD[MAXSIZE], *BOARDE, *BOARD1, *BOARD2;
int  MASK, TOPBIT, SIDEMASK, LASTMASK, ENDBIT;
int  BOUND1, BOUND2;

long int  COUNT8, COUNT4, COUNT2;
long int  TOTAL, UNIQUE;

/**********************************************/
/* Display the Board Image                    */
/**********************************************/
void Display(void)
{
    int  y, bit;

    printf("N= %d\n", SIZE);
    for (y=0; y<SIZE; y++) {
        for (bit=TOPBIT; bit; bit>>=1)
            printf("%s ", (BOARD[y] & bit)? "Q": "-");
        printf("\n");
    }
    printf("\n");
}
/**********************************************/
/* Check Unique Solutions                     */
/**********************************************/
void Check(void)
{
    int  *own, *you, bit, ptn;

    /* 90-degree rotation */
    if (*BOARD2 == 1) {
        for (ptn=2,own=BOARD+1; own<=BOARDE; own++,ptn<<=1) {
            bit = 1;
            for (you=BOARDE; *you!=ptn && *own>=bit; you--)
                bit <<= 1;
            if (*own > bit) return;
            if (*own < bit) break;
        }
        if (own > BOARDE) {
            COUNT2++;
            //Display();
            return;
        }
    }

    /* 180-degree rotation */
    if (*BOARDE == ENDBIT) {
        for (you=BOARDE-1,own=BOARD+1; own<=BOARDE; own++,you--) {
            bit = 1;
            for (ptn=TOPBIT; ptn!=*you && *own>=bit; ptn>>=1)
                bit <<= 1;
            if (*own > bit) return;
            if (*own < bit) break;
        }
        if (own > BOARDE) {
            COUNT4++;
            //Display();
            return;
        }
    }

    /* 270-degree rotation */
    if (*BOARD1 == TOPBIT) {
        for (ptn=TOPBIT>>1,own=BOARD+1; own<=BOARDE; own++,ptn>>=1) {
            bit = 1;
            for (you=BOARD; *you!=ptn && *own>=bit; you++)
                bit <<= 1;
            if (*own > bit) return;
            if (*own < bit) break;
        }
    }
    COUNT8++;
    //Display();
}
/**********************************************/
/* First queen is inside                      */
/**********************************************/
void Backtrack2(int y, int left, int down, int right)
{
    int  bitmap, bit;

    bitmap = MASK & ~(left | down | right);
    if (y == SIZEE) {
        if (bitmap) {
            if (!(bitmap & LASTMASK)) {
                BOARD[y] = bitmap;
                Check();
            }
        }
    } else {
        if (y < BOUND1) {
            bitmap |= SIDEMASK;
            bitmap ^= SIDEMASK;
        } else if (y == BOUND2) {
            if (!(down & SIDEMASK)) return;
            if ((down & SIDEMASK) != SIDEMASK) bitmap &= SIDEMASK;
        }
        while (bitmap) {
            bitmap ^= BOARD[y] = bit = -bitmap & bitmap;
            Backtrack2(y+1, (left | bit)<<1, down | bit, (right | bit)>>1);
        }
    }
}
/**********************************************/
/* First queen is in the corner               */
/**********************************************/
void Backtrack1(int y, int left, int down, int right)
{
    int  bitmap, bit;

    bitmap = MASK & ~(left | down | right); //lugares seguros donde poner una reina
    if (y == SIZEE) {
        if (bitmap) {
            BOARD[y] = bitmap;
            COUNT8++;
            //Display();
        }
    } else {
        if (y < BOUND1) {
            bitmap |= 2;
            bitmap ^= 2;
        }
       while (bitmap) {
            bitmap ^= BOARD[y] = bit = -bitmap & bitmap;
            Backtrack1(y+1, (left | bit)<<1, down | bit, (right | bit)>>1);
        }
    }
}
/**********************************************/
/* Search of N-Queens                         */
/**********************************************/
void NQueens(void)
{
    int  bit, cant;  //La variable bit se va a usar para representar la posición de una reina usando unos y ceros

    /* Initialize */
    COUNT8 = COUNT4 = COUNT2 = 0; //Pone en cero tres contadores. Estos llevarán la cuenta de las soluciones que se vayan encontrando, agrupadas según sus "simetrías" (es decir, si la solución genera 8, 4 o 2 tableros diferentes al rotarlo o reflejarlo como en un espejo).
    SIZEE  = SIZE - 1; //Calcula cuál es la última fila del tablero. Como se cuenta desde cero, al tamaño total se le resta uno.
    BOARDE = &BOARD[SIZEE]; //Crea un puntero que apunta directamente a la última fila del tablero.
    TOPBIT = 1 << SIZEE; //Crea un número binario que solo tiene un "1" en la posición de más a la izquierda del tablero. Sirve como marca para saber dónde está el borde izquierdo.
    MASK   = (1 << SIZE) - 1; //Crea una mascara de todos 1 

    /* 0:000000001 */
    /* 1:011111100 */
    BOARD[0] = 1; //Coloca la primera reina en la primera fila (fila 0), exactamente en la esquina derecha del todo.
    for (BOUND1=2; BOUND1<SIZEE; BOUND1++) { // For para ir probando diferentes posiciones para la segunda reina en la segunda fila, empezando desde la tercera columna (posición 2) hasta la anteultima columna, para evidar configutraciones repetidas mas adelante).
        BOARD[1] = bit = 1 << BOUND1; //Coloca esta segunda reina en la posición i(BOUND1) de la segunda fila.
        Backtrack1(2, (2 | bit)<<1, 1 | bit, bit>>1); //El objetivo de Backtrack1 es tomar estas dos primeras reinas ya puestas y explorar todas las combinaciones posibles para llenar el resto de las filas hacia abajo. Si logra colocar todas las reinas sin que se ataquen, se encargará de sumar un punto a los contadores.
    }
    /* 0:000001110 */
    SIDEMASK = LASTMASK = TOPBIT | 1; //Asigna dos mascaras con una filas de 0 salvo en la primera y ultima posicion, serve para ver simetrias
    ENDBIT = TOPBIT >> 1; //ENDBIT pasa a la posicion anteultima empezando de la izquierda
    for (BOUND1=1,BOUND2=SIZE-2; BOUND1<BOUND2; BOUND1++,BOUND2--) {// For que recorre la primera fila, es para probar colocar la primera reina en posiciones que no sean la esquina, empezando a moverse desde los lados hacia el centro de la primera fila.
        BOARD1 = &BOARD[BOUND1]; //SE guarda la filas  correspondientes a las posiciones de las comlumnas de la primera fila, para despues verificar la simetria
        BOARD2 = &BOARD[BOUND2]; //
        BOARD[0] = bit = 1 << BOUND1;  //Coloca la primera reina en la nueva posición que toca en este ciclo.
        Backtrack2(1, bit<<1, bit, bit>>1); //explorar todas las combinaciones para poner el resto de las reinas partiendo de esta nueva configuración inicial. La diferencia principal (lo que hace para afuera) es que está diseñada para manejar estos casos centrales y detectar si la solución que encuentra es única o es un reflejo de otra que ya se encontró
        LASTMASK |= LASTMASK>>1 | LASTMASK<<1; //Modifica  la mascara corriendole los 1 hacia adentro 1 posicion
        ENDBIT >>= 1; //desplaza el bit de la izquierda una posicion mas a la derecha
    }

    /* Unique and Total Solutions */
    UNIQUE = COUNT8     + COUNT4     + COUNT2; //Total solucones unicas
    TOTAL  = COUNT8 * 8 + COUNT4 * 4 + COUNT2 * 2; //Total soluciones (incluyendo rotaciones y espejos)
    
}

/**********************************************/
/* N-Queens Solutions MAIN                    */
/**********************************************/
int main(int argC, char *argV[])
{  double tIni, tFin;

    SIZE=atoi(argV[1]); 
    tIni= dwalltime();
    NQueens();
    tFin= dwalltime();

    printf("Número de resultados: %lu -  Tiempo Total: %f segundos \n", TOTAL, tFin-tIni);
    return 0;
}

#include <sys/time.h>

double dwalltime()
{
    double sec;
    struct timeval tv;

    gettimeofday(&tv,NULL);
    sec = tv.tv_sec + tv.tv_usec/1000000.0;
    return sec;
}