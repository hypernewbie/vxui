#include "toml.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef struct toml_parser_t
{
    const char* src;
    size_t at;
    size_t length;
    int line;
    char* errbuf;
    int errbufsz;
} toml_parser_t;

static void toml__set_error( toml_parser_t* parser, const char* message )
{
    if ( !parser || !parser->errbuf || parser->errbufsz <= 0 ) {
        return;
    }

    snprintf( parser->errbuf, ( size_t ) parser->errbufsz, "line %d: %s", parser->line, message ? message : "parse error" );
    parser->errbuf[ parser->errbufsz - 1 ] = '\0';
}

static char* toml__strdup_range( const char* src, size_t length )
{
    char* copy = ( char* ) malloc( length + 1u );
    if ( !copy ) {
        return NULL;
    }
    memcpy( copy, src, length );
    copy[ length ] = '\0';
    return copy;
}

static char toml__peek( toml_parser_t* parser )
{
    if ( !parser || parser->at >= parser->length ) {
        return '\0';
    }
    return parser->src[ parser->at ];
}

static char toml__advance( toml_parser_t* parser )
{
    char c = toml__peek( parser );
    if ( c != '\0' ) {
        parser->at += 1u;
        if ( c == '\n' ) {
            parser->line += 1;
        }
    }
    return c;
}

static void toml__skip_ws_and_comments( toml_parser_t* parser )
{
    for ( ;; ) {
        char c = toml__peek( parser );
        while ( c == ' ' || c == '\t' || c == '\r' || c == '\n' ) {
            toml__advance( parser );
            c = toml__peek( parser );
        }

        if ( c != '#' ) {
            break;
        }

        while ( c != '\0' && c != '\n' ) {
            c = toml__advance( parser );
        }
    }
}

static int toml__expect( toml_parser_t* parser, char expected )
{
    if ( toml__peek( parser ) != expected ) {
        char message[ 64 ] = {};
        snprintf( message, sizeof( message ), "expected '%c'", expected );
        toml__set_error( parser, message );
        return 0;
    }
    toml__advance( parser );
    return 1;
}

static char* toml__parse_identifier( toml_parser_t* parser )
{
    size_t start = parser->at;
    char c = toml__peek( parser );
    while ( isalnum( ( unsigned char ) c ) || c == '_' || c == '-' ) {
        toml__advance( parser );
        c = toml__peek( parser );
    }

    if ( parser->at == start ) {
        toml__set_error( parser, "expected identifier" );
        return NULL;
    }

    return toml__strdup_range( parser->src + start, parser->at - start );
}

static char* toml__parse_string( toml_parser_t* parser )
{
    size_t start;
    size_t length = 0u;
    char* out;

    if ( !toml__expect( parser, '"' ) ) {
        return NULL;
    }

    start = parser->at;
    while ( toml__peek( parser ) != '\0' && toml__peek( parser ) != '"' ) {
        if ( toml__peek( parser ) == '\\' ) {
            parser->at += 1u;
            if ( parser->at >= parser->length ) {
                toml__set_error( parser, "unterminated string" );
                return NULL;
            }
        }
        parser->at += 1u;
    }

    if ( toml__peek( parser ) != '"' ) {
        toml__set_error( parser, "unterminated string" );
        return NULL;
    }

    length = parser->at - start;
    out = ( char* ) malloc( length + 1u );
    if ( !out ) {
        toml__set_error( parser, "out of memory" );
        return NULL;
    }

    {
        size_t src = start;
        size_t dst = 0u;
        while ( src < parser->at ) {
            char c = parser->src[ src++ ];
            if ( c == '\\' && src < parser->at ) {
                char escaped = parser->src[ src++ ];
                if ( escaped == 'n' ) {
                    out[ dst++ ] = '\n';
                } else if ( escaped == 't' ) {
                    out[ dst++ ] = '\t';
                } else {
                    out[ dst++ ] = escaped;
                }
            } else {
                out[ dst++ ] = c;
            }
        }
        out[ dst ] = '\0';
    }

    toml__advance( parser );
    return out;
}

static int toml__parse_int64( toml_parser_t* parser, int64_t* out )
{
    char* end = NULL;
    long long value;
    errno = 0;
    value = strtoll( parser->src + parser->at, &end, 10 );
    if ( end == parser->src + parser->at || errno != 0 ) {
        toml__set_error( parser, "expected integer" );
        return 0;
    }
    parser->at = ( size_t ) ( end - parser->src );
    *out = ( int64_t ) value;
    return 1;
}

static int toml__parse_double( toml_parser_t* parser, double* out )
{
    char* end = NULL;
    errno = 0;
    *out = strtod( parser->src + parser->at, &end );
    if ( end == parser->src + parser->at || errno != 0 ) {
        toml__set_error( parser, "expected number" );
        return 0;
    }
    parser->at = ( size_t ) ( end - parser->src );
    return 1;
}

