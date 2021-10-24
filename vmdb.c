/**
 * \file vmdb.c
 * \author Paweł Sobótka <p.sobotka\@mini.pw.edu.pl>
 * This is an implementation of OPS1 2020Z L1 task done to fulfill students' request.
 * Compile with gcc vmdb.c -Wall -Wextra -pedantic -Werror -o vmdb
 * \remark Source code is extensively documented what is not required during the class.
 * This is just to highlight some issues for you (students).
 * \remark This implementation may contain hidden bugs - it's good for you if you can find one :)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * \brief Maximum length of a single line in *.vdb file.
 * This constant includes a newline character so a line may contain at most 63 meaningful characters.
 * The task did not specify it strictly - you may want to have 64 meaningful characters + a newline in your programs.
 */
#define MAX_LINE_LENGTH 64
/**
 * \brief Maximum length of vote option name.
 * This constant does not include terminating NULL byte.
 */
#define MAX_OPTION_NAME_LENGTH 32

/**
 * \brief Print program usage and exit.
 */
void show_usage() {
    printf("./vmdb create [-l] database.vdb \"OptionA\" \"OptionB\" [...]\n");
    printf("./vmdb show database.vmdb\n");
    printf("./vmdb vote database.vmdb [-r] \"OptionX\"\n");
    exit(EXIT_FAILURE);
}

/**
 * \brief Print DB corrupted message and exit.
 * \param msg Additional message suffix.
 */
void db_corrupted(const char *msg) {
    fprintf(stderr, "Database is corrupted: %s\n", msg);
    exit(EXIT_FAILURE);
}

/**
 * \brief Formats option name and vote count into database line.
 * Function always creates a C-string of length 63 and stores terminating NULL byte at the end of the output buffer.
 * \param line Buffer where created line will be stored. Function closes the buffer with NULL byte.
 * \param opt Option name as a C string.
 * \param val Option vote count.
 */
void make_line(char line[MAX_LINE_LENGTH], const char opt[MAX_OPTION_NAME_LENGTH + 1], int val) {
    // Note the left padding with '-' and field widths in formatting string.
    // The formatted vote count will never be longer than 10 characters (see int32 max value).
    // snprintf() will always close the buffer with '\0'.
    snprintf(line, MAX_LINE_LENGTH, "%-32s%-31d", opt, val);
}

/**
 * \brief Create lock line in provided buffer.
 * Populates the buffer with 'X' characters closing it with NULL byte at the end.
 * \param line Buffer where lock line shall be stored.
 */
void make_lock_line(char line[MAX_LINE_LENGTH]) {
    // One could use memset() here
    for (int i = 0; i < 63; ++i) {
        line[i] = 'X';
    }
    line[63] = '\0';
}

/**
 * \brief Check if line is a lock line.
 * Function checks if all line characters are 'X'.
 * This function is required for the last stage.
 * \param line Buffer holing line characters.
 * \return 1 if buffer contains lock line, 0 otherwise.
 */
int is_lock_line(const char line[MAX_LINE_LENGTH]) {
    // One could use memcmp() here
    for (int i = 0; i < 63; ++i) {
        if (line[i] != 'X')
            return 0;
    }
    return 1;
}

/**
 * \brief Validates a line read from *.vdb file. It can distinguish between a lock line and a normal record
 * and returns different values.
 * \param line Buffer containing a line read from VDB (including a newline character).
 * \param name Buffer where parsed vote option name will be stored (plus terminating NULL byte).
 * \param val Pointer to an integer where parsed vote count will be stored.
 * \return Parse result flag - see possible return values.
 * \retval 0 Invalid line.
 * \retval 1 Normal record line was parsed - name and val contain parsed components.
 * \retval 2 A lock line was parsed.
 */
