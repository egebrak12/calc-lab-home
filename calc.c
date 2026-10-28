// Ege Burak Ergun 251ADB009
// GitHub repository: optional; add real repository URL if desired
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: ChatGPT (GPT-6), chat history: https://chatgpt.com/share/6ae15220-486c-83ed-8180-586684b43b93
// The following are actual user prompts in this conversation, copied verbatim.
// P4: Tamam, o zaman Grade 4'ten başlayalım. Codespaces zaten hazır, calc.c dosyam da boş. Grade 4 için hocanın istediği bütün gereksinimleri karşılayan kodu hazırla. Kodun tamamını tek seferde ver, direkt calc.c içine yapıştırayım. Sonra testlerini birlikte yapalım. Ayrıca kodun önemli kısımlarını da basitçe açıkla ki nasıl çalıştığını anlayayım.
// P7: grade 7 ye kadar yapsak olur mu ? bana anlatsan önemli kısımları kodda, yani sen bana bitmiş kodu ver 7 ye kadar olan kısmı ve önemli yerleri ve değişiklikleri anlat ki bende anlıyım
// P10: 10 a kadar yapabilir misin bu sefer ama açıklamalar biraz daha detaylı olsun ayrıca en sonda sana sorularım olucak
// PAUDIT: Kodumuz bütün örnek testleri geçti. Ama teslimden önce iki şey fark ettim. Hoca sadece belirli kütüphanelere izin vermiş, biz ise limits.h ve stdint.h kullanmışız. Bunlar kurallara uygun mu? Bir de AI kullandığımız yerlerdeki yorumlar hocanın istediği prompt ve kaynak gösterme şartlarını karşılıyor mu? İkisini de kontrol edip gerekiyorsa düzeltelim. Gerçekte sormadığım promptları eklemeyelim.
// AI comments below identify the prompt(s) relevant to each assisted code block.

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define FIRST_NAME "ege"
#define LAST_NAME "ergun"
#define STUDENT_ID "251ADB009"

// AI used: ChatGPT (GPT-6); exact actual user prompt:
// PAUDIT: Kodumuz bütün örnek testleri geçti. Ama teslimden önce iki şey fark ettim. Hoca sadece belirli kütüphanelere izin vermiş, biz ise limits.h ve stdint.h kullanmışız. Bunlar kurallara uygun mu? Bir de AI kullandığımız yerlerdeki yorumlar hocanın istediği prompt ve kaynak gösterme şartlarını karşılıyor mu? İkisini de kontrol edip gerekiyorsa düzeltelim. Gerçekte sormadığım promptları eklemeyelim.
// On Codespaces GCC/Linux, long long is a 64-bit two's-complement type,
// and unsigned long long has the same storage width (one extra value bit).
// Therefore the following replace limits.h without removing overflow checks.
#define MAX_LONG_LONG ((long long)(~0ULL >> 1))
#define MIN_LONG_LONG (-MAX_LONG_LONG - 1LL)
// size_t is unsigned; casting -1 to it yields its largest value.
#define MAX_SIZE_T ((size_t)-1)

// AI used: ChatGPT (GPT-6); exact actual user prompt:
// P10: 10 a kadar yapabilir misin bu sefer ama açıklamalar biraz daha detaylı olsun ayrıca en sonda sana sorularım olucak
// Keep both integer and floating-point forms.
// This preserves integer division (7/2 = 3) but promotes mixed operations to double.
typedef struct {
    long long integer;
    double floating;
    int is_float;
} Value;

static Value integer_value(long long x)
{
    Value value = {x, (double)x, 0};
    return value;
}

static Value float_value(double x)
{
    Value value = {0, x, 1};
    return value;
}

static double as_double(Value x)
{
    return x.is_float ? x.floating : (double)x.integer;
}

