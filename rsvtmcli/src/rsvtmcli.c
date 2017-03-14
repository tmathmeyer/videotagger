#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <dirent.h>
#include <stdbool.h>

#define MSG "HELP MESSAGE TODO"
#define exit_msg(...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
        exit(1); \
    } while(0)
void apply_tag(char *, char *);

typedef enum {
    NONE, HELP, INIT, TRACK, TAG, SEARCH, FULL
} cli_opt;

void exit_help(void) {
    puts(MSG);
    exit(1);
}

/**
 * rtvsmcli help
 * rtvsmcli init
 * rtvsmcli track [filename]
 * rtvsmcli tag [filename] [tag] #Creates a new tag if non-existant
 * rtvsmcli search [query]
 * rtvsmcli full [filename]
 */
int main(int argc, char **argv) {
    if (argc == 1) {
        exit_help();
    }
    cli_opt input = NONE;
    if (!strcmp(argv[1], "tag")) input = TAG;
    else if (!strcmp(argv[1], "help")) input = HELP;
    else if (!strcmp(argv[1], "full")) input = FULL;
    else if (!strcmp(argv[1], "init")) input = INIT;
    else if (!strcmp(argv[1], "track")) input = TRACK;
    else if (!strcmp(argv[1], "search")) input = SEARCH;
    
    switch(input) {
        case NONE:
        case HELP:
            exit_help();
            break;
        case FULL: {
            if (argc != 3) {
                exit_msg("full requires a filename");
            }
            if (access(".rsvtm", F_OK)) {
                exit_msg("Must run in a rsvtm directory");
            }
            char *file_name = argv[2];
            char *base_name = basename(file_name);
            char *dir = calloc(strlen(".tags/Tracking/")+strlen(base_name)+1, sizeof(char));
            sprintf(dir, ".tags/Tracking/%s", base_name);

            char *actual = realpath(dir, NULL);
            if (actual != NULL) {
                puts(actual);
                free(dir);
            } else {
                exit_msg("Could not read link at %s\n", dir);
            }
            break;
        }
        case INIT:
            if (!access(".rsvtm", F_OK)) {
                exit_msg("Already exists!");
            }
            int rtsvm_signal_fd = open(".rsvtm", O_RDWR|O_CREAT, S_IRUSR|S_IRGRP|S_IROTH);
            if (rtsvm_signal_fd == -1) {
                exit_msg("Cannot initialize - no permissions");
            }
            close(rtsvm_signal_fd);
            mkdir(".tags", 0775);
            mkdir(".tags/Tracking", 0775);
            mkdir(".applied", 0775);
            break;
        case TRACK: {
            if (argc != 3) {
                exit_msg("tracking a file requires a filename");
            }
            if (access(".rsvtm", F_OK)) {
                exit_msg("Must run in a rsvtm directory");
            }
            char *file_name = argv[2];
            char *base_name = basename(file_name);
            char *applied_dir = calloc(strlen(".applied/")+strlen(base_name)+1, sizeof(char));
            sprintf(applied_dir, ".applied/%s", base_name);
            if (!access(applied_dir, F_OK)) {
                free(applied_dir);
                exit_msg("Already tracking that file");
            }
            if (mkdir(applied_dir, 0775)) {
                free(applied_dir);
                exit_msg("Could not make directory");
            }
            apply_tag(file_name, "Tracking");
            break;
        }
        case TAG: {
            if (argc != 4) {
                exit_msg("tagging a file requires a file and a tag");
            }
            if (access(".rsvtm", F_OK)) {
                exit_msg("Must run in a rsvtm directory");
            }
            char *file_name = argv[2];
            char *base_name = basename(file_name);
            char *applied_dir = calloc(strlen(".applied/")+strlen(base_name)+1, sizeof(char));
            char *tracked_name = calloc(strlen(".tags/Tracking/")+strlen(base_name)+1, sizeof(char));
            sprintf(tracked_name, ".tags/Tracking/%s", base_name);
            sprintf(applied_dir, ".applied/%s", base_name);
            if (access(applied_dir, F_OK)) {
                free(applied_dir);
                exit_msg("Not tracking that file");
            }
            apply_tag(tracked_name, argv[3]);
            break;
        }
        case SEARCH: {
            if (access(".rsvtm", F_OK)) {
                exit_msg("Must run in a rsvtm directory");
            }
            int nots=0,havs=0;
            for(int i=2; i<argc; i++) {
                if (argv[i][0] == '-') {
                    nots++;
                }
                if (argv[i][0] == '+') {
                    havs++;
                }
            }
            char **not_list = malloc(sizeof(char *) * nots);
            char **hav_list = malloc(sizeof(char *) * havs);
            nots=0,havs=0;
            for(int i=2; i<argc; i++) {
                if (argv[i][0] == '-') {
                    not_list[nots++] = argv[i]+1;
                }
                if (argv[i][0] == '+') {
                    hav_list[havs++] = argv[i]+1;
                }
            }

            DIR *dir;
            struct dirent *ent;
            if ((dir = opendir (".tags/Tracking")) != NULL) {
                while ((ent = readdir (dir)) != NULL) {
                    char *dirname = ent->d_name;
                    char *tagdir = NULL;
                    bool print = (dirname[0]!='.');
                    if (print) {
                        for(int i=0; i<nots; i++) {
                            tagdir = calloc(strlen(".tags//")+strlen(not_list[i])+strlen(dirname)+2, sizeof(char));
                            if (!tagdir) {
                                exit_msg("Not Enough Memory");
                            }
                            sprintf(tagdir, ".tags/%s/%s", not_list[i], dirname);
                            if (!access(tagdir, F_OK)) {
                                print = false;
                                i = nots;
                            }
                            free(tagdir);
                        }
                    }
                    if (print) {
                        for(int i=0; i<havs; i++) {
                            tagdir = calloc(strlen(".tags//")+strlen(hav_list[i])+strlen(dirname)+2, sizeof(char));
                            if (!tagdir) {
                                exit_msg("Not Enough Memory");
                            }
                            sprintf(tagdir, ".tags/%s/%s", hav_list[i], dirname);
                            if (access(tagdir, F_OK)) {
                                print = false;
                                i = havs;
                            }
                            free(tagdir);
                        }
                    }
                    if (print) {
                        puts(dirname);
                    }
                }
                closedir (dir);
            } else {
                exit_msg("Could not open directory");
            }
        }
    }
}

