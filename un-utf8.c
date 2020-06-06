/**
  @file un-utf8.c
  @author Jessica Horn (jphorn)

  Opens and reads from the input file, detects errors, and prints the names of valid codepoints.
*/

#include <stdio.h>
#include <stdlib.h>
#include "codepoints.h"

/** The number of command line arguments that should be used when starting the program. */
#define ARGS 2
/** A mask to check if a code is represented with 7 or less bits. */
#define SM_TEST_MASK 0x80
/** A mask to check if a code is represented with 8 to 11 bits. */
#define MED_TEST_MASK 0xe0
/** A mask to check if a code is represented with 12 to 16 bits. */
#define LG_TEST_MASK 0xf0
/** A mask to check if a code is represented with 17 to 21 bits. */
#define XL_TEST_MASK 0xf8
/** A moveable mask to test that a code is valid. */
#define MOVE_TEST_MASK 0x0000c0
/** The capacity of the buffer. */
#define CAP_BUFFER 1000
/** The number of bytes in a small code. */
#define SM_BYTES 1
/** The number of bytes in a medium code. */
#define MED_BYTES 2
/** The number of bytes in a large code. */
#define LG_BYTES 3
/** The number of bytes in a extra large code. */
#define XL_BYTES 4
/** Moveable mask used to select the low order bits. */
#define MOVE_MASK 0x00003F
/** The number of bits to shift the moveable mask. */
#define SHIFT 8
/** Mask used to select the low order bits of a small code. */
#define SM_LOW_MASK 0xFF
/** Mask used to select the low order bits of the first byte of a medium code. */
#define MED_LOW_MASK 0x1F00
/** Mask used to select the low order bits fo the first byte of a large code. */
#define LG_LOW_MASK 0x0F0000
/** Mask used to select the low order bits of the first byteof an extra large code. */
#define XL_LOW_MASK 0x07000000
/** Max value stored in a small code. */
#define MAX_SM 127
/** Max value stored in a medium code. */
#define MAX_MED 2047
/** Max value stored in a large code. */
#define MAX_LG 65535
/** Mask to clear all bytes except the lowest. */
#define CLEAR 0x000000FF
/** Expected result after testing a byte of a small code. */
#define EXP_SM 0
/** Expected result after testing the first byte of a medium code. */
#define EXP_MED 0xc0
/** Expected result after testing the first byte of a large code. */
#define EXP_LG 0xe0
/** Expected result affer testing the first byte of an extra large code. */
#define EXP_XL 0xf0
/** Expected result after testing the remaining bytes of a code with moveable mask. */
#define EXP_MOVE 0x80
/** The number of bits to shift a byte to the right.. */
#define SHIFT_R 2
/** The amount by which the buffer capacity increases */
#define DOUBLE 2

/**
  Checks the bytes at the current location in the buffer against the requirements of an utf8 code.
  If they meet the requirements, checks to see if the code is located in the table.

  @param b An array of bytes.
  @param location The location in the buffer from which to begin reading.
  @param tMask The mask to test the first byte against.
  @param exp The expected result after testing a byte against tMask.
  @param size The number of bytes to read in.
  @param lMask A mask used to clear the low bytes of a code.
  @param max The maximum number held by the bits in a smaller code.
  @return true If the bytes meet the requirements of an utf8 code.
*/
static bool isCode ( char *b, int location, int tMask, int exp, int size, int lMask, int max )
{
  int loc = location;
  char c = b[ location ];
  int code = CLEAR & ( int ) c;
  bool match = true;
  int low = 0;
  if ( ( code & tMask ) == exp ) {
    for ( int i = 1; i < size; i++ ) {
      location++;
      code <<= SHIFT;
      c = b[ location ];
      low = CLEAR & ( int ) c;
      if ( ( low & MOVE_TEST_MASK ) != EXP_MOVE ) {
        match = false;
        break;
      }
      code |= low;
    }
    if ( match ) {
      int newCode = 0;
      int count = 0;
      int byte = 0;
      for ( int i = 0; i < size - 1; i++ ) {
        byte = code & ( MOVE_MASK << ( SHIFT * i ) );
        newCode |= ( byte >> ( SHIFT_R * i ) );
        count ++;
      }
      byte = ( code & lMask ) >>  ( SHIFT_R * count );
      newCode |= byte;
      if ( newCode <= max ) {
        fprintf( stderr, "Invalid encoding: 0x%X at %d\n", newCode, loc );
      } else if ( !reportCode( newCode ) ) {
        fprintf( stderr, "Unknown code: 0x%X at %d\n", newCode, loc );
      }
    } else {
          fprintf( stderr, "Invalid byte: 0x%X at %d\n", low, location );
    }
    return true;
  }
  return false;
}