// AI used: ChatGPT (GPT-6); exact actual user prompt:
// P4: Tamam, o zaman Grade 4'ten başlayalım. Codespaces zaten hazır, calc.c dosyam da boş. Grade 4 için hocanın istediği bütün gereksinimleri karşılayan kodu hazırla. Kodun tamamını tek seferde ver, direkt calc.c içine yapıştırayım. Sonra testlerini birlikte yapalım. Ayrıca kodun önemli kısımlarını da basitçe açıkla ki nasıl çalıştığını anlayayım.
// Double the buffer instead of reallocating
// for every character. This handles lines longer than a fixed-size array.
static char *read_line(FILE *input, int *has_newline, size_t *length)
{
    size_t capacity = 256;
    size_t used = 0;
    char *buffer = malloc(capacity);
    int ch = EOF;

    if (buffer == NULL) return NULL;
    *has_newline = 0;
    while ((ch = fgetc(input)) != EOF) {
        if (ch == '\n') {
            *has_newline = 1;
            break;
        }
        if (used + 1 >= capacity) {
            if (capacity > MAX_SIZE_T / 2) {
                free(buffer);
                return NULL;
            }
            size_t next_capacity = capacity * 2;
            char *larger = realloc(buffer, next_capacity);
            if (larger == NULL) {
                free(buffer);
                return NULL;
            }
            buffer = larger;
            capacity = next_capacity;
        }
        buffer[used++] = (char)ch;
    }
    if (ch == EOF && used == 0) {
        free(buffer);
        return NULL;
    }
    buffer[used] = '\0';
    *length = used;
    return buffer;
}

typedef struct {
    const char *text;
    size_t position;
    size_t error;
    unsigned int depth;
} Parser;

static void skip_whitespace(Parser *p)
{
    while (p->text[p->position] == ' ' || p->text[p->position] == '\t' ||
           p->text[p->position] == '\r') {
        p->position++;
    }
}

static int parse_expression(Parser *p, Value *out);
static int parse_power(Parser *p, Value *out);

// AI used: ChatGPT (GPT-6); exact actual user prompts:
// P7: grade 7 ye kadar yapsak olur mu ? bana anlatsan önemli kısımları kodda, yani sen bana bitmiş kodu ver 7 ye kadar olan kısmı ve önemli yerleri ve değişiklikleri anlat ki bende anlıyım
// P10: 10 a kadar yapabilir misin bu sefer ama açıklamalar biraz daha detaylı olsun ayrıca en sonda sana sorularım olucak
// Parse a number or a parenthesised expression.
// A recursive call gives parentheses their higher priority and supports nesting.
static int parse_primary(Parser *p, Value *out)
{
    skip_whitespace(p);
    size_t start = p->position;

    if (p->text[start] == '(') {
        if (p->depth >= 1500) {
            p->error = start + 1;
            return 0;
        }
        p->position++;
        p->depth++;
        int ok = parse_expression(p, out);
        p->depth--;
        if (!ok) return 0;
        skip_whitespace(p);
        if (p->text[p->position] != ')') {
            p->error = p->position + 1;
            return 0;
        }
        p->position++;
        return 1;
    }

    if (!isdigit((unsigned char)p->text[start])) {
        p->error = start + 1;
        return 0;  // Unary signs and numbers starting with '.' are not allowed.
    }

    size_t index = start;
    while (isdigit((unsigned char)p->text[index])) index++;
    int is_float = 0;
    if (p->text[index] == '.' && isdigit((unsigned char)p->text[index + 1])) {
        is_float = 1;
        index++;
        while (isdigit((unsigned char)p->text[index])) index++;
    }

    errno = 0;
    char *end;
    if (is_float) {
        double number = strtod(p->text + start, &end);
        if ((size_t)(end - p->text) != index) {
            p->error = index + 1;  // E.g. 1.5e3 is not allowed by this grammar.
            return 0;
        }
        if (!isfinite(number)) {
            p->error = start + 1;
            return 0;
        }
        *out = float_value(number);
    } else {
        long long number = strtoll(p->text + start, &end, 10);
        if (errno == ERANGE || (size_t)(end - p->text) != index) {
            p->error = (errno == ERANGE ? start : index) + 1;
            return 0;
        }
        *out = integer_value(number);
    }
    p->position = index;
    return 1;
}

// AI used: ChatGPT (GPT-6); exact actual user prompt:
// P10: 10 a kadar yapabilir misin bu sefer ama açıklamalar biraz daha detaylı olsun ayrıca en sonda sana sorularım olucak
// Overflow-safe integer arithmetic, using only standard C17 operations.
static int int_add(long long a, long long b, long long *out)
{
    if ((b > 0 && a > MAX_LONG_LONG - b) ||
        (b < 0 && a < MIN_LONG_LONG - b)) return 0;
    *out = a + b;
    return 1;
}

