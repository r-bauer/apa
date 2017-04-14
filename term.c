
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "longmath.h"





/* 
* Converts a null-terminated ASCII number in either normal or 
* scientific notation to a term. 
• 
* If the first character is a sign or a decimal point, we 
* process it and replace it with a 0, so as to enable 
* processing of a string of digits. However, to maintain the 
* integrity of the ASCII string, we save the original first 
* character in first_char and restore it before returning. 
* This is of use only when the calculator is reading input 
* from a file. 
* 
* Returns NORMAL, SCIENTIFIC depending on format of string, 
* or returns 0 on error. 
* */
int
AsciiToTerm (char *buffer, struct TermData *t)
{
  char *dec_pt, *p;
  int i, exponent, len, notation;
  char *new_term;
  char first_char;
  char *exp;                    // where the exponent flag is


  new_term = t->term;           // point new__term to where it'll go

  // is it scientific notation?

  exp = strpbrk (buffer, "eE");
  notation = (exp == NULL ? NORMAL : SCIENTIFIC);

  first_char = *buffer;

  if (isdigit (*buffer))        // get the sign
    t->sign = +1;
  else
    {
      if (*buffer == '-')
        t->sign = -1;
      else if (*buffer == '+')
        t->sign = +1;
      else if (*buffer != '.')
        {
          t->sign = 0;          // flag error
          return (0);
        }

      // replace any leading sign by a zero
      if (*buffer != '.')
        *buffer = '0';
    }

  dec_pt = strchr (buffer, '.');
  len = strlen (buffer);

  // load the digits after the decimal point. The first 
  // digit goes at term[DEC__LOC], subsequent digits 
  // go to the right up to MAX_SIZE digits.  
  if (dec_pt != NULL)           // Only if there's a decimal point
    {
      p = dec_pt + 1;
      for (i = DEC_LOC; *p && i != 2 * MAX_SIZE; i++, p++)
        {
          if (!isdigit (*p))    // check that it's a digit
            {
              if (notation == NORMAL)
                {
                  t->sign = 0;  // if not, show an error
                  break;
                }
              else              // if it's scientific notation...
                {
                  if (*p == 'e' || *p == 'E')
                    break;
                  else
                    {
                      t->sign = 0;      // show an error
                      break;
                    }
                }
            }
          else
            new_term[i] = *p - '0';
        }
    }
  if (notation == SCIENTIFIC)
    {
      p = exp + 1;
      exponent = atoi (p);
    }

  if (t->sign == 0)             // any error so far ? 
    return (0);

  // load the digits before the decimal point. You load at 
  // the first place right of buffer[DEC__LOC] and add digits 
  // to the left up to MAX_SIZE digits. 

  if (dec_pt == NULL)
    {
      if (notation == NORMAL)
        p = buffer + len - 1;
      else
        p = exp - 1;
    }
  else
    p = dec_pt - 1;

  for (i = DEC_LOC - 1; p >= buffer && i >= 0; i--, p--)
    {
      if (!isdigit (*p))        // check that it's a digit
        {
          t->sign = 0;          // if not, indicate an error
          break;
        }
      else
        {
          new_term[i] = *p - '0';
          if (p == buffer)      // this test for pointer wrap- 
            break;              // around on Intel segments
        }
    }

  // if it's scientific notation, shift the term right or left 
  // depending on the exponent. If the exponent is > 0, shift 
  // left exponent number of places; if it's < 0, shift right.  
  if (notation == SCIENTIFIC)
    {
      if (exponent > 0)
        {
          while (exponent--)    // shift left
            {
              if (*new_term > 0)
                {
                  printf ("Error: %s too large\n", buffer);
                  return (0);
                }
              memmove (new_term, new_term + 1, 2 * MAX_SIZE - 1);
              new_term[2 * MAX_SIZE - 1] = 0;
            }
        }
      else if (exponent < 0)    // shift right
        {
          int warning = 0;
          while (exponent++)
            {
              if (new_term[2 * MAX_SIZE - 1] > 0)
                warning += 1;
              memmove (new_term + 1, new_term, 2 * MAX_SIZE - 1);
              *new_term = 0;
            }
          if (warning)
            printf ("Low order truncation of % digits.\n", warning);
        }
    }                           // note: if exponent = 0, no shift occurs

  // find out how many places before decimal point: 
  // start at new_term[0] and move left until you encounter 
  // the first non-zero digit or the decimal point. 
  // Minimum places__before = 1. 
  for (p = new_term; *p == '\0' && p < new_term + DEC_LOC; p++);        /* just loop to the first non-zero digit */

  t->places_before = (new_term + DEC_LOC) - p;

  // find out how many places after decimal point: 
  // if there was a decimal point, then start at right end 
  // of new_term and go left until you encounter the first 
  // non-zero digit or the decimal point. If there was no 
  // decimal point, then places_after = 0.  

  if (dec_pt == NULL)
    t->places_after = 0;
  else
    {
      for (p = new_term + 2 * MAX_SIZE;
           *p == '\0' && p >= new_term + DEC_LOC; p--)
        ;                       // just loop to the first non-zero digit

      t->places_after = (p - (new_term + DEC_LOC) + 1);
    }

  *buffer = first_char;

  return (notation);

}

