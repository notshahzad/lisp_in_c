#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
typedef enum TOKEN_TYPE {
  TOK_STRING,
  TOK_NUMBER,
  TOK_PLUS,
  TOK_MINUS,
  TOK_UNIDENTIFIED,
  TOK_MUL,
  TOK_DIV,
  TOK_QUOTE,
  TOK_OSQUARE,
  TOK_CSQUARE,
  TOK_OROUND,
  TOK_CROUND,
  TOK_OCURLY,
  TOK_CCURLY,
  TOK_FUNC_NAME,
} TOKEN_TYPE;

typedef struct token {
  TOKEN_TYPE tok_type;
  char *data;
} token;
typedef struct token_list {
  int32_t size;
  int32_t capacity;
  token **items;
} token_list;

token_list tl = {0};
void tl_init(token_list *tl, int capacity) {
  tl = malloc(sizeof(token_list));
  tl->capacity = capacity;
}

#define da_append(da, item)                                                    \
  do {                                                                         \
    if (da.size == da.capacity) {                                              \
      da.capacity *= 2;                                                        \
      da.items = realloc(da.items, da.capacity);                               \
    }                                                                          \
    da.items[da.size++] = item;                                                \
  } while (0)

void print_usage(int argc, char **args) {
  (void)argc;
  fprintf(stdout, "file: %s", args[0]);
  fprintf(stdout, "\tusage: %s [input file]", args[0]);
}
char *substr(char *start_cursor, char *end_cursor) {
  char *tok_str = malloc(sizeof(char) * ((end_cursor - start_cursor) + 1));
  snprintf(tok_str, (end_cursor - start_cursor) + 1, "%s", start_cursor);
  return tok_str;
}

token *create_token(TOKEN_TYPE type, char *data) {
  token *tok = malloc(sizeof(*tok));
  tok->tok_type = type;
  tok->data = data;
  return tok;
}

bool map_identifiers(char *start_cursor, char *end_cursor) {
  bool found = false;
  if (strncmp(start_cursor, "print", end_cursor - start_cursor) == 0) {
    token *t = create_token(TOK_FUNC_NAME, substr(start_cursor, end_cursor));
    printf("(%s)\n", t->data);
    found = true;
  }
  return found;
}
bool is_arithematic(char identifier) {
  return (identifier == '+' || identifier == '-' || identifier == '/' ||
          identifier == '*');
}
TOKEN_TYPE arithmentic_to_token(char identifier) {
  switch (identifier) {
  case '+':
    return TOK_PLUS;
  case '-':
    return TOK_MINUS;
  case '/':
    return TOK_DIV;
  case '*':
    return TOK_MUL;
  default:
    assert("ILLEGAL ARITHEMATIC TOKEN");
  }
  return TOK_UNIDENTIFIED;
}

void extract_tokens_from_string(char *data) {
  char *start_cursor = data;

  while (*start_cursor != 0) {
    if (isdigit(*start_cursor)) {
      char *end_cursor = start_cursor;
      while (isdigit(*end_cursor)) {
        end_cursor++;
      }
      end_cursor++;
      char *tok_str = substr(start_cursor, end_cursor);
      token *t = create_token(TOK_NUMBER, tok_str);
      da_append(tl, t);
      printf("TOKEN NUMBER :%s\n", t->data);
      start_cursor = end_cursor;
    } else if (is_arithematic(*start_cursor)) {
      TOKEN_TYPE tok = arithmentic_to_token(*start_cursor);
      if (tok != TOK_UNIDENTIFIED) {
        token *t = create_token(tok, NULL);
        da_append(tl, t);
      } else {
        printf("UNIDENTIFIED TOKEN :%c\n", *start_cursor);
      }
      start_cursor++;
    } else if (*start_cursor == '(') {

      token *t = create_token(TOK_OROUND, NULL);
      da_append(tl, t);
      printf("(TOK_OROUND)\n");
      start_cursor++;
    } else if (*start_cursor == ')') {
      token *t = create_token(TOK_CROUND, NULL);
      da_append(tl, t);
      printf("(TOK_CROUND)\n");
      start_cursor++;
    } else if (*start_cursor == '"') {
      char *end_cursor = start_cursor + 1;
      while (*end_cursor != 0 && *end_cursor != '"') {
        end_cursor++;
      }
      end_cursor++;
      char *tok_str = substr(start_cursor, end_cursor);
      token *t = create_token(TOK_STRING, tok_str);
      da_append(tl, t);
      printf("(TOK_STRING)%s\n", tok_str);
      start_cursor = end_cursor;
    } else if (isalpha(*start_cursor)) {
      char *end_cursor = start_cursor;
      while (isalpha(*end_cursor))
        end_cursor++;
      bool is_identifier = map_identifiers(start_cursor, end_cursor);
      if (is_identifier) {
        start_cursor = end_cursor++;
      } else {
        printf("unknown identifier %.*s\n", (int)(end_cursor - start_cursor),
               start_cursor);
      }
    } else if (*start_cursor == ' ') {
      start_cursor++;
    } else if (*start_cursor == '\t') {
      start_cursor++;
    } else if (*start_cursor == '\n') {
      start_cursor++;
    } else {
      printf("ILLEGAL IDENTIFIER SKIPPING\n");
      start_cursor++;
    }
  }
  return;
}

int main(int argc, char **args) {
  tl_init(&tl, 16);
  if (argc < 2) {
    print_usage(argc, args);
    exit(EXIT_FAILURE);
  }
  int fd = open(args[1], O_RDONLY | O_CREAT);
  struct stat st;
  stat(args[1], &st);
  int size = st.st_size;
  char *file_data = malloc(sizeof(char) * size);
  read(fd, file_data, size);
  extract_tokens_from_string(file_data);
  return 0;
}
