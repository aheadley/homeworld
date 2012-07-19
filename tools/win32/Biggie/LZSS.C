//
//  LZSS Compression Module
//
//  Original code from The Data Compression Book, by Mark Nelson & Jean-Loup Gailly
//
//  Modified by Darren Stone 1998/10/05 to allow IO to/from memory buffers as well
//  as files and bitfiles.
//

//
//  NOTE: You must initialize BitIO before using these routines.
//

/*
 * This is the LZSS module, which implements an LZ77 style compression
 * algorithm.  As iplemented here it uses a 12 bit index into the sliding
 * window, and a 4 bit length, which is adjusted to reflect phrase lengths
 * of between 2 and 17 bytes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "BitIO.h"
#include "LZSS.h"

/*
 * Various constants used to define the compression parameters.  The
 * INDEX_BIT_COUNT tells how many bits we allocate to indices into the
 * text window.  This directly determines the WINDOW_SIZE.  The
 * LENGTH_BIT_COUNT tells how many bits we allocate for the length of
 * an encode phrase. This determines the size of the look ahead buffer.
 * The TREE_ROOT is a special node in the tree that always points to
 * the root node of the binary phrase tree.  END_OF_STREAM is a special
 * index used to flag the fact that the file has been completely
 * encoded, and there is no more data.  UNUSED is the null index for
 * the tree. MOD_WINDOW() is a macro used to perform arithmetic on tree
 * indices.
 *
 */

#define INDEX_BIT_COUNT      12
#define LENGTH_BIT_COUNT     4
#define WINDOW_SIZE          ( 1 << INDEX_BIT_COUNT )
#define RAW_LOOK_AHEAD_SIZE  ( 1 << LENGTH_BIT_COUNT )
#define BREAK_EVEN           ( ( 1 + INDEX_BIT_COUNT + LENGTH_BIT_COUNT ) / 9 )
#define LOOK_AHEAD_SIZE      ( RAW_LOOK_AHEAD_SIZE + BREAK_EVEN )
#define TREE_ROOT            WINDOW_SIZE
#define END_OF_STREAM        0
#define UNUSED               0
#define MOD_WINDOW( a )      ( ( a ) & ( WINDOW_SIZE - 1 ) )

char *CompressionName = "LZSS Encoder";
char *Usage           = "in-file out-file\n\n";

/*
 * These are the two global data structures used in this program.
 * The window[] array is exactly that, the window of previously seen
 * text, as well as the current look ahead text.  The tree[] structure
 * contains the binary tree of all of the strings in the window sorted
 * in order.
*/

unsigned char window[ WINDOW_SIZE ];

struct tree_s {
    int parent;
    int smaller_child;
    int larger_child;
} tree[ WINDOW_SIZE + 1 ];

// local prototypes
void InitTree( int r );
void ContractNode( int old_node, int new_node );
void ReplaceNode( int old_node, int new_node );
int FindNextNode( int node );
void DeleteString( int p );
int AddString( int new_node, int *match_position );

/*
 * Since the tree is static data, it comes up with every node
 * initialized to 0, which is good, since 0 is the UNUSED code.
 * However, to make the tree really usable, a single phrase has to be
 * added to the tree so it has a root node.  That is done right here.
*/
static void InitTree( r )
int r;
{
	memset((void*)&tree, 0, sizeof(struct tree_s) * WINDOW_SIZE + 1);
    tree[ TREE_ROOT ].larger_child = r;
    tree[ r ].parent = TREE_ROOT;
    tree[ r ].larger_child = UNUSED;
    tree[ r ].smaller_child = UNUSED;
}

/*
 * This routine is used when a node is being deleted.  The link to
 * its descendant is broken by pulling the descendant in to overlay
 * the existing link.
 */
static void ContractNode( old_node, new_node )
int old_node;
int new_node;
{
    tree[ new_node ].parent = tree[ old_node ].parent;
    if ( tree[ tree[ old_node ].parent ].larger_child == old_node )
        tree[ tree[ old_node ].parent ].larger_child = new_node;
    else
        tree[ tree[ old_node ].parent ].smaller_child = new_node;
    tree[ old_node ].parent = UNUSED;
}