static int int_sub(long long a, long long b, long long *out)
{
    if ((b < 0 && a > MAX_LONG_LONG + b) ||
        (b > 0 && a < MIN_LONG_LONG + b)) return 0;
    *out = a - b;
    return 1;
}

static int int_mul(long long a, long long b, long long *out)
{
    if (a > 0) {
        if (b > 0 && a > MAX_LONG_LONG / b) return 0;
        if (b < 0 && b < MIN_LONG_LONG / a) return 0;
    } else if (a < 0) {
        if (b > 0 && a < MIN_LONG_LONG / b) return 0;
        if (b < 0 && a < MAX_LONG_LONG / b) return 0;
    }
    *out = a * b;
    return 1;
}

// AI used: ChatGPT (GPT-6); exact actual user prompt:
// P10: 10 a kadar yapabilir misin bu sefer ama açıklamalar biraz daha detaylı olsun ayrıca en sonda sana sorularım olucak
// Integer exponentiation by squaring.
// It calculates powers in O(log exponent) multiplications and checks overflow.
static int int_power(long long base, long long exponent, long long *out)
{
    long long result = 1;
    while (exponent > 0) {
        if (exponent & 1) {
            if (!int_mul(result, base, &result)) return 0;
        }
        exponent /= 2;
        if (exponent > 0 && !int_mul(base, base, &base)) return 0;
    }
    *out = result;
    return 1;
}

// AI used: ChatGPT (GPT-6); exact actual user prompt:
// P10: 10 a kadar yapabilir misin bu sefer ama açıklamalar biraz daha detaylı olsun ayrıca en sonda sana sorularım olucak
// Power calls itself on its RIGHT side;
// therefore 2**3**2 means 2**(3**2), not (2**3)**2.
static int parse_power(Parser *p, Value *out)
{
    if (!parse_primary(p, out)) return 0;
    skip_whitespace(p);
    if (p->text[p->position] != '*' || p->text[p->position + 1] != '*')
        return 1;

    size_t operator_start = p->position;
    p->position += 2;
    skip_whitespace(p);
    size_t exponent_start = p->position;
    Value exponent;
    if (!parse_power(p, &exponent)) return 0;

    if (!out->is_float && !exponent.is_float) {
        if (exponent.integer < 0) {
            p->error = exponent_start + 1;
            return 0;
        }
        long long answer;
        if (!int_power(out->integer, exponent.integer, &answer)) {
            p->error = operator_start + 1;
            return 0;
        }
        *out = integer_value(answer);
    } else {
        errno = 0;
        double answer = pow(as_double(*out), as_double(exponent));
        if (!isfinite(answer)) {
            p->error = operator_start + 1;
            return 0;
        }
        *out = float_value(answer);
    }
    return 1;
}

// AI used: ChatGPT (GPT-6); exact actual user prompts:
// P7: grade 7 ye kadar yapsak olur mu ? bana anlatsan önemli kısımları kodda, yani sen bana bitmiş kodu ver 7 ye kadar olan kısmı ve önemli yerleri ve değişiklikleri anlat ki bende anlıyım
// P10: 10 a kadar yapabilir misin bu sefer ama açıklamalar biraz daha detaylı olsun ayrıca en sonda sana sorularım olucak
// Parse * and / from left to right.
// Testing for '**' is necessary because it belongs to parse_power, not here.
static int parse_term(Parser *p, Value *out)
{
    if (!parse_power(p, out)) return 0;
    for (;;) {
        skip_whitespace(p);
        size_t operator_start = p->position;
        char op = p->text[p->position];
        if ((op != '*' && op != '/') ||
            (op == '*' && p->text[p->position + 1] == '*')) return 1;

        p->position++;
        skip_whitespace(p);
        size_t right_start = p->position;
        Value right;
        if (!parse_power(p, &right)) return 0;

        if (op == '/' && as_double(right) == 0.0) {
            p->error = right_start + 1;  // Report right operand start, not '/'.
            return 0;
        }

        if (!out->is_float && !right.is_float) {
            long long answer;
            if (op == '/') {
                if (out->integer == MIN_LONG_LONG && right.integer == -1) {
                    p->error = operator_start + 1;
                    return 0;
                }
                answer = out->integer / right.integer;  // C17 truncates toward zero.
            } else if (!int_mul(out->integer, right.integer, &answer)) {
                p->error = operator_start + 1;
                return 0;
            }
            *out = integer_value(answer);
        } else {
            double answer = (op == '*')
                ? as_double(*out) * as_double(right)
                : as_double(*out) / as_double(right);
            if (!isfinite(answer)) {
                p->error = operator_start + 1;
                return 0;
            }
            *out = float_value(answer);
        }
    }
}

