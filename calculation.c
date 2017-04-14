
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "longmath.h"


//
// This function handles the signs and invokes the correct 
// arithmetic operation. 
// 
int ComputeResult ( struct TermData *t1, int operator, 
                    struct TermData *t2, struct TermData *sol ) 
{ 
    int cmp; 
    TermInit ( sol ); // just good practice
    switch ( operator ) 
    { 
        case '+': 
        // addition really occurs only when the signs 
        // are the same. If signs differ, the operation 
        // is really subtraction. In that case, we call 
        // subtraction routines. Before calling 
        // NormalSubtract(), we make sure the terms are 
        // in the right order. See the comments in the 
        // subtraction section below for more info on this. 
        if ( t1->sign == t2->sign ) /* a + b or -a + -b */ 
        { 
            if ( ! NormalAdd ( t1, t2, sol )) 
            { 
                fprintf ( stderr, "Overflow on addition\n" ); 
                return ( 0 ); 
            } 
            sol->sign = t1->sign; 
        } 
        else 
        if ( t1->sign == -1 ) // -a + b
        { 
            sol->sign = -1; 
            cmp = NormAbsCmp ( t1, t2 ); 
            if ( cmp < 0 ) // t2 larger than t1
            { 
                sol->sign = -sol->sign; 
                NormalSubtract ( t2, t1, sol ); 
            } 
            else 
            if ( cmp > 0 ) // t1 larger than t2
                NormalSubtract ( t1, t2, sol ); 
            else 
            { // t1 = t2, so sol = 0
                TermInit ( sol ); 
                return ( 1 ); 
            } 
        } 
        else // a + -b
        { 
            sol->sign = +1; 
            cmp = NormAbsCmp ( t1, t2 ); 
            if ( cmp < 0 ) // t2 larger than t1
            { 
                sol->sign = -sol->sign; 
                NormalSubtract ( t2, t1, sol ); 
            } 
            else 
            if ( cmp > 0 ) // t1 larger than t2
                NormalSubtract ( t1, t2, sol ); 
            else 
            { // t1 = t2, so sol = 0
                TermInit ( sol ); 
                return ( 1 ); 
            } 
        } 
        break;



        case '-': 
        // there are four possible cases for subtraction, 
        // and each is treated different1y: 
        // a - b subtract a from b 
        // a - -b add a and b 
        // -a - b add a and b, multiply by -1 
        // -a - -b subtract a from b, multiply by -1 
        // 
        // multiplying by -1 is accomplished simply 
        // by flipping the sign of the result. 
        // Moreover, subtraction is set up so that we always 
        // subtract the smaller term from the larger. If t2 
        // is larger than t1, however, then we flip the 
        // sign of the solution and reverse the terms. This 
        // approach works because x-y=-l(y-x). That 
        // is, you can flip the terms in subtraction if you 
        // flip the sign of the difference: 
        // 3-7=-l*(7-3) 
        // 
        if ( t1->sign == +1 ) 
        { 
            if ( t2->sign == +1 ) // a - b
            { 
                sol->sign = +1; 
                cmp = NormAbsCmp ( t1, t2 ); 
                if ( cmp < 0 ) // t2 larger than t1
                { 
                    sol->sign = -sol->sign; 
                    NormalSubtract ( t2, t1, sol ); 
                } 
                else 
                if ( cmp > 0 ) // t1 larger than t2
                    NormalSubtract ( t1, t2, sol ); 
                else 
                { // t1 = t2, so sol = 0
                    TermInit ( sol ); 
                    return ( 1 ); 
                } 
            } 
            else 
            { // a b
            if ( ! NormalAdd ( t1, t2, sol )) 
            { 
            fprintf ( stderr, 
            "Overflow on addition\n" ); 
            return ( 0 ); 
            } 
            sol->sign = +1; 
            } 
        } 
        else 
        { // -a - b
            if ( t2->sign == +1 ) 
            { 
                if ( ! NormalAdd ( t1, t2, sol )) 
                { 
                fprintf ( stderr, 
                "Overflow on addition\n" ); 
                return ( 0 ); 
                } 
                sol->sign = -1; 
            } 
            else // -a - -b
            { 
                sol->sign = -1; 
                cmp = NormAbsCmp ( t1, t2 ); 
                if ( cmp < 0 ) // t2 larger than t1
                { 
                sol->sign = -sol->sign; 
                NormalSubtract ( t2, t1, sol ); 
                } 
                else 
                if ( cmp > 0 ) // t1 larger than t2
                NormalSubtract ( t1, t2, sol ); 
                else 
                { // t1 = t2, so sol = 0
                TermInit ( sol ); 
                return ( 1 ); 

                } 
            } 
        } 
        break; 

        case '*': 
        // 
        // multiplication sign issues are straightforward. 
        // just multiply and set the sign, depending on 
        // whether the terms have the same sign. 
        // 
        if ( ! NormalMultiply ( t1, t2, sol )) 
        { 
            fprintf ( stderr, "Overflow on mulitply\n" ); 
            return ( 0 ); 
        } 
        if ( t1->sign == t2->sign ) 
            sol->sign = +1; 
        else 
            sol->sign = -1; 
        break; 

        case '/': 
        // likewise for division
        if ( ! NormalDivide ( t1, t2, sol )) 
            return ( 0 );
 
        if ( t1->sign == t2->sign ) 
            sol->sign = +1; 
        else 
            sol->sign = -1; 
        break;

        default: 
        fprintf ( stderr, "Unsupported operation %c\n", operator ); 
        return ( 0 ); 
     
    }

