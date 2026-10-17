size_t str_length(const char* str) {
    const char* p = str;
    while (*p) {
        ++p;
    }
    return p - str;
}

bool minha_isprint(int caractere) {
    // Verificar se o caractere é imprimível
    return (caractere >= 32 && caractere <= 126);
}

#define HEAP_SIZE 10000

static char heap[HEAP_SIZE]; // Espaço de memória para alocação
static size_t heap_index = 0; // Índice atual na memória alocada


void *minha_malloc(size_t size) {
    if (heap_index + size > HEAP_SIZE) {
        return NULL; // Não há espaço suficiente para alocar
    }

    void *ptr = &heap[heap_index];
    heap_index += size;
    return ptr;
}

char *minha_fgets(char *line, int size, FILE *stream) {
    if (size <= 0 || line == NULL || stream == NULL) {
        return NULL; // Verifica argumentos inválidos
    }

    int i = 0;
    int c;

    // Lê caracteres do fluxo até atingir EOF ou atingir o tamanho máximo - 1
    while (i < size - 1 && (c = fgetc(stream)) != EOF) {
        line[i++] = (char)c;

        // Se encontrarmos um caractere de nova linha, paramos a leitura
        if (c == '\n') {
            break;
        }
    }

    // Adiciona terminador nulo
    line[i] = '\0';

    // Retorna ponteiro para a linha lida, ou NULL se não houver nada a ser lido
    return (i > 0 || c != EOF) ? line : NULL;
}


int minha_vsnprintf(char *str, size_t size, const char *format, va_list args) {
    int ret;
#ifdef _MSC_VER
    ret = _vsnprintf_s(str, size, size - 1, format, args);
    if (ret < 0)
        ret = _vscprintf(format, args);
#else
    ret = vsnprintf(str, size, format, args);
#endif
    return ret;
}


int minha_sprintf(char *buffer, const char *formato, ...) {
    va_list args;
    int caracteres_escritos;

    // Inicializa a lista de argumentos variáveis
    va_start(args, formato);

    // Formata a string e armazena no buffer
    caracteres_escritos = minha_vsnprintf(buffer,sizeof(buffer), formato, args);

    // Finaliza a lista de argumentos variáveis
    va_end(args);

    return caracteres_escritos;
}

char* minha_strcpy(char *destino, const char *origem) {
    char *inicio = destino;
    while (*origem != '\0') {
        *destino = *origem;
        destino++;
        origem++;
    }
    *destino = '\0'; // Adiciona o caractere nulo no final da string de destino
    return inicio; // Retorna um ponteiro para o início da string de destino
}

