//
// Created by Joschka Goes on 11/2/2018.
//


/*
    Schema of input files and reserved names

    Input files are in csv format and contain following columns
    >> State [str], Read [char], Write [char], Move {R,L,-}, Next State [str]
    The first row contains names for each column used for printing steps.
    The first state defined is the initial state.
    The final state should be called 'Finish' signaling the program has finished.
    !Right now states should be defined in one block.

    Calling the program:
    >> ./VTM [file] [tape as string] [cycles] [left fill char] [right fill char] [tape offset]

    Cycles is the number of cycles the program is maximally running, if a 0 is
    entered the number of cycles is not limited. Can be omitted and default is
    1000.

    The fill chars are the chars which will be appended to the sides of the tape
    if the program exceeded the specified tape. They can be omitted and default
    to the '#' symbol.

    The tape offset has to be positive.

    Optionally any combination of these flags can be added before [file]
        -f      expects a path to a plain text file as tape instead of the string
        -a      auto stepping, runs the program without halting
        -q      quiet, only print the resulting tape (implies -a)
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "utils/darray.h"

//constants for modes
const unsigned char TAPE_FROM_FILE = 0x01;
const unsigned char AUTO_STEPPING = 0x02;
const unsigned char QUIET_MODE = 0x04;

typedef struct Tape {
    Darray *left;
    Darray *right;
} Tape;

typedef struct Transition {
    char read;
    char write;
    signed char move;
    size_t next;
} Transition;

char left_fill = '#';
char right_fill = '#';
char *initialTape = NULL;
Tape tape;

char *columnNames[5];
Darray *stateNames = NULL;
long *stateMap = NULL;
Transition *transitionMap = NULL;

long readWriteHead = 0;
size_t state = 0;
size_t finalState = SIZE_MAX;

char *getTapeAt(long i) {
    Darray *tape_side;
    char *fill;
    if (i >= 0) {
        tape_side = tape.right;
        fill = &right_fill;
    } else {
        tape_side = tape.left;
        fill = &left_fill;
        i = -i - 1;
    }
    if (i < tape_side->size) return (char *) Darray_get(tape_side, (size_t) i);
    else {
        Darray_set(tape_side, (size_t) i, fill);
        return fill;
    }
}

void setTapeAt(long i, char *c) {
    if (i < 0) {
        Darray_set(tape.left, (size_t) -i - 1, (void *) c);
    } else Darray_set(tape.right, (size_t) i, (void *) c);
}

int loadTape(const char *mode, char *tape) {
    size_t i = 0;
    if (*mode & TAPE_FROM_FILE) {
        //load from file
        LOG_ERR("LOADING TAPE FROM FILE NOT IMPLEMENTED");
        return -1;
    } else {
        for (i = 0; i < strlen(tape); ++i) {
            setTapeAt((long) i, &tape[i]);
        }
        return 0;
    }
}

void printTape() {
    printf("TAPE: ");
    long i;
    for (i = tape.left->size - 1; i >= 0; --i) {
        printf("%c", *(char *) tape.left->data[i]);
    }
    printf(" ");
    for (i = 0; i < tape.right->size; ++i) {
        printf("%c", *(char *) tape.right->data[i]);
    }
    printf("\n");
}

int step(const char *mode) {
    char *read = getTapeAt(readWriteHead);
    CHECK(read != NULL, "Out of memory!");

    long i = 0;
    for (i = stateMap[state]; i < stateMap[state + 1]; ++i) {
        if (transitionMap[i].read == *read) {
            if (!(*mode & QUIET_MODE)) {
                printf("%s: %s; ", columnNames[0], (char *) stateNames->data[state]);
                printf("%s: %c; ", columnNames[1], *read);
                printf("%s: %c; ", columnNames[2], transitionMap[i].write);
                printf("%s: ", columnNames[3]);
                switch (transitionMap[i].move) {
                    case -1:
                        printf("L; ");
                        break;
                    case 0:
                        printf("-; ");
                        break;
                    case 1:
                        printf("R; ");
                        break;
                }
                printf("%s: %s;\n", columnNames[4], (char *) stateNames->data[transitionMap[i].next]);
            }

            setTapeAt(readWriteHead, &transitionMap[i].write);
            readWriteHead += transitionMap[i].move;
            state = transitionMap[i].next;

            return 0;
        }
    }

    LOG_ERR("Unhanded input! (state %s %c)", (char *) stateNames->data[state], *read);
    error:
    return -1;
}

char *getTrimmedString(FILE *file, const unsigned char *criteria) {
    int c = EOF;
    long pos = 0;
    do {
        pos = ftell(file);
        c = fgetc(file);
    } while (isspace(c) && c != '\n' && c != EOF);
    CHECK(pos >= 0, "ftell failed.");
    int i = 0;
    size_t j = 0;
    size_t criteria_len = strlen(criteria);
    loop:
    ++i;
    if (c == EOF) goto exit;
    for (j = 0; j < criteria_len; ++j) {
        if (criteria[j] == c) goto exit;
    }
    c = fgetc(file);
    goto loop;
    exit:
    CHECK(fseek(file, pos, SEEK_SET) == 0, "Failed to rewind file.");
    char *out = (char *) malloc(i * sizeof(char));
    out = fgets(out, i, file);
    if (out == NULL) goto error;
    for (i -= 1; i >= 0; --i) {
        if (isspace(out[i])) out[i] = '\0';
        else break;
    }
    return out;
    error:
    return NULL;
}

int buildTransitionMap(char *file) {
    char *rv = NULL;
    FILE *program = fopen(file, "rb");
    CHECK(program != NULL, "Unable to open Program: %s", file);

    int i = 0;
    int c = EOF;

    for (i = 0; i < 5; ++i) {
        columnNames[i] = getTrimmedString(program, ",\n");
        CHECK(columnNames[i] != NULL, "Error reading column names.");
        c = fgetc(program);
        if (c == '\n') CHECK(i == 4, "Too few column names.");
    }

    while (c != '\n') {
        c = fgetc(program);
        CHECK(iscntrl(c), "To many columns.");
        CHECK(c == EOF, "No states defined in program.");
    }

    long data_row_start = ftell(program);
    stateNames = Darray_create(sizeof(char *), 1000);

    long line_nr = 0;
    //map state name
    do {
        ++line_nr;
        char *stateName = getTrimmedString(program, ",\n");
        CHECK(stateName != NULL, "Error reading line %ld", line_nr);

        long k = 0;
        for (k = 0; k < stateNames->size; ++k) {
            if (strcmp(stateName, (char *) Darray_get(stateNames, (size_t) k)) == 0)
            {
                free(stateName);
                goto skip;
            }
        }
        if (strcmp(stateName, "Final") == 0) finalState = stateNames->size;
        Darray_push(stateNames, stateName);
        skip:
        do { c = fgetc(program); } while (c != '\n' && c != EOF);

    } while (c != EOF);

    CHECK(finalState != SIZE_MAX, "No final state defined!");
    CHECK(fseek(program, data_row_start, SEEK_SET) == 0, "Failed to rewind program file");

    //assuming the states are ordered for now.
    transitionMap = (Transition *) calloc(sizeof(Transition), (size_t) line_nr);
    stateMap = (long *) malloc(sizeof(size_t) * (stateNames->size + 1));
    size_t last_state = 0;
    stateMap[0] = 0;
    stateMap[stateNames->size] = line_nr;

    long k = 0;
    for (k = 0; k < line_nr; ++k) {
        rv = getTrimmedString(program, ",\n");
        CHECK(rv != NULL, "Error reading line %ld", k + 2);
        if (strcmp(rv, (char *) Darray_get(stateNames, last_state)) != 0) {
            stateMap[++last_state] = k;
        }
        free(rv);
        c = fgetc(program);

        rv = getTrimmedString(program, ",\n");
        CHECK(rv != NULL && strlen(rv) == 1, "Expected char for %s in line %ld", columnNames[1], k + 2);
        transitionMap[k].read = rv[0];
        free(rv);
        c = fgetc(program);

        rv = getTrimmedString(program, ",\n");
        CHECK(rv != NULL && strlen(rv) == 1, "Expected char for %s in line %ld", columnNames[2], k + 2);
        transitionMap[k].write = rv[0];
        free(rv);
        c = fgetc(program);

        rv = getTrimmedString(program, ",\n");
        CHECK(rv != NULL && strlen(rv) == 1, "Expected char for %s in line %ld", columnNames[1], k + 2);
        transitionMap[k].move = rv[0];
        switch (rv[0]) {
            case 'R':
            case 'r':
                transitionMap[k].move = 1;
                break;
            case 'L':
            case 'l':
                transitionMap[k].move = -1;
                break;
            case '-':
                transitionMap[k].move = 0;
                break;
            case EOF:
                goto error;
            default:
                LOG_ERR("Unexpected char '%c' in line %ld for \"%s\"", rv[0], k + 2, columnNames[3]);
                goto error;
        }
        free(rv);
        c = fgetc(program);

        rv = getTrimmedString(program, ",\n");
        CHECK(rv != NULL, "Error reading line %ld", k);
        size_t l = 0;
        for (l = 0; l < stateNames->size; ++l) {
            if (strcmp(rv, Darray_get(stateNames, l)) == 0) {
                transitionMap[k].next = l;
                goto exit;
            }
        }
        LOG_ERR("Unresolved reference to %s %s on line %ld.", columnNames[0], rv, k);
        goto error;
        exit:
        free(rv);
        c = fgetc(program);
    }

    return 0;
    error:
    if (rv != NULL) free(rv);
    return -1;
}

void dummy() {}

int main(int argc, char *argv[]) {
    long cycles = 1000;

    tape.left = Darray_create(sizeof(char), 100);
    tape.right = Darray_create(sizeof(char), 100);

    CHECK_MEM(tape.left);
    CHECK_MEM(tape.right);

    //mode 0b'0000'0000 from left to right: -f, -a, -q
    unsigned char mode = 0x00;

    if (argc < 3) {
        LOG_ERR("Missing arguments: Need at least program file and tape!");
        goto error;
    }

    //parse flags
    size_t i = 1;
    for (i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            size_t j = 1;
            for (j = 1; j < strlen(argv[i]); ++j) {
                switch (argv[i][j]) {
                    case 'f':
                        mode |= TAPE_FROM_FILE;
                        break;
                    case 'q':
                        mode |= QUIET_MODE;
                        //no break is intentional
                    case 'a':
                        mode |= AUTO_STEPPING;
                        break;
                    default:
                        LOG_ERR("Unknown flag: %c", argv[i][j]);
                        goto error;
                }
            }
        }
    }


    //parse arguments
    size_t arg_nr = 0;
    for (i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            switch (++arg_nr) {
                case 1:
                    if (buildTransitionMap(argv[i]) != 0) goto error;
                    break;
                case 2:
                    if (loadTape(&mode, argv[i]) != 0) goto error;
                    break;
                case 3:
                    errno = 0;
                    cycles = strtol(argv[i], NULL, 0);
                    if (errno != 0) {
                        LOG_ERR("Error parsing arguments. Is the third argument the cycle number?");
                        goto error;
                    }
                    break;
                case 4:
                    left_fill = argv[i][0];
                    break;
                case 5:
                    right_fill = argv[i][0];
                    break;
                case 6:
                    errno = 0;
                    readWriteHead = strtol(argv[i], NULL, 0);
                    if (errno != 0) {
                        LOG_ERR("Error parsing arguments. Is the 6th argument really the readwrite head offset?");
                        goto error;
                    }
                    break;
                default:
                    LOG_ERR("Too many arguments!");
                    goto error;
            }
        }
    }

    void (*printPtr)() = (!(mode & QUIET_MODE)) ? printTape : dummy;

    //execution
    if (cycles != 0) {
        for (; cycles > 0 && state != finalState; --cycles) {
            if (step(&mode) != 0) goto error;
            printPtr();
        }
    } else {
        while (state != finalState) {
            if (step(&mode) != 0) goto error;
            printPtr();
        }
    }

    if (mode & QUIET_MODE) printTape();

    //sets return value
    int rv = 0;
    if (1 == 2)
        error:
        rv = -1;

    //cleanup
    if (tape.left != NULL) Darray_destroy(tape.left);
    if (tape.right != NULL) Darray_destroy(tape.right);
    if (stateNames != NULL) Darray_clear_destroy(stateNames);
    if (initialTape != NULL) free(initialTape);
    if (transitionMap != NULL) free(transitionMap);
    if (stateMap != NULL) free(stateMap);
    for (i = 0; i < 5; ++i) {
        free(columnNames[i]);
    }

    return rv;
}