int parse_line(const char line[MAX_LINE_LENGTH], char name[MAX_OPTION_NAME_LENGTH + 1], int *val) {
    // Note the %c conversion usage - it won't store terminating NULL byte to the buffer.
    // Note the check for return value of sscanf().
    if (is_lock_line(line)) {
        return 2;
    } else if (sscanf(line, "%32c%32d", name, val) == 2) {
        name[MAX_OPTION_NAME_LENGTH] = '\0';
        for (int i = MAX_OPTION_NAME_LENGTH - 1; i >= 0; i--) {
            // Trim the option name from the right to create a valid C string
            if (isspace(name[i])) {
                name[i] = '\0';
            } else break;
        }
        return 1;
    } else return 0;
}

/**
 * \brief Create new database file.
 * In case the file already exists function exits with error.
 * This should be implemented in stage 1 (without lock parameter).
 * \param dbname Filesystem path to the new database file.
 * \param optc Number of vote options passed to the program.
 * \param optv Vote option strings passed to the program.
 * \param lock Database lock flag. If not equal to 0 then database shall be locked.
 */
void do_create(const char *dbname, int optc, char *const *optv, int lock) {
    printf("Creating %s database in %s\n", lock ? "locked" : "unlocked", dbname);

    // Check if file already exists by trying to open it for reading.
    // Note that this method is not fully correct - someone could create a file after this line and before next fopen() call.
    FILE *f = fopen(dbname, "r");
    if (f) {
        fprintf(stderr, "File already exists!\n");
        fclose(f);
        exit(EXIT_FAILURE);
    }

    f = fopen(dbname, "w");

    // This is temporary working buffer which will be saved to a file repeatedly
    char line[MAX_LINE_LENGTH];

    for (int i = 0; i < optc; ++i) {
        if (strlen(optv[i]) > MAX_OPTION_NAME_LENGTH) {
            fclose(f);
            fprintf(stderr, "Too long option given\n");
            exit(EXIT_FAILURE);
        }
        make_line(line, optv[i], 0);
        // make_line() always returns string of length 63, below we append a newline to it.
        fprintf(f, "%s\n", line);
    }

    // This is required for the last stage
    if (lock) {
        make_lock_line(line);
        fprintf(f, "%s\n", line);
    }

    fclose(f);
}

/**
 * \brief Format the database contents to stdout.
 * This should be implemented in stage 2 (without lock line handling).
 * \param dbname Filesystem path to the existing database file.
 */
void do_show(const char *dbname) {
    FILE *f = fopen(dbname, "r");
    if (!f) {
        fprintf(stderr, "Could not open database file\n");
        exit(EXIT_FAILURE);
    }

    printf("Database filename: %s\n", dbname);

    // Note the +1 as fgets() will store both the newline character and a terminating NULL byte.
    char line[MAX_LINE_LENGTH + 1];
    int locked = 0;

    while (fgets(line, sizeof(line), f) != NULL) {
        // This is required for the last stage
        if (locked) {
            fclose(f);
            db_corrupted("record found after lock record");
        }

        // Verify that we've read a line of correct length and drop the newline character.
        if (line[MAX_LINE_LENGTH - 1] != '\n') {
            fclose(f);
            db_corrupted("too long line found");
        }
        line[MAX_LINE_LENGTH - 1] = '\0';

        char name[MAX_OPTION_NAME_LENGTH + 1];
        int val;

        switch (parse_line(line, name, &val)) {
            case 0:
                fclose(f);
                db_corrupted("invalid line format");
                break;
            case 1:
                // Correct line can be printed right away
                printf("%s\n", line);
                break;
            case 2:
                // This is required for the last stage
                locked = 1;
                break;
            default:
                // This shall never happen
                fclose(f);
                db_corrupted("???");
                break;
        }
    }

    // For the second stage one could just always print "unlocked"
    printf("Database is %s\n", locked ? "locked" : "unlocked");

    fclose(f);
}

