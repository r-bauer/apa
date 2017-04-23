// sqrtmain.c Listing 8-22 
// Uses the longmath routines to perform Newton's algorithm for 
// finding square roots. Compares the result to the C floating- 
// point function sqrt(). This implementation keeps approximating 
// until Newton's method no longer generates a difference 
// or until 50 iterations have been performed. Beyond 50 
// iterations, it is assumed that the algorithm is ping-ponging 
// around the final number. Square root of 525 with MAX_SIZE 
// = 20 is an example of this problem. 
// 
#include <stdio.h> 
#include <stdlib.h> 
#include <math.h> 
# include "longmath.h" 
void main ( int argc, char *argv[] ) 
{ 
    int i; 
    char buffer[2*MAX_SIZE]; 
    struct TermData *square;        // the original number
    struct TermData *xn, 
                    *xnl, 
                    *product;       // variables in equation
    struct TermData *two, 
                    *three;         // constants
    struct TermData *tempi, 
                    *temp2, 
                    *temp3, 
                    *z;             // intermediate results

    float fsquare, fsqrt;           // the floats we'll compare to

    square = TermCreate(); 
    xn = TermCreate(); 
    xnl = TermCreate(); 
    product = TermCreate(); 


    two = TermCreate(); 
    three = TermCreate(); 
    tempi = TermCreate(); 
    temp2 = TermCreate(); 
    temp3 = TermCreate(); 
    z = TermCreate(); 

    if ( argc < 2 ) 
    { 
        printf ( "Usage: sqrtmain number\n" ); 
        return; 
    } 
    AsciiToTerm ( argv[1], square ); 

    // no imaginary numbers in this version
    if ( square->sign == -1 ) 
    { 
        printf ( "Square cannot be negative\n" ); 
        return; 
    } 

    // square root of zero is zero
    if ( square->places_before + square->places_after == 0 ) 
    { 
        printf ( "0\n" ); 
        return; 
    } 

    //
    // We start with a guess of dividing square by 3, hence 
    // xn = square / 3.0 
    // 
    AsciiToTerm ( "3.0", three ); 
    if ( ! ComputeResult ( square, '/', three, xn )) 
    { 
        printf ( "error l\n" ); 
        return; 
    } 
    printf ( "Square root of %s\n", argv[1] ); 


    TermToAscii ( xn, buffer, NORMAL ); 
    printf ( "First guess is %s\n", buffer ); 
    AsciiToTerm ( "2.0", two ); 
    for ( i = 0; ; i++ ) // repeat until root or max 50 loops 
    { 
        // Newton's approach keeps approximating using 
        // this formula: 
        // xnl = ( square + ( xn * xn )) / ( 2.0 * xn ) 
        // 
        TermCopy ( z, xn ); 
        if ( ! ComputeResult ( z, '*', xn, temp2 )) 
        { 
            printf ( "error 2\n" ); 
            return; 
        }
        
        if ( ! ComputeResult ( square, '+', temp2, tempi )) 
        { 
            printf ( "error 3\n" ); 
            return; 
        }

        if ( ! ComputeResult ( two, '*', xn, temp3 )) 
        { 
            printf ( "error 4\n" ); 
            return; 
        }

        if ( ! ComputeResult ( tempi, '/', temp3, xnl )) 
        { 
            printf ( "error 5\n" ); 
            return; 
        }
        TermToAscii ( xnl, buffer, NORMAL ); 
        printf ( "%2d %s\n", i, buffer ); 

        // Are we done ?
        if ( ! NormAbsCmp ( xn, xnl )) 
            break; 

        if ( i > 49 ) 
        {
            printf ( "\nLast entry is the closest approxima" 
            "tion. Algorithm no longer converging\n" ); 
            break; 
        } 
        // The current xnl becomes the next xn
        TermCopy ( xn, xnl ); 
    } 

    // print the square root
    TermToAscii ( xnl, buffer, NORMAL ); 
    printf ( "\nSquare root: %s\n", buffer ); 

    // print the square of the square root
    TermCopy ( xn, xnl ); 
    if ( ! ComputeResult ( xn, '*', xnl, product )) 
    { 
        printf ( "error 6\n" ); 
        return; 
    } 
    TermToAscii ( product, buffer, NORMAL ); 
    printf ( "Computed square: %s\n", buffer ); 
    // print the difference
    if ( ! ComputeResult ( square, '-', product, xn )) 
    { 
        printf ( "error 7\n" ); 
        return; 
    } 
    TermToAscii ( xn, buffer, NORMAL ); 
    printf ( "Delta: %s\n", buffer ); 

    // now test the built-in floating-point routines
    fsquare = atof ( argv[1] ); 
    fsqrt = sqrt ( fsquare ); 
    printf ( "\n\nMath lib root: %4.12f, square: %4.12f\n", fsqrt, fsqrt * fsqrt ); 
    return; 
} 