    return 1;
}

// 
// We start at the rightmost digit of the terms, but no farther 
// left than the decimal point. We work our way left, adding as 
// we go until we reach the leftmost digit, and then go one digit 
// farther, in case of any carry. 
// 
int NormalAdd ( struct TermData *t1, struct TermData *t2, struct TermData *sum ) 
{ 
    int i, j, start, stop; 
    
    start = DEC_LOC + max ( t1->places_after, t2->places_after ); 
    stop = DEC_LOC - 1 - max ( t1->places_before, t2->places_before );

    for ( i = start; i >= stop; i-- ) 
    { 
        sum->term[i] += ( t1->term[i] + t2->term[i] ); 
        if ( sum->term[i] > 9 ) 
        { 
            sum->term[i] -= 10; 
            sum->term[i-1] += 1; 
        } 
    } 

    sum->places_after = start - DEC_LOC; 

    for ( i = 0; i < DEC_LOC; i++ ) 
        if ( (sum->term)[i] != 0 ) 
            break;
    sum->places_before = DEC_LOC - i; 

    // make sure that the sum is within MAX_SIZE digits

    i = sum->places_before + sum->places_after - MAX_SIZE; 
    if ( i > 0 ) // sum is larger than MAX_SIZE
    { 
        int carry = 0; 

        if ( i > sum->places_after ) 
            return ( 0 ); // high-order truncation will occur
        sum->places_after -= i; // adjust sum

        j = DEC_LOC + sum->places_after; // do rounding
        if ( (sum->term)[j] > 4 ) 
            carry = 1; 

        if ( carry ) 
            while ( 1 ) 
            { 
                j--; 

                if ( j == 0 && (sum->term)[j] > 8 ) 
                    return ( 0 ); 

                (sum->term)[j] += carry; 

                if ( (sum->term)[j] > 9 ) 
                    (sum->term)[j] -= 10; 
                else 
                    break; 
            } 

        while ( i-- ) // do low-order truncation
        { 
            (sum->term)[DEC_LOC + sum->places_after + i ] = 0; 
        } 
    } 
    return ( 1 ); 
} 

// 
// NormAbsCmp() works similar to strcmp(). The return value is 
// < 0, = 0, > 0 depending on whether the *absolute value* of 
// the first term is less than, equal to, or greater than the 
// second term, respectively. 
// 
int NormAbsCmp ( struct TermData * t1, struct TermData * t2 ) 
{ 
    int memcmp_result; 

    // first check the digits before the decimal point

    if ( t1->places_before > t2->places_before ) 
        return ( 1 ); 

    if ( t2->places_before > t1->places_before ) 
        return ( -1 ); 

    // 
    // same number of digits before decimal point, 
    // so compare character by character 
    // 
    memcmp_result = 
        memcmp ( t1->term + DEC_LOC - t1->places_before, 
                t2->term + DEC_LOC - t2->places_before, 
                t1->places_before + 
                max ( t1->places_after, t2->places_after )); 
    if ( memcmp_result > 0 ) 
        return ( 1 ); 
    else 
    if ( memcmp_result < 0 ) 
        return ( -1 ); 
    else 
        return ( 0 ); 
} 