/*
 * This routine is also used when a node is being deleted.  However,
 * in this case, it is being replaced by a node that was not previously
 * in the tree.
 */
static void ReplaceNode( old_node, new_node )
int old_node;
int new_node;
{
    int parent;

    parent = tree[ old_node ].parent;
    if ( tree[ parent ].smaller_child == old_node )
        tree[ parent ].smaller_child = new_node;
    else
        tree[ parent ].larger_child = new_node;
    tree[ new_node ] = tree[ old_node ];
    tree[ tree[ new_node ].smaller_child ].parent = new_node;
    tree[ tree[ new_node ].larger_child ].parent = new_node;
    tree[ old_node ].parent = UNUSED;
}

/*
 * This routine is used to find the next smallest node after the node
 * argument.  It assumes that the node has a smaller child.  We find
 * the next smallest child by going to the smaller_child node, then
 * going to the end of the larger_child descendant chain.
*/
static int FindNextNode( node )
int node;
{
    int next;

    next = tree[ node ].smaller_child;
    while ( tree[ next ].larger_child != UNUSED )
        next = tree[ next ].larger_child;
    return( next );
}

/*
 * This routine performs the classic binary tree deletion algorithm.
 * If the node to be deleted has a null link in either direction, we
 * just pull the non-null link up one to replace the existing link.
 * If both links exist, we instead delete the next link in order, which
 * is guaranteed to have a null link, then replace the node to be deleted
 * with the next link.
 */
static void DeleteString( p )
int p;
{
    int  replacement;

    if ( tree[ p ].parent == UNUSED )
        return;
    if ( tree[ p ].larger_child == UNUSED )
        ContractNode( p, tree[ p ].smaller_child );
    else if ( tree[ p ].smaller_child == UNUSED )
        ContractNode( p, tree[ p ].larger_child );
    else {
        replacement = FindNextNode( p );
        DeleteString( replacement );
        ReplaceNode( p, replacement );
    }
}

/*
 * This where most of the work done by the encoder takes place.  This
 * routine is responsible for adding the new node to the binary tree.
 * It also has to find the best match among all the existing nodes in
 * the tree, and return that to the calling routine.  To make matters
 * even more complicated, if the new_node has a duplicate in the tree,
 * the old_node is deleted, for reasons of efficiency.
 */

static int AddString( new_node, match_position )
int new_node;
int *match_position;
{
    int i;
    int test_node;
    int delta;
    int match_length;
    int *child;

    if ( new_node == END_OF_STREAM )
        return( 0 );
    test_node = tree[ TREE_ROOT ].larger_child;
    match_length = 0;
    for ( ; ; ) {
        for ( i = 0 ; i < LOOK_AHEAD_SIZE ; i++ ) {
            delta = window[ MOD_WINDOW( new_node + i ) ] -
                    window[ MOD_WINDOW( test_node + i ) ];
            if ( delta != 0 )
                break;
        }
        if ( i >= match_length ) {
            match_length = i;
            *match_position = test_node;
            if ( match_length >= LOOK_AHEAD_SIZE ) {
                ReplaceNode( test_node, new_node );
                return( match_length );
            }
        }
        if ( delta >= 0 )
            child = &tree[ test_node ].larger_child;
        else
            child = &tree[ test_node ].smaller_child;
        if ( *child == UNUSED ) {
            *child = new_node;
            tree[ new_node ].parent = test_node;
            tree[ new_node ].larger_child = UNUSED;
            tree[ new_node ].smaller_child = UNUSED;
            return( match_length );
        }
        test_node = *child;
    }
}

