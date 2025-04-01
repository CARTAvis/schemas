/* This program converts a schema file into a header which can be included in 
the CARTA backend at compile time. The header contains an unsigned char array 
containing the raw data, an unsigned int containing the array length, and a 
std::string constructed from these variables.

Based on: https://github.com/mortie/strliteral
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

static void make_identifier(char *str) {
    char c;
    size_t i;
    for (i = 0; (c = str[i]) != '\0'; ++i) {
        if (c == '.') {
            str[i] = '\0';
            return;
        } else if (!(
                (c >= '0' && c <= '9') ||
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z'))) {
            str[i] = '_';
        }
    }
}

static void make_headername(char *str) {
    char c;
    size_t i;
    for (i = 0; (c = str[i]) != '\0'; ++i) {
        if (c >= 'a' && c <= 'z') {
            str[i] = toupper((unsigned char) c);
        }
    }
}

/* strdup is actually a POSIX thing, not a C thing, so don't use it */
static char *dupstr(char *str) {
    size_t len = strlen(str);
    char *dup = malloc(len + 1);
    strcpy(dup, str);
    dup[len] = '\0';
    return dup;
}

int main(int argc, char **argv) {
    int ret = 0;
    char *inp = "stdin", *outp = "stdout";
    FILE *inf = stdin, *outf = stdout;
    char *inbase = NULL;
    char *ident = NULL;
    char *headername = NULL;
    char *buffer = NULL;
    char *conststr = "const ";
    int alwaysescape = 0;
    size_t maxlength = 100;

    if (argc < 3) {
        fprintf(stderr, "USAGE: schemacompiler <input> <output>\n");
        goto fail;
    }

    inp = argv[1];
    inf = fopen(inp, "rb");
    if (inf == NULL) {
        perror(inp);
        goto fail;
    }

    (inbase = strrchr(inp, '/')) ? ++inbase : (inbase = inp);
    ident = dupstr(inbase);
    make_identifier(ident);
    
    headername = dupstr(ident);
    make_headername(headername);

    outp = argv[2];
    outf = fopen(outp, "w");
    if (outf == NULL) {
        perror(outp);
        goto fail;
    }
    
    if (fprintf(outf, "#ifndef _%s\n#define _%s\n\nnamespace CARTASCHEMA {\n\n",
                headername, headername) < 0) {
        perror("write header");
        goto fail;
    }

    if (fprintf(outf, "%sunsigned char %s_char[] =\n\t\"", conststr, ident) < 0) {
        perror("write char header");
        goto fail;
    }

    buffer = malloc(maxlength + 4);

    size_t linechar = 0;
    size_t length = 0;
    int c;
    while ((c = fgetc(inf)) != EOF) {
        if (alwaysescape) {
            buffer[linechar++] = '\\';
            buffer[linechar++] = '0' + ((c & 0700) >> 6);
            buffer[linechar++] = '0' + ((c & 0070) >> 3);
            buffer[linechar++] = '0' + ((c & 0007) >> 0);
        } else if (c >= 32 && c <= 126 && c != '"' && c != '\\' && c != '?' && c != ':' && c != '%') {
            buffer[linechar++] = (char)c;
        } else if (c == '\r') {
            buffer[linechar++] = '\\';
            buffer[linechar++] = 'r';
        } else if (c == '\n') {
            buffer[linechar++] = '\\';
            buffer[linechar++] = 'n';
        } else if (c == '\t') {
            buffer[linechar++] = '\\';
            buffer[linechar++] = 't';
        } else if (c == '\"') {
            buffer[linechar++] = '\\';
            buffer[linechar++] = '"';
        } else if (c == '\\') {
            buffer[linechar++] = '\\';
            buffer[linechar++] = '\\';
        } else {
            buffer[linechar++] = '\\';
            buffer[linechar++] = '0' + ((c & 0700) >> 6);
            buffer[linechar++] = '0' + ((c & 0070) >> 3);
            buffer[linechar++] = '0' + ((c & 0007) >> 0);
        }

        length += 1;

        if (linechar >= maxlength) {
            if (fwrite(buffer, 1, linechar, outf) != linechar) {
                perror("write");
                goto fail;
            }

            if (fwrite("\"\n\t\"", 1, 4, outf) != 4) {
                perror("write");
                goto fail;
            }

            linechar = 0;
        }
    }

    if (linechar >= 1) {
        if (fwrite(buffer, 1, linechar, outf) != linechar) {
            perror("write buffer");
            goto fail;
        }
    }

    if (ferror(inf)) {
        perror("read");
        goto fail;
    }

    if (fprintf(outf, "\";\n\n%sunsigned int %s_len = %u;\n",
                conststr, ident, (unsigned int)length) < 0) {
        perror("write char footer");
        goto fail;
    }

    if (fprintf(outf, "\ninline std::string_view %s(reinterpret_cast<%schar*>(%s_char), %s_len);\n",
               ident, conststr, ident, ident) < 0) {
        perror("write string");
        goto fail;
    }

    if (fprintf(outf, "\n} // CARTASCHEMA\n#endif // _%s\n",
                headername) < 0) {
        perror("write footer");
        goto fail;
    }

    if (fclose(inf) == EOF) {
        perror("close");
        goto fail;
    }

    if (fclose(outf) == EOF) {
        perror("close");
        goto fail;
    }

exit:
    free(ident);
    free(headername);
    free(buffer);
    return ret;

fail:
    ret = 1;
    goto exit;
}