// 
// subtraction: We first determine where to start the process 
// of subtraction. It is the rightmost digit of the two terms, 
// that is to the right of the decimal point, else it is 
// at the first digit left of the decimal point. We then 
// proceed from that digit (the least significant digit) and 
// move to the left until we reach the leftmost digit of the 
// two terms. Because of the possibility of a carry, we go one 
// digit more to the left. This duplicates manual subtraction. 
// 
int NormalSubtract ( struct TermData * t1, struct TermData * t2, struct TermData * diff ) 
{ 
    // 
    // The result of a subtraction is called a difference, 
    // hence we named the variable containing the answer diff. 
    // 
    int carry, i, j; 
    int max_after, max_before; 
    char *p1, *p2, *pd; 
    // 
    // we'll copy the terms to scratch since we'll be altering 
    // hence we named the variable containing the answer diff. 
    // 
    char scratch1 [2*MAX_SIZE +1], 
    scratch2 [2*MAX_SIZE +2]; 
    
    memcpy ( scratch1, t1->term, 2*MAX_SIZE + 1 ); 
    memcpy ( scratch2, t2->term, 2*MAX_SIZE + 1 ); 
    
    // where to start subtracting?
    
    max_before = max( t1->places_before, t2->places_before ); 
    max_after = max( t1->places_after, t2->places_after ); 

    j = max_before + max_after; 

    //
    // at worst, the rightmost digit is the last digit left of 
    // the decimal point. So, start no further left than there. 
    // 

    p1 = scratch1 + DEC_LOC + max_after - 1; 
    p2 = scratch2 + DEC_LOC + max_after - 1;
    pd = diff->term + DEC_LOC + max_after - 1; 
    while ( j >= 0 ) 
    { 
        // 
        // if there is a carry, borrow 10 and 
        // add 1 to the next higher digit in t2 
        // 
        if ( *p2 > *p1 ) 
        { 
            *p1 += 10; 
            *( p2 - 1 ) += 1; 
        } 
        *pd = *p1 - *p2; // the actual subtraction
        pd--; p1--; p2--; // move to the next higher digit
        j--; 
    }
 
    for ( j = 0, pd = diff->term; pd < diff->term + DEC_LOC; j++, pd++ ) 
        if ( *pd != 0 ) 
            break; 

    diff->places_before = DEC_LOC - j ; 

    // 
    // to get the number of places after, start at max_after, 
    // the maximum number of places after, and move left 
    // until the first nonzero digit right of the decimal pt. 
    // 

    pd = diff->term + DEC_LOC + max_after - 1; 
    while ( pd >= diff->term + DEC_LOC ) 
        if ( *pd == 0 ) 
        { 
            max_after -= 1; 
            pd -= 1; 
        } 
        else 
            break; 

    diff->places_after = max_after; 

    //
    // There can be too many digits in the result: subtract 
    // MAX_SIZE digits right of the decimal pt from MAX_SIZE 
    // digits left of the decimal point. If this occurs, we 
    // round and truncate. 
    // 

    i = diff->places_before + diff->places_after - MAX_SIZE; 

    if ( i > 0 ) // overflow
    { 
        diff->places_after = MAX_SIZE - diff->places_before; 
        i = DEC_LOC + diff->places_after; 
        if ( (diff->term)[i] > 4 ) // round up if > 4
            carry = 1; 
        else 
            carry = 0; 

        j = i; 
        i -= 1; 

        // add carry (the rounding)

        while ( carry ) 
        { 
            if ( i == 0 && (diff->term)[i] > 8 ) 
            { 
                printf ( "Overflow on subtraction\n" ); 
                return ( 0 ); 
            } 

            (diff->term)[i] += carry; 

            if ( (diff->term)[i] > 9 ) 
            { 
                (diff->term)[i] -= 10; 
                carry = 1; 
                i--; 
            } 
            else 
                carry = 0; 
        } 

        // now zero out the digits we have truncated

        while ( j < 2*MAX_SIZE ) 
            (diff->term)[j++] = 0; 
    } 

    return ( 1 ); 
} 

