//
// Created by Harm Smits on 7/28/20.
//

#include "Regex.hpp"
#include "../utils/utils.hpp"

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

auto Regex::error(const char* message) -> int
{
    (void)message;
    return 0;
}

auto Regex::compile(const char* pattern) -> int
{
    size_t len;
#ifdef REGEX_DEBUGGER_STRING
    _regex_debugger_string = std::string(pattern);
#endif

    if (!pattern || !(len = utils::strlen(pattern)))
        return this->error("Pattern does not have a length");

    auto nodes = this->regex.nodes;
    auto buffer = this->regex.buffer;
    auto buffer_length = sizeof(this->regex.buffer);
    auto quantifiable = '\0';
    char temp;

    unsigned index = 0;

    unsigned long value;
    unsigned i = 0;
    unsigned j = 0;

    while (i < len && (j + 1 < MAX_NODES)) {
        switch (pattern[i]) {
        case '^':
            quantifiable = 0;
            nodes[j].type = REGEX_BEGIN;
            break;

        case '$':
            quantifiable = 0;
            nodes[j].type = REGEX_END;
            break;

        case '.':
            quantifiable = 1;
            nodes[j].type = REGEX_DOT;
            break;

        case '*':
            if (unlikely(!quantifiable))
                return this->error("Non quantifiable character before '*'");

            quantifiable = 0;
            nodes[j].type = (pattern[i + 1] == '?') ? (i++, REGEX_LSTAR) : REGEX_STAR;
            break;

        case '+':
            if (unlikely(!quantifiable))
                return this->error("Non quantifiable character before '+");

            quantifiable = 0;
            nodes[j].type = (pattern[i + 1] == '?') ? (i++, REGEX_LPLUS) : REGEX_LPLUS;
            break;

        case '?':
            if (unlikely(!quantifiable))
                return this->error("Non quantifiable character before '?");

            quantifiable = 0;
            nodes[j].type = (pattern[i + 1] == '?') ? (i++, REGEX_LQMARK) : REGEX_QMARK;
            break;

        case '\\': {
            quantifiable = 1;
            if (unlikely(++i >= len))
                return this->error("Dangling \\");

            switch (pattern[i]) {
            case 'd':
                nodes[j].type = REGEX_DIGIT;
                break;
            case 'D':
                nodes[j].type = REGEX_NDIGIT;
                break;
            case 'w':
                nodes[j].type = REGEX_ALPHA;
                break;
            case 'W':
                nodes[j].type = REGEX_NALPHA;
                break;
            case 's':
                nodes[j].type = REGEX_SPACE;
                break;
            case 'S':
                nodes[j].type = REGEX_NSPACE;
                break;
            default:
                nodes[j].type = REGEX_CHAR;
                nodes[j].u.ch = pattern[i];
                break;
            }
        } break;

        case '[': {
            quantifiable = 1;
            nodes[j].type = (pattern[i + 1] == '^') ? (i++, REGEX_NCLASS) : REGEX_CLASS;
            nodes[j].u.cc = buffer + index;

            while (pattern[++i] != ']' && i < len) {
                temp = 0;

                if (pattern[i] == '\\') {
                    if (unlikely(++i < len))
                        return this->error("Dangling \\");

                    temp = IS_META(pattern[i]);
                    if (temp || pattern[i] == '\\') {
                        if (unlikely(index > buffer_length - 2))
                            return this->error("Buffer overflow");
                        buffer[index] = '\\';
                    }
                }

                if (unlikely(index > buffer_length - 2))
                    return this->error("Buffer overflow");

                buffer[index++] = pattern[i];

                if (temp)
                    continue;
                if (pattern[i] != '-' || i + 2 >= len || pattern[i + 2] == ']')
                    continue;
                if ((temp = (char)(pattern[i + 2] == '\\')) && (i + 3 >= len || IS_META(pattern[i + 3])))
                    continue;

                temp = pattern[i + 2 + (bool)(temp)];
                if (unlikely(temp < pattern[i]))
                    return this->error("Incorrect range");
            }

            if (unlikely(pattern[i] != ']'))
                return this->error("Non terminated range");

            buffer[index++] = 0;
        } break;

        case '{': {
            if (unlikely(!quantifiable))
                return this->error("Non quantifiable character before '{");

            i++;
            value = 0;
            quantifiable = 0;

            do {
                if (unlikely(i >= len || pattern[i] < '0' || pattern[i] > '9'))
                    return this->error("Non-digit min value in quantifier");
                value = 10 * value + (unsigned)(pattern[i++] - '0');
            } while (pattern[i] != ',' && pattern[i] != '}');

            if (unlikely(value > MAX_QUANTITY))
                return this->error("Minimum value too large in quantifier");

            nodes[j].u.mn[0] = value;

            if (pattern[i] == ',') {
                if (unlikely(++i >= len))
                    return this->error("Dangling ',' in quantifier");
                if (pattern[i] == '}') {
                    value = MAX_QUANTITY;
                } else {
                    value = 0;
                    while (pattern[i] != '}') {
                        if (unlikely(i >= len || pattern[i] < '0' || pattern[i] > '9'))
                            return this->error("Non-digit max value in quantifier");
                        value = 10 * value + (unsigned)(pattern[i++] - '0');
                    }

                    if (unlikely(value > MAX_QUANTITY || value < nodes[j].u.mn[0]))
                        return this->error("Max value too big or less than min value in quantifier");
                }

                nodes[j].type = (i + 1 < len && pattern[i + 1] == '?') ? (i++, REGEX_LQUANT) : REGEX_QUANT;
                nodes[j].u.mn[1] = value;
            }
        } break;

        default:
            quantifiable = 1;
            nodes[j].type = REGEX_CHAR;
            nodes[j].u.ch = pattern[i];
            break;
        }

        i++, j++;
    }

    nodes[j].type = REGEX_NONE;

    return 1;
}