// AI used: ChatGPT (GPT-6); exact actual user prompts:
// P7: grade 7 ye kadar yapsak olur mu ? bana anlatsan önemli kısımları kodda, yani sen bana bitmiş kodu ver 7 ye kadar olan kısmı ve önemli yerleri ve değişiklikleri anlat ki bende anlıyım
// P10: 10 a kadar yapabilir misin bu sefer ama açıklamalar biraz daha detaylı olsun ayrıca en sonda sana sorularım olucak
// + and - work on terms, so * and /
// have higher precedence. A loop gives + and - left associativity.
static int parse_expression(Parser *p, Value *out)
{
    if (!parse_term(p, out)) return 0;
    for (;;) {
        skip_whitespace(p);
        size_t operator_start = p->position;
        char op = p->text[p->position];
        if (op != '+' && op != '-') return 1;

        p->position++;
        Value right;
        if (!parse_term(p, &right)) return 0;
        if (!out->is_float && !right.is_float) {
            long long answer;
            int ok = (op == '+')
                ? int_add(out->integer, right.integer, &answer)
                : int_sub(out->integer, right.integer, &answer);
            if (!ok) {
                p->error = operator_start + 1;
                return 0;
            }
            *out = integer_value(answer);
        } else {
            double answer = (op == '+')
                ? as_double(*out) + as_double(right)
                : as_double(*out) - as_double(right);
            if (!isfinite(answer)) {
                p->error = operator_start + 1;
                return 0;
            }
            *out = float_value(answer);
        }
    }
}

// AI used: ChatGPT (GPT-6); exact actual user prompts:
// P7: grade 7 ye kadar yapsak olur mu ? bana anlatsan önemli kısımları kodda, yani sen bana bitmiş kodu ver 7 ye kadar olan kısmı ve önemli yerleri ve değişiklikleri anlat ki bende anlıyım
// P10: 10 a kadar yapabilir misin bu sefer ama açıklamalar biraz daha detaylı olsun ayrıca en sonda sana sorularım olucak
// 1 = valid expression, 0 = blank/comment, -1 = invalid expression.
static int evaluate_line(const char *text, Value *out, size_t *error_column)
{
    Parser p = {text, 0, 0, 0};
    skip_whitespace(&p);
    if (text[p.position] == '\0' || text[p.position] == '#') return 0;

    if (!parse_expression(&p, out)) {
        *error_column = p.error;
        return -1;
    }
    skip_whitespace(&p);
    if (text[p.position] != '\0') {
        *error_column = p.position + 1;
        return -1;
    }
    return 1;
}

// AI used: ChatGPT (GPT-6); exact actual user prompt:
// P4: Tamam, o zaman Grade 4'ten başlayalım. Codespaces zaten hazır, calc.c dosyam da boş. Grade 4 için hocanın istediği bütün gereksinimleri karşılayan kodu hazırla. Kodun tamamını tek seferde ver, direkt calc.c içine yapıştırayım. Sonra testlerini birlikte yapalım. Ayrıca kodun önemli kısımlarını da basitçe açıkla ki nasıl çalıştığını anlayayım.
// Read the file twice. Validate every
// line before writing anything so the output is only ERROR:<pos> on an error.
// Count original characters, including comment text and newlines, for positions.
static int process_file(FILE *input, FILE *output, size_t *error_position)
{
    size_t before = 0;
    int had_newline;
    size_t length;
    char *line;

    while ((line = read_line(input, &had_newline, &length)) != NULL) {
        Value value;
        size_t column = 0;
        int status = evaluate_line(line, &value, &column);
        free(line);

        if (status == -1) {
            *error_position = before + column;
            return 1;
        }
        if (status == 1 && output != NULL) {
            int written = value.is_float
                ? fprintf(output, "%.15g\n", value.floating)
                : fprintf(output, "%lld\n", value.integer);
            if (written < 0) return -1;
        }
        if (before > MAX_SIZE_T - length - (size_t)had_newline) return -1;
        before += length + (size_t)had_newline;
    }
    return feof(input) ? 0 : -1;
}