// 
// Multiplication of normalized terms. 
// In the expression c = a * b, a is called the multiplicand, 
// b the multiplier, and c the product. 
// 
int NormalMultiply ( struct TermData * t1, 
                        struct TermData * t2, struct TermData * prod ) 
{ 
    // 
    // Each digit of the multiplier will require the generation 
    // of an intermediate result, which is added to previous 
    // intermediate results, to produce the product. Hence, 
    // we'll make the multiplier the shorter of the two terms. 
    // 
    char *mcand, /* the multiplicand */ 
        *mier, /* the multiplier */ 
        *temp; /* temporary hold area for product */ 
    int mcand_curr, /* where we are in the muliplicand */ 
        mier_curr, /* where we are in the multiplier */ 
        temp_curr, /* where we are in the temp product */ 
        temp_here; 
    int mcand_len, /* number of digits in multiplicand */ 
        mier_len; /* number of digits in multiplier */ 
    int carry; /* the carry digit when adding the intermediate results */ 
    int i, j, from, to; 
    static int table [100] = 
    { 
        /*      0*0     0*1     0*2     0*3     0*4 */ 
        /* 0 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        /*      0*5     0*6     0*7     0*8     0*9 */ 
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        /*      1*0     1*1     1*2     1*3     1*4 */ 
        /* 1 */ 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 
        /*      1*5     1*6     1*7     1*8     1*9 */ 
                0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 
        /*      2*0     2*1     2*2     2*3     2*4 */ 
        /* 2 */ 0x0000, 0x0002, 0x0004, 0x0006, 0x0008, 
        /*      2*5     2*6     2*7     2*8     2*9 */ 
                0x0100, 0x0102, 0x0104, 0x0106, 0x0108, 
        /*      3*0     3*1     3*2     3*3     3*4 */ 
        /* 3 */ 0x0000, 0x0003, 0x0006, 0x0009, 0x0102, 
        /*      3*5     3*6     3*7     3*8     3*9 */ 
                0x0105, 0x0108, 0x0201, 0x0204, 0x0207, 
        /*      4*0     4*1     4*2     4*3     4*4 */ 
        /* 4 */ 0x0000, 0x0004, 0x0008, 0x0102, 0x0106, 
        /*      4*5     4*6     4*7     4*8     4*9 */ 
                0x0200, 0x0204, 0x0208, 0x0302, 0x0306, 
        /*      5*0     5*1     5*2     5*3     5*4 */ 
        /* 5 */ 0x0000, 0x0005, 0x0100, 0x0105, 0x0200, 
        /*      5*5     5*6     5*7     5*8     5*9 */ 
                0x0205, 0x0300, 0x0305, 0x0400, 0x0405, 
        /*      6*0     6*1     6*2     6*3     6*4 */ 
        /* 6 */ 0x0000, 0x0006, 0x0102, 0x0108, 0x0204, 
        /*      6*5     6*6     6*7     6*8     6*9 */ 
                0x0300, 0x0306, 0x0402, 0x0408, 0x0504, 
        /*      7*0     7*1     7*2     7*3     7*4 */ 
        /* 7 */ 0x0000, 0x0007, 0x0104, 0x0201, 0x0208, 
        /*      7*5     7*6     7*7     7*8     7*9 */ 
                0x0305, 0x0402, 0x0409, 0x0506, 0x0603, 
        /*      8*0     8*1     8*2     8*3     8*4 */ 
        /* 8 */ 0x0000, 0x0008, 0x0106, 0x0204, 0x0208, 
        /*      8*5     8*6     8*7     8*8     8*9 */ 
                0x0400, 0x0408, 0x0506, 0x0604, 0x0702, 
        /*      9*0     9*1     9*2     9*3     9*4 */ 
        /* 9 */ 0x0000, 0x0009, 0x0108, 0x0207, 0x0306, 
        /*      9*5     9*6     9*7     9*8     9*9 */ 
                0x0405, 0x0504, 0x0603, 0x0702, 0x0801 
    };

    static int mults_of_ten [10] = 
          { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90 } ; 

    mcand_len = mier_len = 0; 