auto Regex::matchMetaCharacter(char c, char mc) -> int
{
    switch (mc) {
    case 'd':
        return MATCH_DIGIT(c);

    case 'D':
        return !MATCH_DIGIT(c);

    case 'w':
        return MATCH_ALNUM(c);

    case 'W':
        return !MATCH_ALNUM(c);

    case 's':
        return MATCH_SPACE(c);

    case 'S':
        return !MATCH_SPACE(c);

    default:
        return (c == mc);
    }
}

auto Regex::matchCharacterClass(char c, const char* str) const -> int
{
    char rmax;
    while (*str != '\0') {
        if (str[0] == '\\') {
            if (this->matchMetaCharacter(c, str[1]))
                return 1;
            str += 2;
            if (IS_META(*str))
                continue;
        } else {
            if (c == *str)
                return 1;
            str += 1;
        }

        if (*str != '-' || !str[1])
            continue;
        if ((rmax = (char)(str[1] == '\\')) && IS_META(str[2]))
            continue;
        if (c >= str[-1] && c <= (rmax ? str[2] : str[1]))
            return 1;
        str++;
    }

    return 0;
}

auto Regex::matchOne(const regex_node_t* node, char c) const -> int
{
    switch (node->type) {
    case REGEX_CHAR:
        return (node->u.ch == c);

    case REGEX_DOT:
        return MATCH_DOT(c);

    case REGEX_CLASS:
        return this->matchCharacterClass(c, node->u.cc);

    case REGEX_NCLASS:
        return !this->matchCharacterClass(c, node->u.cc);

    case REGEX_DIGIT:
        return MATCH_DIGIT(c);

    case REGEX_NDIGIT:
        return !MATCH_DIGIT(c);

    case REGEX_ALPHA:
        return MATCH_ALNUM(c);

    case REGEX_NALPHA:
        return !MATCH_ALNUM(c);

    case REGEX_SPACE:
        return MATCH_SPACE(c);

    case REGEX_NSPACE:
        return !MATCH_SPACE(c);

    default:
        return 0;
    }
}

auto Regex::matchQuantity(const regex_node_t* nodes, const char* text, const char* tend, unsigned min,
    unsigned max) const -> const char*
{
    const char *end, *start = text + min;
    while (max && text < tend && this->matchOne(nodes, *text))
        text++, max--;

    while (text >= start) {
        if ((end = this->matchPattern(nodes + 2, text--, tend)))
            return end;
    }

    return nullptr;
}

auto Regex::matchQuantityLazy(const regex_node_t* nodes, const char* text, const char* tend, unsigned int min,
    unsigned int max) const -> const char*
{
    const char* end;
    max = max - min;

    while (min && text < tend && this->matchOne(nodes, *text))
        text++, min--;

    if (min)
        return nullptr;

    if ((end = this->matchPattern(nodes + 2, text, tend)))
        return end;

    while (max && text < tend && this->matchOne(nodes, *text)) {
        text++, max--;
        if ((end = this->matchPattern(nodes + 2, text, tend)))
            return end;
    }

    return nullptr;
}

auto Regex::matchPattern(const regex_node_t* nodes, const char* text, const char* tend) const -> const char*
{
    do {
        if (nodes[0].type == REGEX_NONE)
            return text;

        if ((nodes[0].type == REGEX_END) && nodes[1].type == REGEX_NONE)
            return (text == tend) ? text : nullptr;

        switch (nodes[1].type) {
        case REGEX_QMARK:
            return this->matchQuantity(nodes, text, tend, 0, 1);
        case REGEX_LQMARK:
            return this->matchQuantityLazy(nodes, text, tend, 0, 1);
        case REGEX_QUANT:
            return this->matchQuantity(nodes, text, tend, nodes[1].u.mn[0], nodes[1].u.mn[1]);
        case REGEX_LQUANT:
            return matchQuantityLazy(nodes, text, tend, nodes[1].u.mn[0], nodes[1].u.mn[1]);
        case REGEX_STAR:
            return this->matchQuantity(nodes, text, tend, 0, MAX_PLUS_STAR);
        case REGEX_LSTAR:
            return matchQuantityLazy(nodes, text, tend, 0, MAX_PLUS_STAR);
        case REGEX_PLUS:
            return this->matchQuantity(nodes, text, tend, 1, MAX_PLUS_STAR);
        case REGEX_LPLUS:
            return matchQuantityLazy(nodes, text, tend, 1, MAX_PLUS_STAR);
        }
    } while (text < tend && this->matchOne(nodes++, *text++));

    return nullptr;
}

auto Regex::match(const char* text, const char** end) const -> const char*
{
    size_t len;
    if (unlikely(!text || !(len = utils::strlen(text)))) {
        this->error("No text to match");
        return nullptr;
    }

    const char* tend = text + len;
    const char* mend;
    auto nodes = this->regex.nodes;

    if (nodes->type == REGEX_BEGIN) {
        if ((mend = this->matchPattern(nodes + 1, text, tend))) {
            if (end)
                *end = mend;
            return text;
        }
        return nullptr;
    }

    do {
        if ((mend = this->matchPattern(nodes, text, tend))) {
            if (end)
                *end = mend;
            return text;
        }
    } while (tend > text++);

    return nullptr;
}

Regex::Regex(std::string const& pattern)
{
    if (unlikely(!this->compile(pattern.c_str())))
        throw std::exception();
}

Regex::Regex(const char* pattern)
{
    if (unlikely(!this->compile(pattern)))
        throw std::exception();
}