/**
  Reads from the input file into a buffer.
  Calls appropriate functions to see if the each byte in the buffer is part of a utf8 code.

  @param input A file stream.
*/
static void processInput( FILE *input )
{
  char *buffer = ( char * )malloc( CAP_BUFFER );
  size_t bytesRead = fread( buffer, sizeof( char ), CAP_BUFFER, input );
  int cap = CAP_BUFFER;
  while ( bytesRead == cap ) {
    cap *= DOUBLE;
    buffer = ( char * )realloc( buffer, cap );
    bytesRead += fread( buffer + cap / DOUBLE, sizeof( char ), cap / DOUBLE, input );
  }
  int location = 0;
  while ( location < bytesRead ) {
    if ( isCode( buffer, location, SM_TEST_MASK, EXP_SM, SM_BYTES, SM_LOW_MASK, 0 ) ) {
      location += SM_BYTES;
    } else if ( location + MED_BYTES - 1 < bytesRead ) {
      if ( isCode( buffer, location, MED_TEST_MASK, EXP_MED, MED_BYTES, MED_LOW_MASK, MAX_SM ) ) {
        location += MED_BYTES;
      } else if ( location + LG_BYTES - 1 < bytesRead ) {
        if ( isCode( buffer, location, LG_TEST_MASK, EXP_LG, LG_BYTES, LG_LOW_MASK, MAX_MED ) ) {
          location += LG_BYTES;
        } else if ( location + XL_BYTES - 1 < bytesRead ) {
          if ( isCode( buffer, location, XL_TEST_MASK, EXP_XL, XL_BYTES, XL_LOW_MASK, MAX_LG) ) {
            location += XL_BYTES;
          } else {
            char c = buffer[ location ];
            fprintf(stderr, "Invalid byte: 0x%hhX at %d\n", c, location );
            location += SM_BYTES;
          }
        } else {
          fprintf( stderr, "Incomplete code at %d\n", location );
          location += LG_BYTES;
        }
      } else {
        fprintf( stderr, "Incomplete code at %d\n", location );
        location += MED_BYTES;
      }
    } else {
      fprintf( stderr, "Incomplete code at %d\n", location );
      location += SM_BYTES;
    }
  }
  free( buffer );
}

/**
  Starts the program.
  Reads command line input to open an input file.
  Delegates the processing of input and creation of a table of utf8 codes to other functions.

  @param argc The number of arguments from the command line.
  @param argv An array of strings representing commands.
  @return exit status
*/
int main( int argc, char *argv[] )
{
  if ( argc != ARGS ) {
    fprintf( stderr, "usage: un-utf8 <input-file>\n" );
    return EXIT_FAILURE;
  }
  
  FILE *input = fopen( argv[ 1 ], "rb" );
  if ( input == NULL ) {
    fprintf( stderr, "Can't open file: %s\n", argv[ 1 ] );
    fprintf( stderr, "usage: un-utf8 <input-file>\n" );
    return EXIT_FAILURE;
  }
  
  loadTable();
  processInput( input );
  fclose( input );
  freeTable();
  return EXIT_SUCCESS;
}