    // 
    // products can overflow a term, since the number of 
    // digits left of the decimal pt in the product is the 
    // sum of the digits left of the decimal pt in the 
    // multiplier and multiplicand - 1. Likewise, the number 
    // of digits right of the decimal point is the sum of 
    // these digits in the multiplier and multiplicand. 
    // 
    // Hence, our internal representation of the product, 
    // temp, has 2*MAX_SIZE digits on either side of the 
    // decimal point. We will round and truncate as necessary 
    // when we load the digits into prod. 
    // 

    temp = calloc ( 1, 4*MAX_SIZE + 1 ); 
    if ( temp == NULL ) 
    { 
       printf ( "Out of memory in multiplication.\n" ); 
       return ( 0 ); 
    } 

    // 
    // the following test avoids guaranteed high-order 
    // truncation and saves having to do the multiplication 
    // only to discover the overflow at end. 
    // 
    if ( t1->places_before + t2->places_before - 1 > MAX_SIZE ) 
    { 
        free ( temp ); 
        return ( 0 ); 
    }
 
    if (( t1->places_before + t1->places_after ) > 
        ( t2->places_before + t2->places_after )) 
    { 
        mcand = t1->term; 
        mier = t2->term; 
        mcand_len = t1->places_before + t1->places_after; 
        mier_len = t2->places_before + t2->places_after; 
        mcand_curr = DEC_LOC + t1->places_after - 1; 
        mier_curr = DEC_LOC + t2->places_after - 1; 
    } 
    else 
    { 
        mcand = t2->term; 
        mier = t1->term; 
        mcand_len = t2->places_before + t2->places_after; 
        mier_len = t1->places_before + t1->places_after; 
        mcand_curr = DEC_LOC + t2->places_after - 1; 
        mier_curr = DEC_LOC + t1->places_after - 1; 
    } 
    
    // 
    // The number of digits after the decimal points in a 
    // product is the sum of the number of decimal digits 
    // in each term: 12.6 * 1.2 = 15.12. 
    // Hence, we start putting digits into prod, using the 
    // following formulation: 
    // 
    temp_curr = 2*DEC_LOC + 
    t1->places_after + t2->places_after - 1; 

    carry = 0; 
    while ( mier_len > 0 ) // the multiplication loop
    { // for each digit of multiplier
        int j, a, b, val; 
        i = mcand_len; 
        j = mcand_curr; 
        temp_here = temp_curr; 
        while ( i >= 0 ) // process the whole multiplicand
        { 
            if ( mier [ mier_curr ] == 0 ) 
               break; 
            a = mier [ mier_curr ]; 
            b = mcand [ j ]; 
            a += mults_of_ten [ b ]; 
            val = table [ a ]; 
            temp[ temp_here ] += carry + val & 0x00FF; 
            carry = val >> 8; 
            if ( temp[temp_here] > 9 ) 
            { 
               carry += temp [ temp_here ] / 10; 
               temp[ temp_here ] %= 10; 
            } 
            j--; // move up the multiplicand
            i--; // one less iteration to do
            temp_here--; // move one product digit to the left
        } 
        mier_curr--; // move up the multiplier
        mier_len--; // one less multiplier digit
        temp_curr--; // move up the solution by one digit
    } 

    if ( carry > 0 ) 
        temp [ temp_curr ] = carry; 

    for ( i = 0; i < 2*DEC_LOC; i++ ) 
        if ( temp[i] != 0 ) 
            break; 

    // did we overflow anyway?
    if (( 2*DEC_LOC - i ) > MAX_SIZE ) 
    { 
        free ( temp ); 
        return ( 0 ); 

    } 
    else 
        prod->places_before = 2*DEC_LOC - i; 

    // copy the digits before the dec pt from temp to prod
    from = 2*DEC_LOC - 1; 
    to = DEC_LOC - 1; 
    i = prod->places_before; 
    while ( i-- ) 
        (prod->term)[to--] = temp[from--]; 

    //
    // now examine the digits after the decimal point 
    // and perform, rounding and truncation as necessary 
    // 
    for ( i = 4*MAX_SIZE - 1; i >= 2*DEC_LOC; i-- ) 
        if ( temp[i] != 0 ) 
            break; 

    prod->places_after = i - 2*DEC_LOC + 1; 

    // j = maximum places after

    j = MAX_SIZE - prod->places_before; 

