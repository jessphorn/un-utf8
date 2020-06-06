/**
  @file codepoints.h
  @author Jessica Horn (jphorn)

  Defines a CodePoint, loadTable, freeTable, and reportCode.
*/

#include <stdbool.h>

/** The maximum length of a code point name. */
#define MAX_NAME_LEN 90

/** Representation of a code point. */
typedef struct {
  /** The UTF-8 code */
  int code;
  /** The name of the character that corresponds to the code */
  char name[ MAX_NAME_LEN + 1 ];
} CodePoint;

/**
  Opens unicode.txt and reads code point descriptions into a table.
*/
void loadTable ();

/**
  Frees the memory used by the code point table.
*/
void freeTable();

/**
  Prints the name of a code to standard output.

  @param code The numeric value of the code point that will be printed.
  @return true if the code is in the code point table.
*/
bool reportCode( int );
