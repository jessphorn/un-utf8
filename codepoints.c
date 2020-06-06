/**
  @file codepoints.c
  @author Jessica Horn (jphorn)

  Reads and maintains a list of codepoints.
*/

#include <stdio.h>
#include <stdlib.h>
#include "codepoints.h"

/** The initial capacity for the table of code points. */
#define INIT_CAP 100
/** The amount by which to resize the table of or the search area used in binary search. */
#define RESIZE 2
/** The number of characters to allow for when reading in a code. */
#define CODE_LENGTH 10

/** A table of CodePoints. */
static CodePoint **table;
/** The number of CodePoints stored in the table. */
static int len;

/**
  Compares two CodePoints according to their codes.

  @param p A CodePoint.
  @param q A CodePoint.
  @return Negative if p is less than q, zero if p equals q, and positive if p is greater than q.
*/
static int compare_codes( const void *p, const void *q ) {
  return ( *( CodePoint ** )p )->code - ( *( CodePoint ** )q )->code;
}

void loadTable ()
{
  FILE *records = fopen( "unicode.txt", "r" );
  if ( records == NULL ) {
    fprintf( stderr, "Can't open records file: unicode.txt" );
    exit( EXIT_FAILURE );
  }

  table = ( CodePoint ** )malloc( sizeof( CodePoint * ) *  INIT_CAP );
  int cap = INIT_CAP;
  len = 0;
  char line[ CODE_LENGTH + MAX_NAME_LEN + 1 ];
  while ( ( fgets( line, CODE_LENGTH + MAX_NAME_LEN + 1, records ) ) != NULL ) {
    if ( len >= cap ) {
      cap *= RESIZE;
      table = ( CodePoint ** )realloc( table, sizeof( CodePoint * ) * cap );
    }

    CodePoint *cp = ( CodePoint * )malloc( sizeof( CodePoint ) );
    sscanf( line, "%x\t%[^\n]s", &( cp->code ), cp->name );
    
    table[ len++ ] = cp;

  }
  fclose( records );
  qsort( table, len, sizeof( CodePoint * ), compare_codes );
}

void freeTable()
{
  for ( int i = 0; i < len; i++ ) {
    if ( table[ i ] != NULL ) {
      free( table[ i ] );
    }
  }
  free( table );
}

bool reportCode( int code )
{
  int start = 0;
  int end = len - 1;
  while ( start <= end ) {
    int mid = ( start + end ) / RESIZE;
    if ( code > table[ mid ]->code ) {
      start = mid + 1;
    } else {
        end = mid - 1;
    }
  }
  if ( code == table[ start ]->code ) {
    printf( "%s\n", table[ start ]->name );
    return true;
  }
  return false;
}