/* 
* Converts a normalized term into an ASCII string 
• */
void
TermToAscii (struct TermData *t, char *ascii, int notation)
{
  char *first,                  // first printing digit
   *last,                       // last printing digit 
   *output;                     // where the ascii string is built

  first = t->term;
  output = ascii;
  // skip leading zeros
  while (*first == '\0' && first < t->term + 2 * MAX_SIZE)
    first++;

  // if at end, print answer of 0 and exit
  if (first == t->term + 2 * MAX_SIZE)
    {
      *output++ = '0';
      *output = '\0';
      return;
    }
  else                          // if past decimal point, start at dec. pt. 
    {
      if (first >= t->term + DEC_LOC)
        first = t->term + DEC_LOC;
    }

  // find last non-zero digit up to decimal point
  last = t->term + 2 * MAX_SIZE;
  while (last > first && last >= t->term + DEC_LOC && *last == '\0')
    last--;

  // before beginning printing, check the sign
  if (t->sign == -1)
    *output++ = '-';

  // if we start at the decimal point, print leading 0 
  if (first == t->term + DEC_LOC)
    *output++ = '0';

  while (first <= last)
    {
      if (first == t->term + DEC_LOC)
        *output++ = '.';
      *output++ = *first + '0';
      first += 1;
    }
  *output = '\0';

  if (notation == SCIENTIFIC)
    AsciiToScientific (ascii);
}

/* 
* Takes a string in normal notation and converts it to 
* scientific notation of the form: 
[-](0-9).(0-9)*e[+|-](0-9)* 
* The final string overlays the original string. This could be 
* dangerous since the new form is conceivably longer than the 
* original form. However, we know the input string is twice 
* MAX_SIZE, so there should always be enough room to fit. This 
* routine would have to modify this aspect if it were to be 
* used as a generic format conversion function. 
• 
*/
int
AsciiToScientific (char *ascii)
{
  char *buffer, *dec_pt,        // location of the decimal pt, if any 
   *first_digit;                // where the first non-zero digit is
  char str_exp[8];              // will hold exponent string
  int i, j, ascii_len, exponent;

  i = j = exponent = 0;
  ascii_len = strlen (ascii);
  buffer = malloc (ascii_len + 6);
  if (ascii_len < 1 || buffer == NULL)
    return (0);

  // process the sign
  if (!isdigit (*ascii))
    {
      if (*ascii == '-')
        buffer[i++] = ascii[j++];
      else if (*ascii != '+')
        {
          printf ("Invalid number for scientific format\n");
          return (0);
        }
    }

  // now process the digits. First check for decimal point.
  dec_pt = strchr (ascii, '.');

  // skip to the first non-zero digit
  while (ascii[j] == '.' || ascii[j] == '0')
    {
      j += 1;
      if (ascii[j] == '\0')     // end of string
        {
          printf ("Invalid number for scientific format\n");
          return (0);
        }
    }
  first_digit = &ascii[j];

  buffer[i++] = ascii[j++];
  buffer[i++] = '.';

  if (!ascii[j])                // e.g. 6 = 6.0el 
    buffer[i++] = '0';
  else
    while (ascii[j])
      if (ascii[j] == '.')
        j += 1;
      else
        buffer[i++] = ascii[j++];

  // do we have trailing zeros? Trim 1.600e4 to 1.6e4
  j = i - i;
  while (j > 1 && buffer[j - 1] != '.')
    {
      if (buffer[j--] == '0')
        i -= 1;
      else
        break;
    }

  /* compute the exponent */
  if (dec_pt)
    {
      if (first_digit > dec_pt) // e.g., 0.0065
        exponent = (first_digit - dec_pt) * -1;
      else                      // e.g., 650.2
        exponent = dec_pt - first_digit - 1;
    }
  else                          // no decimal point
    {
      exponent = ascii_len - 1;
      if (!isdigit (*ascii))    // was there a + or - ?
        exponent -= 1;
    }

  // output the exponent
  buffer[i++] = 'e';

  memset (str_exp, '\0', 8);
  sprintf (str_exp, "%d", exponent);
  strcpy (buffer + i, str_exp);

  // overlay the original string
  strcpy (ascii, buffer);
  free (buffer);

  return (1);
}

/* 
* Creates a TermData structure and initializes it to 
* a value of zero. Returns NULL on error. 
* */ 
struct TermData * TermCreate ( void ) 
{ 
    struct TermData *t; 
    t = malloc ( sizeof ( struct TermData )); 
    if ( t == NULL ) 
        return ( NULL ); 

    t->term = (char *) malloc ( 2*MAX_SIZE + 1 ); 
    TermInit ( t ); 

    return ( t ); 
} 

// 
// Initialize a term to zero. 
void TermInit (struct TermData *t)
{ 
    t->places_before = 0; 
    t->places_after = 0; 
    t->sign = 1; 
    memset ( t->term, '\0', 2 * MAX_SIZE + 1 ); 
}

// 
// Copy a term from the second argument to the first. 
// Returns a pointer to the copied string. 
// 
struct TermData * TermCopy ( struct TermData * dest, struct TermData * src ) 
{ 
    dest->sign = src->sign; 
    dest->places_before = src->places_before; 
    dest->places_after = src->places_after; 
    memcpy ( dest->term, src->term, 2*MAX_SIZE + 1 ); 
    return ( dest ); 
} 