void apply_tag(char *file, char *tag) {
    char *file_base = basename(file);
    
    char *tag_dir = calloc(strlen(".tags/")+strlen(tag)+1, sizeof(char));
    sprintf(tag_dir, ".tags/%s", tag);

    char *tag_link = calloc(strlen(tag_dir)+strlen(file_base)+2, sizeof(char));
    sprintf(tag_link, "%s/%s", tag_dir, file_base);

    char *applied_link = calloc(strlen(".applied//")+strlen(tag)+strlen(file_base)+1, sizeof(char));
    sprintf(applied_link, ".applied/%s/%s", file_base, tag);

    char *applied_src = calloc(strlen("../../.tags/")+strlen(tag)+1, sizeof(char));
    sprintf(applied_src, "../../.tags/%s", tag);

    char *err_msg = NULL;
    char *nfile = NULL;

    if (file[0] != '/') {
        nfile = calloc(strlen("../../")+strlen(file)+1, sizeof(char));
        sprintf(nfile, "../../%s", file);
    } else {
        nfile = strdup(file);
    }

    if (access(tag_dir, F_OK)) {
        if (mkdir(tag_dir, 0775)) {
            err_msg = "Could not make directory";
            goto cleanup;
        }
    }

    if (symlink(nfile, tag_link)) {
        err_msg = "Could not create symlink";
        goto cleanup;
    }

    if (symlink(applied_src, applied_link)) {
        err_msg = "Could not create symlink";
        goto cleanup;
    }

cleanup:
    free(tag_dir);
    free(tag_link);
    free(applied_link);
    free(nfile);
    free(applied_src);
    if (err_msg) {
        exit_msg(err_msg);
    }
}