    // do we have to round and truncate?
    if ( j < prod->places_after ) 
    { 
        prod->places_after = j ; 
        if ( temp[2*DEC_LOC + j] > 4 ) // we round up if > 4 
            carry = 1; 
        else 
            carry = 0; 

        // copy the digits over
        from = 2*DEC_LOC + j - 1; 
        to = DEC_LOC + j - 1; 
        while ( j-- ) 
            (prod->term)[to--] = temp[from--]; 

        // now do the rounding  
        if ( carry ) 
        { 
            i = DEC_LOC + prod->places_after - 1; 
            while ( 1 ) 
            { 
                // 
                // we now add carry (the rounding). If 
                // the current digit is 9, the carry will 
                // generate a 10, meaning that we have to 
                // carry to the next digit left. If this 
                // occurs at the leftmost digit, it can 
                // cause an overflow, so this possibility 
                // is checked for first. 
                // 
                if ( i == 0 && // overflow
                    (prod->term)[i] > 8 ) 
                { 
                    free ( temp ); 
                    return ( 0 ); 
                } 

                (prod->term)[i] += carry; 
                if ( (prod->term)[i] > 9 ) 
                { 
                    (prod->term)[i] -= 10; 
                    i--;
                } 
                else 
                    break; 
            } 
        } 
    } 
    else // no truncation so just copy the digits 
    { 
        j = prod->places_after; 
        from = 2*DEC_LOC + j - 1; 
        to = DEC_LOC + j - 1; 
        while ( j-- ) 
            (prod->term)[to--] = temp[from--]; 
    } 

    free ( temp ); 
    return ( 1 ); 
}

/* 
* Division is accomplished by turning the term data into an 
* array of characters rather than bytes holding the numeric 
* value of each digit—the latter format is used in the other 
* arithmetic operations. However, in division, it becomes a 
* cumbersome format. (See text.) 
• • */ 

int NormalDivide ( struct TermData * dvend_arg, struct TermData * dvsor_arg, struct TermData * quot ) 
{ 
    #define ASCII_VAL ( '0' ) /* the difference between 0 and '0' */ 

    unsigned int 
        leading_zeros = 0, 
        divend_len, 
        divsor_len, 
        idivend_len; 

    int strcmp_return, 
        strncmp_return; 

    char *answer, 
        *dividend, 
        *divisor, 
        *interim, 
        *new_interim, 
        *product; 

    unsigned int 
        quo_guess, 
        next_dvend_digit = 0, 
        answer_idx = 0, 
        dvsor_len, 
        idvend_len; 


    int dvend_order, // order of magnitude of divisor and
       dvsor_order; // dividend. Used for locating decimal
    // point in quotient.
    char *pc; //:1q generic variables
    int i, j;


    dividend 
    divisor 
    interim 
    new_interim 
    product 
    answer 
    calloc 
    calloc 
    calloc 
    calloc 
    calloc 
    calloc 
    1, 2*MAX_SIZE + 1 ); 
    1, MAX__SIZE + 1 ); 
    1, 2*MAX_SIZE + 1 ); 
    1, 2*MAX_SIZE + 1 ) ; 
    2, 2*MAX__SIZE + 1 ); 
    2, MAX SIZE + 1 ); 
    if ( dividend == NULL || divisor == NULL || 
    interim == NULL || new_interim == NULL || 
    product == NULL j | answer == NULL ) 
    { 
    printf ( "Error allocating memory in division\n" ); 
    return ( 0 ); 
    } 
    /* load dividend with the digits as character values */ 
    i = 0; 
    for ( pc = dvend__arg->term + 
    DEC_LOC - dvend_arg->places_before; 
    pc <= dvend__arg->term + 
    DEC__LOC + dvend_arg->places__after - 1; 
    pc++, i++ ) 
    dividend[i] = *pc + ASCII__VAL; 
    /* remove leading zeros */ 
    while ( ^dividend == '0' ) 
    DivShiftArrayLeft ( dividend ); 
    /* likewise, for the divisor */ 
    i = 0; 
    for ( pc = dvsor_arg->term + 
    DEC_LOC - dvsor__arg->places__before; 
    pc <= dvsor___arg->term + 
    DEC_LOC + dvsor__arg->places__after - 1; 
    pc++, i++ ) 
    divisor[i] = *pc + ASCII VAL; 
    while ( *divisor == '0' ) 
    DivShiftSmallArrayLeft ( divisor ); 