// AI used: ChatGPT (GPT-6); exact actual user prompt:
// P10: 10 a kadar yapabilir misin bu sefer ama açıklamalar biraz daha detaylı olsun ayrıca en sonda sana sorularım olucak
// Create paths dynamically, not using a
// small fixed array that might overflow on long filenames.
static char *join_path(const char *dir, const char *name)
{
    size_t a = strlen(dir), b = strlen(name);
    if (a > MAX_SIZE_T - b - 2) return NULL;
    char *path = malloc(a + b + 2);
    if (path == NULL) return NULL;
    memcpy(path, dir, a);
    path[a] = '/';
    memcpy(path + a + 1, name, b + 1);
    return path;
}

static char *formatted_result_name(const char *name)
{
    size_t size = strlen(name) + strlen(FIRST_NAME) + strlen(LAST_NAME)
        + strlen(STUDENT_ID) + 8;
    char *result = malloc(size);
    if (result != NULL)
        snprintf(result, size, "%s_%s_%s_%s.txt", name,
                 FIRST_NAME, LAST_NAME, STUDENT_ID);
    return result;
}

// AI used: ChatGPT (GPT-6); exact actual user prompt:
// P10: 10 a kadar yapabilir misin bu sefer ama açıklamalar biraz daha detaylı olsun ayrıca en sonda sana sorularım olucak
// Get file stem (name without .txt) or directory basename (without ending /).
static char *base_name(const char *path, int is_directory)
{
    size_t path_length = strlen(path);
    while (is_directory && path_length > 0 && path[path_length - 1] == '/')
        path_length--;
    size_t start = path_length;
    while (start > 0 && path[start - 1] != '/') start--;
    size_t length = path_length - start;
    if (!is_directory && length >= 4 &&
        strncmp(path + path_length - 4, ".txt", 4) == 0) length -= 4;
    char *name = malloc(length + 1);
    if (name == NULL) return NULL;
    memcpy(name, path + start, length);
    name[length] = '\0';
    return name;
}

static char *default_output_dir(const char *input, int directory_mode)
{
    char *base = base_name(input, directory_mode);
    if (base == NULL) return NULL;
    size_t size = strlen(base) + strlen(FIRST_NAME) + strlen(STUDENT_ID) + 3;
    char *dir = malloc(size);
    if (dir != NULL)
        snprintf(dir, size, "%s_%s_%s", base, FIRST_NAME, STUDENT_ID);
    free(base);
    return dir;
}

static int ensure_directory(const char *path)
{
    if (mkdir(path, 0777) == 0) return 1;
    if (errno == EEXIST) {
        struct stat info;
        return stat(path, &info) == 0 && S_ISDIR(info.st_mode);
    }
    return 0;
}

// AI used: ChatGPT (GPT-6); exact actual user prompt:
// P10: 10 a kadar yapabilir misin bu sefer ama açıklamalar biraz daha detaylı olsun ayrıca en sonda sana sorularım olucak
// Read input once to check for errors; write the results on the second pass.
static int process_named_file(const char *input_path, const char *output_dir)
{
    FILE *input = fopen(input_path, "r");
    if (input == NULL) {
        fprintf(stderr, "Cannot open input %s: %s\n", input_path, strerror(errno));
        return 0;
    }

    char *base = base_name(input_path, 0);
    char *filename = base == NULL ? NULL : formatted_result_name(base);
    char *output_path = filename == NULL ? NULL : join_path(output_dir, filename);
    free(base);
    free(filename);
    if (output_path == NULL) {
        fprintf(stderr, "Cannot allocate output filename\n");
        fclose(input);
        return 0;
    }

    size_t error_position = 0;
    int status = process_file(input, NULL, &error_position);
    if (status < 0) {
        fprintf(stderr, "Cannot read input %s\n", input_path);
        free(output_path);
        fclose(input);
        return 0;
    }

    FILE *output = fopen(output_path, "w");
    if (output == NULL) {
        fprintf(stderr, "Cannot write output %s: %s\n", output_path, strerror(errno));
        free(output_path);
        fclose(input);
        return 0;
    }
    if (status == 1) {
        if (fprintf(output, "ERROR:%zu\n", error_position) < 0) status = -1;
        else status = 0;
    } else {
        if (fseek(input, 0, SEEK_SET) != 0) status = -1;
        else status = process_file(input, output, &error_position);
    }
    int write_error = status != 0 || ferror(output);
    if (fclose(output) != 0) write_error = 1;
    if (fclose(input) != 0) write_error = 1;
    if (write_error) fprintf(stderr, "Cannot finish processing %s\n", input_path);
    free(output_path);
    return !write_error;
}