//
// This is the compression routine.  It has to first load up the look
// ahead buffer, then go into the main compression loop.  The main loop
// decides whether to output a single character or an index/length
// token that defines a phrase.  Once the character or phrase has been
// sent out, another loop has to run.  The second loop reads in new
// characters, deletes the strings that are overwritten by the new
// character, then adds the strings that are created by the new
// character.
//
void lzssCompressFile(input, output)
FILE *input;
BIT_FILE *output;
{
    int i;
    int c;
    int look_ahead_bytes;
    int current_position;
    int replace_count;
    int match_length;
    int match_position;

    current_position = 1;
    for ( i = 0 ; i < LOOK_AHEAD_SIZE ; i++ ) {
        if ( ( c = getc( input ) ) == EOF )
            break;
        window[ current_position + i ] = (unsigned char) c;
    }
    look_ahead_bytes = i;
    InitTree( current_position );
    match_length = 0;
    match_position = 0;
    while ( look_ahead_bytes > 0 ) {
        if ( match_length > look_ahead_bytes )
            match_length = look_ahead_bytes;
        if ( match_length <= BREAK_EVEN ) {
            replace_count = 1;
            bitioFileOutputBit( output, 1 );
            bitioFileOutputBits( output,
                        (unsigned long) window[ current_position ], 8 );
        } else {
            bitioFileOutputBit( output, 0 );
            bitioFileOutputBits( output,
                        (unsigned long) match_position, INDEX_BIT_COUNT );
            bitioFileOutputBits( output,
                        (unsigned long) ( match_length - ( BREAK_EVEN + 1 ) ),
                        LENGTH_BIT_COUNT );
            replace_count = match_length;
        }
        for ( i = 0 ; i < replace_count ; i++ ) {
            DeleteString( MOD_WINDOW( current_position + LOOK_AHEAD_SIZE ) );
            if ( ( c = getc( input ) ) == EOF )
                look_ahead_bytes--;
            else
                window[ MOD_WINDOW( current_position + LOOK_AHEAD_SIZE ) ]
                        = (unsigned char) c;
            current_position = MOD_WINDOW( current_position + 1 );
            if ( look_ahead_bytes )
                match_length = AddString( current_position, &match_position );
        }
    };
    bitioFileOutputBit( output, 0 );
    bitioFileOutputBits( output, (unsigned long) END_OF_STREAM, INDEX_BIT_COUNT );
}

//
// This is the expansion routine for the LZSS algorithm.  All it has
// to do is read in flag bits, decide whether to read in a character or
// a index/length pair, and take the appropriate action.
//
void lzssExpandFile(input, output)
BIT_FILE *input;
FILE *output;
{
    int i;
    int current_position;
    int c;
    int match_length;
    int match_position;

    current_position = 1;
    for ( ; ; ) {
        if ( bitioFileInputBit( input ) ) {
            c = (int) bitioFileInputBits( input, 8 );
            putc( c, output );
            window[ current_position ] = (unsigned char) c;
            current_position = MOD_WINDOW( current_position + 1 );
        } else {
            match_position = (int) bitioFileInputBits( input, INDEX_BIT_COUNT );
            if ( match_position == END_OF_STREAM )
                break;
            match_length = (int) bitioFileInputBits( input, LENGTH_BIT_COUNT );
            match_length += BREAK_EVEN;
            for ( i = 0 ; i <= match_length ; i++ ) {
                c = window[ MOD_WINDOW( match_position + i ) ];
                putc( c, output );
                window[ current_position ] = (unsigned char) c;
                current_position = MOD_WINDOW( current_position + 1 );
            }
        }
    }
}

