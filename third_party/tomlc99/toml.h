#ifndef VXUI_TOMLC99_TOML_H
#define VXUI_TOMLC99_TOML_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct toml_step_t
{
    bool has_delay;
    bool has_id;
    bool has_prop;
    bool has_target;
    int64_t delay;
    char* id;
    char* prop;
    double target;
    int unknown_field_count;
    char unknown_key[ 64 ];
} toml_step_t;

typedef struct toml_sequence_t
{
    char* name;
    toml_step_t* steps;
    int step_count;
} toml_sequence_t;

typedef struct toml_doc_t
{
    toml_sequence_t* sequences;
    int sequence_count;
} toml_doc_t;

toml_doc_t* toml_parse_file( FILE* fp, char* errbuf, int errbufsz );
void toml_free( toml_doc_t* doc );

#ifdef __cplusplus
}
#endif

#endif