/**
 * \brief Alternate the vote count in the database for given option.
 * \param dbname Filesystem path to the existing database file.
 * \param opt Vote option name for which modification will happen.
 * \param rev Vote revocation flag. In case it is not equal to 0, the vote count shall be decremented.
 */
void do_vote(const char *dbname, const char *opt, int rev) {
    if (strlen(opt) > MAX_OPTION_NAME_LENGTH) {
        fprintf(stderr, "Too long option given\n");
        exit(EXIT_FAILURE);
    }

    FILE *f = fopen(dbname, "r+");
    if (!f) {
        fprintf(stderr, "Could not open database file\n");
        exit(EXIT_FAILURE);
    }

    // Note the +1 as fgets() will store both the newline character and a terminating NULL byte.
    char line[MAX_LINE_LENGTH + 1];
    int line_index = 0;
    int found = 0;
    int lock = 0;

    while (fgets(line, sizeof(line), f) != NULL) {
        // This is required for the last stage
        if (lock) {
            fclose(f);
            db_corrupted("record found after lock record");
        }
        if (line[MAX_LINE_LENGTH - 1] != '\n') {
            fclose(f);
            db_corrupted("too long line found");
        }
        line[MAX_LINE_LENGTH - 1] = '\0';
        char name[MAX_OPTION_NAME_LENGTH + 1];
        int val;

        switch (parse_line(line, name, &val)) {
            case 0:
                fclose(f);
                db_corrupted("invalid line format");
                break;
            case 1:
                if (strncmp(name, opt, MAX_OPTION_NAME_LENGTH) == 0) {
                    // After reading a line we search for we have to move back in file to overwrite it.
                    // An absolute position in file is given here.
                    if (fseek(f, MAX_LINE_LENGTH * line_index, SEEK_SET) != 0) {
                        fclose(f);
                        db_corrupted("could not change position in file");
                    }
                    make_line(line, name, rev ? (val > 0 ? val - 1 : 0) : val + 1);
                    fprintf(f, "%s\n", line);
                    found = 1;
                }
                break;
            case 2:
                // This is required for the last stage
                lock = 1;
                break;
            default:
                // This shall never happen
                fclose(f);
                db_corrupted("???");
                break;
        }

        line_index++;
    }

    // We have to append a new option in case is was not found in the database
    if (!found) {
        // This is required for the last stage
        if (lock) {
            fprintf(stderr, "Database is locked - cannot add a new option '%s'\n", opt);
            fclose(f);
            exit(EXIT_FAILURE);
        } else if (!rev) {
            make_line(line, opt, 1);
            fprintf(f, "%s\n", line);
        } else {
            fprintf(stderr, "Cannot revoke vote from unknown option '%s'\n", opt);
            fclose(f);
            exit(EXIT_FAILURE);
        }
    }

    fclose(f);
}


int main(int argc, char **argv) {

    // Each command requires at least 2 arguments
    if (argc < 2) {
        show_usage();
    }

    // The first one is always a command name
    char *command = argv[1];

    if (strcmp(command, "create") == 0) {
        //TODO: Check and comment
        if (argc < 3) {
            show_usage();
        } else if (strcmp(argv[2], "-l") == 0 && argc >= 4) {
            do_create(argv[3], argc - 4, argv + 4, 1);
        } else {
            do_create(argv[2], argc - 3, argv + 3, 0);
        }
    } else if (strcmp(command, "show") == 0) {
        // In case of show command no other arguments are required
        if (argc != 3)
            show_usage();
        do_show(argv[2]);
    } else if (strcmp(command, "vote") == 0) {
        // When voting a vote option should be passed, possibly preceded by -r
        if (argc == 4) {
            do_vote(argv[2], argv[3], 0);
        } else if (argc == 5 && strcmp(argv[3], "-r") == 0) {
            do_vote(argv[2], argv[4], 1);
        } else {
            show_usage();
        }
    } else {
        show_usage();
    }

    return EXIT_SUCCESS;
}