static int toml__skip_value( toml_parser_t* parser )
{
    char c = toml__peek( parser );
    if ( c == '"' ) {
        char* text = toml__parse_string( parser );
        if ( !text ) {
            return 0;
        }
        free( text );
        return 1;
    }

    if ( c == '{' ) {
        int depth = 0;
        do {
            c = toml__advance( parser );
            if ( c == '{' ) {
                depth += 1;
            } else if ( c == '}' ) {
                depth -= 1;
            }
        } while ( c != '\0' && depth > 0 );
        return depth == 0;
    }

    if ( c == '[' ) {
        int depth = 0;
        do {
            c = toml__advance( parser );
            if ( c == '[' ) {
                depth += 1;
            } else if ( c == ']' ) {
                depth -= 1;
            }
        } while ( c != '\0' && depth > 0 );
        return depth == 0;
    }

    while ( c != '\0' && c != ',' && c != '\n' && c != '}' ) {
        toml__advance( parser );
        c = toml__peek( parser );
    }
    return 1;
}

static int toml__append_sequence( toml_doc_t* doc, char* name )
{
    toml_sequence_t* sequences = ( toml_sequence_t* ) realloc( doc->sequences, ( size_t ) ( doc->sequence_count + 1 ) * sizeof( toml_sequence_t ) );
    if ( !sequences ) {
        free( name );
        return 0;
    }

    doc->sequences = sequences;
    doc->sequences[ doc->sequence_count ].name = name;
    doc->sequences[ doc->sequence_count ].steps = NULL;
    doc->sequences[ doc->sequence_count ].step_count = 0;
    doc->sequence_count += 1;
    return 1;
}

static int toml__append_step( toml_sequence_t* sequence, toml_step_t step )
{
    toml_step_t* steps = ( toml_step_t* ) realloc( sequence->steps, ( size_t ) ( sequence->step_count + 1 ) * sizeof( toml_step_t ) );
    if ( !steps ) {
        return 0;
    }
    sequence->steps = steps;
    sequence->steps[ sequence->step_count++ ] = step;
    return 1;
}

static int toml__parse_inline_step( toml_parser_t* parser, toml_step_t* out_step )
{
    toml_step_t step;
    memset( &step, 0, sizeof( step ) );

    if ( !toml__expect( parser, '{' ) ) {
        return 0;
    }

    for ( ;; ) {
        char* key;
        toml__skip_ws_and_comments( parser );
        if ( toml__peek( parser ) == '}' ) {
            toml__advance( parser );
            break;
        }

        key = toml__parse_identifier( parser );
        if ( !key ) {
            return 0;
        }

        toml__skip_ws_and_comments( parser );
        if ( !toml__expect( parser, '=' ) ) {
            free( key );
            return 0;
        }
        toml__skip_ws_and_comments( parser );

        if ( strcmp( key, "delay" ) == 0 ) {
            step.has_delay = toml__parse_int64( parser, &step.delay );
            if ( !step.has_delay ) {
                free( key );
                return 0;
            }
        } else if ( strcmp( key, "id" ) == 0 ) {
            step.id = toml__parse_string( parser );
            step.has_id = step.id != NULL;
            if ( !step.has_id ) {
                free( key );
                return 0;
            }
        } else if ( strcmp( key, "prop" ) == 0 ) {
            step.prop = toml__parse_string( parser );
            step.has_prop = step.prop != NULL;
            if ( !step.has_prop ) {
                free( key );
                return 0;
            }
        } else if ( strcmp( key, "target" ) == 0 ) {
            step.has_target = toml__parse_double( parser, &step.target );
            if ( !step.has_target ) {
                free( key );
                return 0;
            }
        } else {
            step.unknown_field_count += 1;
            snprintf( step.unknown_key, sizeof( step.unknown_key ), "%s", key );
            if ( !toml__skip_value( parser ) ) {
                free( key );
                return 0;
            }
        }

        free( key );
        toml__skip_ws_and_comments( parser );
        if ( toml__peek( parser ) == ',' ) {
            toml__advance( parser );
            continue;
        }
        if ( toml__peek( parser ) == '}' ) {
            toml__advance( parser );
            break;
        }

        toml__set_error( parser, "expected ',' or '}'" );
        return 0;
    }

    *out_step = step;
    return 1;
}

static int toml__parse_steps_array( toml_parser_t* parser, toml_sequence_t* sequence )
{
    if ( !toml__expect( parser, '[' ) ) {
        return 0;
    }

    for ( ;; ) {
        toml_step_t step;
        toml__skip_ws_and_comments( parser );
        if ( toml__peek( parser ) == ']' ) {
            toml__advance( parser );
            break;
        }

        if ( !toml__parse_inline_step( parser, &step ) ) {
            return 0;
        }
        if ( !toml__append_step( sequence, step ) ) {
            toml__set_error( parser, "out of memory" );
            return 0;
        }

        toml__skip_ws_and_comments( parser );
        if ( toml__peek( parser ) == ',' ) {
            toml__advance( parser );
            continue;
        }
        if ( toml__peek( parser ) == ']' ) {
            toml__advance( parser );
            break;
        }

        toml__set_error( parser, "expected ',' or ']'" );
        return 0;
    }

    return 1;
}