static int has_txt_extension(const char *name)
{
    size_t length = strlen(name);
    return length >= 4 && strcmp(name + length - 4, ".txt") == 0;
}

// AI used: ChatGPT (GPT-6); exact actual user prompt:
// P10: 10 a kadar yapabilir misin bu sefer ama açıklamalar biraz daha detaylı olsun ayrıca en sonda sana sorularım olucak
// Read only regular *.txt files directly
// inside the requested directory; ignore nested folders and other extensions.
static int process_directory(const char *input_dir, const char *output_dir)
{
    DIR *dir = opendir(input_dir);
    if (dir == NULL) {
        fprintf(stderr, "Cannot open directory %s: %s\n", input_dir, strerror(errno));
        return 0;
    }
    int ok = 1;
    struct dirent *entry;
    errno = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (!has_txt_extension(entry->d_name)) continue;
        char *path = join_path(input_dir, entry->d_name);
        if (path == NULL) { ok = 0; break; }
        struct stat info;
        if (stat(path, &info) != 0) {
            fprintf(stderr, "Cannot inspect %s\n", path);
            ok = 0;
        } else if (S_ISREG(info.st_mode) && !process_named_file(path, output_dir)) {
            ok = 0;
        }
        free(path);
        if (!ok) break;
        errno = 0;
    }
    if (errno != 0) {
        fprintf(stderr, "Cannot read directory entries in %s\n", input_dir);
        ok = 0;
    }
    if (closedir(dir) != 0) ok = 0;
    return ok;
}

static void usage(const char *program)
{
    fprintf(stderr, "Usage: %s [-d DIR|--dir DIR] [-o OUTDIR|--output-dir OUTDIR] [input.txt]\n", program);
}

// AI used: ChatGPT (GPT-6); exact actual user prompt:
// P10: 10 a kadar yapabilir misin bu sefer ama açıklamalar biraz daha detaylı olsun ayrıca en sonda sana sorularım olucak
// Parse switches in either order, and
// use the documented default output folder when -o is omitted.
int main(int argc, char *argv[])
{
    const char *directory = NULL;
    const char *outdir = NULL;
    const char *input_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dir") == 0) {
            if (directory != NULL || i + 1 >= argc || argv[i + 1][0] == '-') {
                usage(argv[0]);
                return 1;
            }
            directory = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output-dir") == 0) {
            if (outdir != NULL || i + 1 >= argc || argv[i + 1][0] == '-') {
                usage(argv[0]);
                return 1;
            }
            outdir = argv[++i];
        } else if (argv[i][0] == '-' || input_file != NULL) {
            usage(argv[0]);
            return 1;
        } else {
            input_file = argv[i];
        }
    }

    if ((directory != NULL && input_file != NULL) ||
        (directory == NULL && input_file == NULL)) {
        usage(argv[0]);
        return 1;
    }

    char *default_dir = NULL;
    if (outdir == NULL) {
        default_dir = default_output_dir(directory ? directory : input_file,
                                         directory != NULL);
        if (default_dir == NULL) {
            fprintf(stderr, "Cannot allocate output directory name\n");
            return 1;
        }
        outdir = default_dir;
    }

    // Avoid creating an output folder when the requested input does not exist.
    if (directory != NULL) {
        struct stat info;
        if (stat(directory, &info) != 0 || !S_ISDIR(info.st_mode)) {
            fprintf(stderr, "Input directory does not exist: %s\n", directory);
            free(default_dir);
            return 1;
        }
    } else {
        FILE *probe = fopen(input_file, "r");
        if (probe == NULL) {
            fprintf(stderr, "Cannot open input %s: %s\n", input_file, strerror(errno));
            free(default_dir);
            return 1;
        }
        fclose(probe);
    }

    if (!ensure_directory(outdir)) {
        fprintf(stderr, "Cannot create output directory %s: %s\n", outdir, strerror(errno));
        free(default_dir);
        return 1;
    }

    int ok = (directory != NULL)
        ? process_directory(directory, outdir)
        : process_named_file(input_file, outdir);
    free(default_dir);
    return ok ? 0 : 1;
}
