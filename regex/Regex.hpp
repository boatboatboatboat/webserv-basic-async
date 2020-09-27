#ifndef REGEX_H
#define REGEX_H

#include <string>

#define MAX_NODES 64
#define BUFFER_LEN 128

typedef struct regex_node regex_node_t;
typedef struct regex_comp regex_comp_t;

struct regex_node {
    unsigned char type;
    union {
        char ch;
        char* cc;
        unsigned short mn[2];
    } u;
};

struct regex_comp {
    regex_node_t nodes[MAX_NODES];
    char buffer[BUFFER_LEN];
};

#define MATCH_DIGIT(c) ((c >= '0') && (c <= '9'))
#define MATCH_ALPHA(c) ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))
#define MATCH_SPACE(c) ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') || (c == '\f') || (c == '\v'))
#define MATCH_ALNUM(c) ((c == '_') || MATCH_ALPHA(c) || MATCH_DIGIT(c))
#define MATCH_DOT(c) ((c != '\n') && (c != '\r'))

#define IS_META(c) ((c == 's') || (c == 'S') || (c == 'w') || (c == 'W') || (c == 'd') || (c == 'D'))

#define MAX_QUANTITY 1024
#define MAX_PLUS_STAR 40000

#define ENUMS                                                      \
    X(NONE)                                                        \
    X(BEGIN)                                                       \
    X(END)                                                         \
    X(QUANT)                                                       \
    X(LQUANT) X(QMARK) X(LQMARK) X(STAR) X(LSTAR) X(PLUS) X(LPLUS) \
        X(DOT) X(CHAR) X(CLASS) X(NCLASS) X(DIGIT) X(NDIGIT) X(ALPHA) X(NALPHA) X(SPACE) X(NSPACE)

#define X(A) REGEX_##A,

enum { ENUMS };

class Regex {
private:
    regex_comp_t regex;

protected:
    auto matchPattern(const regex_node_t* nodes, const char* text, const char* end) const -> const char*;
    auto matchCharacterClass(char c, const char* str) const -> int;
    static auto matchMetaCharacter(char c, char mc) -> int;
    auto matchOne(const regex_node_t* node, char c) const -> int;
    auto matchQuantity(const regex_node_t* nodes, const char* text, const char* tend, unsigned min, unsigned max) const -> const char*;
    auto matchQuantityLazy(const regex_node_t* nodes, const char* text, const char* tend, unsigned min, unsigned max) const -> const char*;
    static auto error(const char* message) -> int;

public:
    explicit Regex(std::string const& pattern);
    explicit Regex(const char* pattern);

    auto compile(const char* pattern) -> int;
    auto match(const char* text, const char** end) const -> const char*;
};

#endif