long int minha_strtol(const char *str, char **endptr, int base) {
    long int result = 0;
    bool negativo = false;

    // Ignorar espaços iniciais
    while (*str == ' ') {
        str++;
    }

    // Verificar sinal
    if (*str == '-') {
        negativo = true;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Converter o número
    while (*str != '\0') {
        // Verificar se é um dígito válido na base especificada
        int digito;
        if (*str >= '0' && *str <= '9') {
            digito = *str - '0';
        } else if (*str >= 'a' && *str <= 'z') {
            digito = *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'Z') {
            digito = *str - 'A' + 10;
        } else {
            break; // Não é um dígito válido
        }

        // Verificar se o dígito está na faixa da base
        if (digito >= base) {
            break; // Dígito inválido para a base
        }

        // Atualizar o resultado
        result = result * base + digito;
        str++;
    }

    // Definir endptr se necessário
    if (endptr != NULL) {
        *endptr = (char *)str;
    }

    // Retornar o resultado com sinal correto
    return (negativo ? -result : result);
}

char *minha_strstr(const char *haystack, const char *needle) {
    if (*needle == '\0') {
        return (char *)haystack; // Caso a substring seja uma string vazia
    }

    while (*haystack != '\0') {
        const char *h = haystack;
        const char *n = needle;

        // Enquanto os caracteres coincidirem e não atingirmos o final de 'needle'
        while (*h != '\0' && *n != '\0' && *h == *n) {
            h++;
            n++;
        }

        // Se 'needle' chegou ao final, encontramos uma correspondência
        if (*n == '\0') {
            return (char *)haystack;
        }

        haystack++; // Avança para o próximo caractere em 'haystack'
    }

    return NULL; // Não encontrou a substring
}

int minha_sscanf(const char *str, const char *format, ...) {
    va_list args;
    va_start(args, format);

    int num_matches = 0;
    const char *format_ptr = format;
    const char *str_ptr = str;

    while (*format_ptr != '\0') {
        if (*format_ptr == '%') {
            // Verifica se a próxima parte do formato é "%lx"
            if (*(format_ptr + 1) == 'l' && *(format_ptr + 2) == 'x') {
                // Lê o próximo valor hexadecimal de str
                char *endptr;
                unsigned long *ptr = va_arg(args, unsigned long *);
                *ptr = strtoul(str_ptr, &endptr, 16);
                if (endptr == str_ptr) {
                    // Não foi possível fazer a correspondência
                    va_end(args);
                    return num_matches;
                }
                num_matches++;
                str_ptr = endptr;
                format_ptr += 2; // Avança para o próximo caractere após 'x'
            }
        } else {
            // Verifica se os caracteres coincidem
            if (*format_ptr != *str_ptr) {
                va_end(args);
                return num_matches;
            }
            str_ptr++;
        }
        format_ptr++;
    }

    va_end(args);
    return num_matches;
}

int minha_sscanfHex(const char *str, const char *format, ...) {
    va_list args;
    const char *p_str = str;
    const char *p_format = format;
    int count = 0;

    va_start(args, format);

    while (*p_format && *p_str) {
        if (*p_format == '%' && *(p_format + 1) == '2' && *(p_format + 2) == 'h' && *(p_format + 3) == 'h' && *(p_format + 4) == 'X') {
            p_format += 5; // Pula o formato "%2hhX"
            unsigned char *ptr = va_arg(args, unsigned char *);

            // Lê dois caracteres hexadecimais e converte para um byte
            *ptr = 0;
            for (int i = 0; i < 2; i++) {
                char hex = *p_str++;
                if (hex >= '0' && hex <= '9')
                    *ptr = (*ptr << 4) | (hex - '0');
                else if (hex >= 'A' && hex <= 'F')
                    *ptr = (*ptr << 4) | (hex - 'A' + 10);
                else if (hex >= 'a' && hex <= 'f')
                    *ptr = (*ptr << 4) | (hex - 'a' + 10);
                else
                    break; // Caractere inválido
            }

            count++;
        } else {
            // Verifica se os caracteres correspondem
            if (*p_format != *p_str)
                break;
            p_format++;
            p_str++;
        }
    }

    va_end(args);

    return count;
}

int minha_sprintfHex(char *str, const char *format, ...) {
    va_list args;
    int ret = 0;

    va_start(args, format);

    while (*format) {
        if (*format == '%' && *(format + 1) == '0' && *(format + 2) == '2' && *(format + 3) == 'X') {
            format += 4; // Pula o formato "\\x%02X"
            unsigned char value = (unsigned char)va_arg(args, int);

            // Converte o byte para código hexadecimal manualmente
            str[ret++] = '\\';
            str[ret++] = 'x';
            str[ret++] = (value >> 4) < 10 ? (value >> 4) + '0' : (value >> 4) - 10 + 'A';
            str[ret++] = (value & 0x0F) < 10 ? (value & 0x0F) + '0' : (value & 0x0F) - 10 + 'A';
        } else {
            // Copia caracteres diretamente
            str[ret++] = *format;
            format++;
        }
    }

    va_end(args);

    // Adiciona o caractere nulo de término da string
    str[ret] = '\0';

    return ret;
}


static unsigned int g_seed = 0; // Variável global para armazenar a semente

void minha_srand(unsigned int seed) {
    g_seed = seed; // Define a semente global
}

unsigned int minha_rand() {
    g_seed = g_seed * 1103515245 + 12345; // Algoritmo linear congruente
    return (unsigned int)(g_seed / 65536) % 32768;
}


char* random_string(int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char* str = (char*)minha_malloc((length + 1) * sizeof(char));
    if (!str) return NULL;
    minha_srand(time(NULL));
    for (int i = 0; i < length; i++) {
        str[i] = charset[minha_rand() % (sizeof(charset) - 1)];
    }
    str[length] = '\0';
    return str;
}

char *obfuscate_char(const char *texto) {
    char alfabeto[] = "abcdefghijklmnopqrstuvwxyz";
    char *criptografado = static_cast<char *>(minha_malloc(str_length(texto) + 1));
    int i;
    for (i = 0; i < str_length(texto); i++) {
        if (texto[i] >= 'a' && texto[i] <= 'z') {
            criptografado[i] = alfabeto[(texto[i] - 'a' + 3) % 26];
        } else {
            criptografado[i] = texto[i];
        }
    }
    criptografado[str_length(texto)] = '\0';

    return criptografado;
}

char *restore_char(const char *texto) {
    char alfabeto[] = "abcdefghijklmnopqrstuvwxyz";
    char *descriptografado = static_cast<char *>(minha_malloc(str_length(texto) + 1));
    int i;
    for (i = 0; i < str_length(texto); i++) {
        if (texto[i] >= 'a' && texto[i] <= 'z') {
            descriptografado[i] = alfabeto[(texto[i] - 'a' - 3 + 26) % 26];
        } else {
            descriptografado[i] = texto[i];
        }
    }
    descriptografado[str_length(texto)] = '\0';
    return descriptografado;
}

char* string_to_hex(const char* input) {
    size_t len = str_length(input);
    char* output = (char*)minha_malloc(len * 2 + 1);
    for (size_t i = 0; i < len; ++i)
        minha_sprintf(output + i * 2, OBFUSCATE("%02X"), input[i]);
    return output;
}

char* hex_to_string(const char* input) {
    size_t len = str_length(input);
    if (len % 2 != 0) return NULL;
    size_t output_len = len / 2;
    char* output = (char*)minha_malloc(output_len + 1);
    for (size_t i = 0; i < output_len; ++i)
        minha_sscanfHex(input + i * 2, OBFUSCATE("%2hhX"), &output[i]);
    output[output_len] = '\0';
    return output;
}

char* obfuscate_string(const char* input) {
    size_t len = str_length(input);
    char* obfuscated = (char*)minha_malloc(len + 1);

    for (size_t i = 0; i < len; i++) {
        obfuscated[i] = input[(i + 8) % len];
    }
    obfuscated[len] = '\0';

    return obfuscated;
}

char* restore_string(const char* obfuscated) {
    size_t len = str_length(obfuscated);
    char* original = (char*)minha_malloc(len + 1);

    for (size_t i = 0; i < len; i++) {
        original[i] = obfuscated[(i - 8 + len) % len];
    }
    original[len] = '\0';
    return original;
}

void confuse_hex(char* hex) {
    size_t len = str_length(hex);
    char temp[len * 4 + 1];
    size_t j = 0;

    for (size_t i = 0; i < len; i++) {
        hex[i] = hex[i] ^ 0xFF;

        if (!minha_isprint(hex[i])) {
            minha_sprintfHex(&temp[j], OBFUSCATE("\\x%02X"), (unsigned char)hex[i]);
            j += 4;
        } else {
            temp[j++] = hex[i];
        }
    }

    temp[j] = '\0';
    minha_strcpy(hex, temp);
}


void unconfuse_hex(char* hex) {
    size_t len = str_length(hex);
    char temp[len + 1];
    size_t j = 0;

    for (size_t i = 0; i < len; i++) {
        if (hex[i] == '\\' && hex[i + 1] == 'x') {
            char hex_digit[3] = {hex[i + 2], hex[i + 3], '\0'};
            temp[j++] = minha_strtol(hex_digit, NULL, 16) ^ 0xFF;
            i += 3;
        } else {
            temp[j++] = hex[i];
        }
    }

    temp[j] = '\0';
    minha_strcpy(hex, temp);
}

char* ProcessSequence(const char* input) {
    char* TokenKey = string_to_hex(input);
    TokenKey = obfuscate_string(TokenKey);
    TokenKey = obfuscate_string(TokenKey);
    TokenKey = obfuscate_string(TokenKey);
    //confuse_hex(TokenKey);
    TokenKey = obfuscate_string(TokenKey);
    return TokenKey;
}

char* RestoreSequence(const char* input) {
    char* RestoreKey = restore_string(input);
    //unconfuse_hex(RestoreKey);
    RestoreKey = restore_string(RestoreKey);
    RestoreKey = restore_string(RestoreKey);
    RestoreKey = restore_string(RestoreKey);
    RestoreKey = hex_to_string(RestoreKey);
    return RestoreKey;
}

char* encrypt(const char* input) {
    char* TokenKey = string_to_hex(input);
    TokenKey = obfuscate_string(TokenKey);
    confuse_hex(TokenKey);
    TokenKey = obfuscate_string(TokenKey);
    TokenKey = string_to_hex(TokenKey);
    TokenKey = obfuscate_string(TokenKey);
    return TokenKey;
}

char* decrypt(const char* input) {
    char* RestoreKey = restore_string(input);
    RestoreKey = hex_to_string(RestoreKey);
    RestoreKey = restore_string(RestoreKey);
    unconfuse_hex(RestoreKey);
    RestoreKey = restore_string(RestoreKey);
    RestoreKey = hex_to_string(RestoreKey);
    return RestoreKey;
}

char* encryptEasy(const char* input) {
    char* TokenKey = string_to_hex(input);
    TokenKey = obfuscate_string(TokenKey);
    TokenKey = obfuscate_string(TokenKey);
    return TokenKey;
}

char* decryptEasy(const char* input) {
    char* RestoreKey = restore_string(input);
    RestoreKey = restore_string(RestoreKey);
    RestoreKey = hex_to_string(RestoreKey);
    return RestoreKey;
}

char* rs(const char* input) {
    char* String = obfuscate_string(input);
    return String;
}

bool constant_time_compare(const char* str1, const char* str2, size_t len) {
    unsigned char result = 0;
    for (size_t i = 0; i < len; ++i) {
        result |= str1[i] ^ str2[i];
    }
    return result == 0;
}

int stringToInt(const std::string& str) {
    return std::stoi(str);
}