toml_doc_t* toml_parse_file( FILE* fp, char* errbuf, int errbufsz )
{
    toml_parser_t parser;
    toml_doc_t* doc;
    toml_sequence_t* current_sequence = NULL;
    long size;
    size_t read_size;
    char* buffer;

    if ( errbuf && errbufsz > 0 ) {
        errbuf[ 0 ] = '\0';
    }
    if ( !fp ) {
        if ( errbuf && errbufsz > 0 ) {
            snprintf( errbuf, ( size_t ) errbufsz, "missing file handle" );
            errbuf[ errbufsz - 1 ] = '\0';
        }
        return NULL;
    }

    if ( fseek( fp, 0, SEEK_END ) != 0 ) {
        return NULL;
    }
    size = ftell( fp );
    if ( size < 0 ) {
        return NULL;
    }
    rewind( fp );

    buffer = ( char* ) malloc( ( size_t ) size + 1u );
    if ( !buffer ) {
        if ( errbuf && errbufsz > 0 ) {
            snprintf( errbuf, ( size_t ) errbufsz, "out of memory" );
            errbuf[ errbufsz - 1 ] = '\0';
        }
        return NULL;
    }

    read_size = fread( buffer, 1u, ( size_t ) size, fp );
    buffer[ read_size ] = '\0';

    memset( &parser, 0, sizeof( parser ) );
    parser.src = buffer;
    parser.length = read_size;
    parser.line = 1;
    parser.errbuf = errbuf;
    parser.errbufsz = errbufsz;

    doc = ( toml_doc_t* ) calloc( 1u, sizeof( toml_doc_t ) );
    if ( !doc ) {
        free( buffer );
        return NULL;
    }

    while ( parser.at < parser.length ) {
        char* table_name;
        toml__skip_ws_and_comments( &parser );
        if ( parser.at >= parser.length ) {
            break;
        }

        if ( toml__peek( &parser ) == '[' ) {
            char* root_name;
            if ( !toml__expect( &parser, '[' ) ) {
                toml_free( doc );
                free( buffer );
                return NULL;
            }
            root_name = toml__parse_identifier( &parser );
            if ( !root_name ) {
                toml_free( doc );
                free( buffer );
                return NULL;
            }
            if ( strcmp( root_name, "sequence" ) != 0 ) {
                free( root_name );
                toml__set_error( &parser, "expected [sequence.name]" );
                toml_free( doc );
                free( buffer );
                return NULL;
            }
            free( root_name );
            if ( !toml__expect( &parser, '.' ) ) {
                toml_free( doc );
                free( buffer );
                return NULL;
            }
            table_name = toml__parse_identifier( &parser );
            if ( !table_name ) {
                toml_free( doc );
                free( buffer );
                return NULL;
            }
            if ( !toml__expect( &parser, ']' ) ) {
                free( table_name );
                toml_free( doc );
                free( buffer );
                return NULL;
            }
            if ( !toml__append_sequence( doc, table_name ) ) {
                toml__set_error( &parser, "out of memory" );
                toml_free( doc );
                free( buffer );
                return NULL;
            }
            current_sequence = &doc->sequences[ doc->sequence_count - 1 ];
            continue;
        }

        if ( !current_sequence ) {
            toml__set_error( &parser, "key/value outside sequence table" );
            toml_free( doc );
            free( buffer );
            return NULL;
        }

        {
            char* key = toml__parse_identifier( &parser );
            if ( !key ) {
                toml_free( doc );
                free( buffer );
                return NULL;
            }
            toml__skip_ws_and_comments( &parser );
            if ( !toml__expect( &parser, '=' ) ) {
                free( key );
                toml_free( doc );
                free( buffer );
                return NULL;
            }
            toml__skip_ws_and_comments( &parser );

            if ( strcmp( key, "steps" ) == 0 ) {
                if ( !toml__parse_steps_array( &parser, current_sequence ) ) {
                    free( key );
                    toml_free( doc );
                    free( buffer );
                    return NULL;
                }
            } else {
                if ( !toml__skip_value( &parser ) ) {
                    free( key );
                    toml_free( doc );
                    free( buffer );
                    return NULL;
                }
            }

            free( key );
        }
    }

    free( buffer );
    return doc;
}

void toml_free( toml_doc_t* doc )
{
    int i;
    if ( !doc ) {
        return;
    }

    for ( i = 0; i < doc->sequence_count; ++i ) {
        int step_index;
        free( doc->sequences[ i ].name );
        for ( step_index = 0; step_index < doc->sequences[ i ].step_count; ++step_index ) {
            free( doc->sequences[ i ].steps[ step_index ].id );
            free( doc->sequences[ i ].steps[ step_index ].prop );
        }
        free( doc->sequences[ i ].steps );
    }

    free( doc->sequences );
    free( doc );
}