//
//  This is like the CompressFile routine, but I/O is to/from memory.
//
//  input
//      data buffer to compress
//  inputSize
//      buffer size
//  output
//      buffer to compress data into (already allocated)
//  outputSize
//      buffer size
//
//  returns
//      -1 if there was an error
//      size of compressed data in output if successful
//
int lzssCompressBuffer(char *input, int inputSize, char *output, int outputSize)
{
    int i;
    int c;
    int look_ahead_bytes;
    int current_position;
    int replace_count;
    int match_length;
    int match_position;
    BIT_BUFFER *outBuffer;
    char *inBuffer = input;

    outBuffer = bitioBufferOpen(output);

    current_position = 1;
    for ( i = 0 ; i < LOOK_AHEAD_SIZE ; i++ )
    {
        if (inBuffer >= input+inputSize)
            break;
        else
            c = *(inBuffer++);
        window[ current_position + i ] = (unsigned char) c;
    }
    look_ahead_bytes = i;
    InitTree( current_position );
    match_length = 0;
    match_position = 0;
    while ( look_ahead_bytes > 0 )
    {
        if ( match_length > look_ahead_bytes )
            match_length = look_ahead_bytes;
        if ( match_length <= BREAK_EVEN )
        {
            replace_count = 1;
            bitioBufferOutputBit(outBuffer, 1 );
            bitioBufferOutputBits(outBuffer, (unsigned long)window[current_position], 8);
        }
        else 
        {
            bitioBufferOutputBit(outBuffer, 0 );
            bitioBufferOutputBits(outBuffer, (unsigned long)match_position, INDEX_BIT_COUNT);
            bitioBufferOutputBits(outBuffer, (unsigned long)( match_length - (BREAK_EVEN + 1)), LENGTH_BIT_COUNT);
            replace_count = match_length;
        }
        for ( i = 0 ; i < replace_count ; i++ )
        {
            DeleteString( MOD_WINDOW( current_position + LOOK_AHEAD_SIZE ) );
            if (inBuffer >= input+inputSize)
                look_ahead_bytes--;
            else
            {
                c = *(inBuffer++);
                window[MOD_WINDOW(current_position + LOOK_AHEAD_SIZE)] = (unsigned char)c;
            }
            current_position = MOD_WINDOW( current_position + 1 );
            if ( look_ahead_bytes )
                match_length = AddString( current_position, &match_position );
        }
    };

    bitioBufferOutputBit(outBuffer, 0 );
    bitioBufferOutputBits(outBuffer, (unsigned long) END_OF_STREAM, INDEX_BIT_COUNT );
    return bitioBufferCloseOutput(outBuffer);
}

//
//  This is like the ExpandFile routine, but I/O is to/from memory.
//
//  input
//      compressed data buffer to read
//  inputSize
//      buffer size
//  output
//      buffer to expand data into
//
//  returns
//      -1 if there was an error
//      size of expanded data in output if successful
//
int lzssExpandBuffer(char *input, int inputSize, char *output, int outputSize)
{
    int i;
    int current_position;
    int c;
    int match_length;
    int match_position;
    BIT_BUFFER *inBuffer;
    char *outBuffer = output;
    int size = 0;
 
    inBuffer = bitioBufferOpen(input);

    current_position = 1;
    for ( ; ; ) {
        if (bitioBufferInputBit(inBuffer))
        {
            c = (int)bitioBufferInputBits(inBuffer, 8);
            *(outBuffer++) = c;
            window[current_position] = (unsigned char)c;
            current_position = MOD_WINDOW(current_position + 1);
        } else {
            match_position = (int)bitioBufferInputBits(inBuffer, INDEX_BIT_COUNT);
            if (match_position == END_OF_STREAM)
                break;
            match_length = (int)bitioBufferInputBits(inBuffer, LENGTH_BIT_COUNT);
            match_length += BREAK_EVEN;
            for ( i = 0 ; i <= match_length ; i++ )
            {
                c = window[MOD_WINDOW(match_position + i)];
                *(outBuffer++) = c;
                window[current_position] = (unsigned char)c;
                current_position = MOD_WINDOW(current_position + 1);
            }
        }
    }

    bitioBufferCloseInput(inBuffer);
	return (outBuffer - output);
}

//
//  Expands from a bit file to a buffer.
//
//  returns
//      -1 if there was an error
//      size of expanded data in output if successful
//
int lzssExpandFileToBuffer(BIT_FILE *input, char *output, int outputSize)
{
    int i;
    int current_position;
    int c;
    int match_length;
    int match_position;
    char *outBuffer = output;
    int size = 0;
 
    current_position = 1;
    for ( ; ; ) {
        if (bitioFileInputBit(input))
        {
            c = (int)bitioFileInputBits(input, 8);
            *(outBuffer++) = c;
            window[current_position] = (unsigned char)c;
            current_position = MOD_WINDOW(current_position + 1);
        } else {
            match_position = (int)bitioFileInputBits(input, INDEX_BIT_COUNT);
            if (match_position == END_OF_STREAM)
                break;
            match_length = (int)bitioFileInputBits(input, LENGTH_BIT_COUNT);
            match_length += BREAK_EVEN;
            for ( i = 0 ; i <= match_length ; i++ )
            {
                c = window[MOD_WINDOW(match_position + i)];
                *(outBuffer++) = c;
                window[current_position] = (unsigned char)c;
                current_position = MOD_WINDOW(current_position + 1);
            }
        }
    }

	return (outBuffer - output);
}

