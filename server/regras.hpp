// regras.hpp
// Validações e regras de negócio do Faz Ai, portadas do faz_ai.cpp original
// (mesmas funções: emailValido, cpfValido, telefoneValido, dataValida, valorValido)
#pragma once

#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <cctype>
#include <openssl/evp.h>
#include <iomanip>

namespace fazai {

inline const std::vector<std::string>& profissoes()
{
    static const std::vector<std::string> PROFISSOES = {
        "Eletricista", "Encanador", "Pedreiro", "Pintor", "Carpinteiro",
        "Marceneiro", "Serralheiro", "Azulejista", "Gesseiro", "Jardineiro",
        "Diarista", "Faxineiro", "Cozinheiro", "Confeiteiro", "Garcom",
        "Fotografo", "Videomaker", "Designer Grafico", "Desenvolvedor de Software",
        "Tecnico de Informatica", "Tecnico de Celular", "Tecnico de Computadores",
        "Instalador de Ar-Condicionado", "Tecnico de Refrigeracao", "Mecanico",
        "Eletricista Automotivo", "Lavador de Carros", "Funileiro", "Motoboy",
        "Motorista", "Professor Particular", "Tradutor", "Redator", "Social Media",
        "Cabeleireiro", "Barbeiro", "Manicure", "Maquiador", "Fotografo de Eventos",
        "Personal Trainer", "Professor de Musica", "Professor de Idiomas",
        "Costureiro", "Decorador", "Cerimonialista", "Cuidador de Idosos",
        "Baba", "Pet Sitter", "Adestrador de Caes", "Tecnico de Seguranca Eletronica"
    };
    return PROFISSOES;
}

// Unidades da federação: {sigla, nome}. O Faz Ai atende o Brasil inteiro.
inline const std::vector<std::pair<std::string, std::string>>& estados()
{
    static const std::vector<std::pair<std::string, std::string>> ESTADOS = {
        {"AC", "Acre"}, {"AL", "Alagoas"}, {"AP", "Amapá"}, {"AM", "Amazonas"},
        {"BA", "Bahia"}, {"CE", "Ceará"}, {"DF", "Distrito Federal"}, {"ES", "Espírito Santo"},
        {"GO", "Goiás"}, {"MA", "Maranhão"}, {"MT", "Mato Grosso"}, {"MS", "Mato Grosso do Sul"},
        {"MG", "Minas Gerais"}, {"PA", "Pará"}, {"PB", "Paraíba"}, {"PR", "Paraná"},
        {"PE", "Pernambuco"}, {"PI", "Piauí"}, {"RJ", "Rio de Janeiro"}, {"RN", "Rio Grande do Norte"},
        {"RS", "Rio Grande do Sul"}, {"RO", "Rondônia"}, {"RR", "Roraima"}, {"SC", "Santa Catarina"},
        {"SP", "São Paulo"}, {"SE", "Sergipe"}, {"TO", "Tocantins"}
    };
    return ESTADOS;
}

inline std::string paraMaiusculo(std::string texto)
{
    for (char& c : texto)
        c = (char)toupper((unsigned char)c);
    return texto;
}

inline bool estadoValido(const std::string& sigla)
{
    for (auto& e : estados())
        if (e.first == sigla)
            return true;
    return false;
}

inline std::string apenasDigitos(const std::string& texto)
{
    std::string digitos;
    for (char c : texto)
        if (isdigit((unsigned char)c))
            digitos += c;
    return digitos;
}

inline std::string paraMinusculo(std::string texto)
{
    for (char& c : texto)
        c = (char)tolower((unsigned char)c);
    return texto;
}

inline bool emailValido(const std::string& email)
{
    auto arroba = email.find('@');
    if (arroba == std::string::npos || arroba == 0)
        return false;
    if (email.find('@', arroba + 1) != std::string::npos)
        return false;
    auto ponto = email.find('.', arroba);
    if (ponto == std::string::npos || ponto <= arroba + 1 || ponto == email.size() - 1)
        return false;
    return true;
}

inline bool telefoneValido(const std::string& telefone)
{
    for (char c : telefone)
        if (!isdigit((unsigned char)c) && c != ' ' && c != '(' && c != ')' && c != '-')
            return false;
    std::string digitos = apenasDigitos(telefone);
    return digitos.size() == 10 || digitos.size() == 11;
}

inline bool cpfValido(const std::string& cpfDigitado)
{
    for (char c : cpfDigitado)
        if (!isdigit((unsigned char)c) && c != '.' && c != '-')
            return false;

    std::string cpf = apenasDigitos(cpfDigitado);
    if (cpf.size() != 11)
        return false;

    bool todosIguais = true;
    for (int i = 1; i < 11; i++)
        if (cpf[i] != cpf[0])
            todosIguais = false;
    if (todosIguais)
        return false;

    int soma = 0;
    for (int i = 0; i < 9; i++)
        soma += (cpf[i] - '0') * (10 - i);
    int d1 = (soma % 11 < 2) ? 0 : 11 - (soma % 11);
    if (d1 != cpf[9] - '0')
        return false;

    soma = 0;
    for (int i = 0; i < 10; i++)
        soma += (cpf[i] - '0') * (11 - i);
    int d2 = (soma % 11 < 2) ? 0 : 11 - (soma % 11);
    if (d2 != cpf[10] - '0')
        return false;

    return true;
}

inline bool anoBissexto(int ano)
{
    return (ano % 4 == 0 && ano % 100 != 0) || ano % 400 == 0;
}

// Espera data no formato DD/MM/AAAA (mesmo formato usado na interface do console original)
inline bool dataValida(const std::string& data)
{
    if (data.size() != 10 || data[2] != '/' || data[5] != '/')
        return false;
    for (int i = 0; i < 10; i++)
        if (i != 2 && i != 5 && !isdigit((unsigned char)data[i]))
            return false;

    int dia = std::stoi(data.substr(0, 2));
    int mes = std::stoi(data.substr(3, 2));
    int ano = std::stoi(data.substr(6, 4));

    if (ano < 1900 || ano > 2026 || mes < 1 || mes > 12)
        return false;

    int diasNoMes[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int limite = diasNoMes[mes - 1];
    if (mes == 2 && anoBissexto(ano))
        limite = 29;

    return dia >= 1 && dia <= limite;
}

// Converte DD/MM/AAAA -> AAAA-MM-DD (formato DATE do PostgreSQL)
inline std::string dataParaISO(const std::string& data)
{
    return data.substr(6, 4) + "-" + data.substr(3, 2) + "-" + data.substr(0, 2);
}

inline bool valorValido(const std::string& valor)
{
    if (valor.empty())
        return false;
    bool temNumero = false;
    int separadores = 0;
    for (char c : valor)
    {
        if (isdigit((unsigned char)c)) temNumero = true;
        else if (c == '.' || c == ',') separadores++;
        else return false;
    }
    return temNumero && separadores <= 1;
}

// Converte "300,00" ou "300.00" -> 300.00 (double), para gravar em NUMERIC
inline double valorParaNumero(std::string valor)
{
    for (char& c : valor) if (c == ',') c = '.';
    try { return std::stod(valor); } catch (...) { return 0.0; }
}

// SHA-256(senha + salt) em hexadecimal - usado para não gravar senha em texto puro.
// Observação: para produção, recomenda-se usar bcrypt/argon2 em vez de SHA-256 simples.
inline std::string hashSenha(const std::string& senha)
{
    static const std::string SALT = "faz_ai_salt_2026";
    std::string entrada = senha + SALT;

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int tamanho = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, entrada.c_str(), entrada.size());
    EVP_DigestFinal_ex(ctx, digest, &tamanho);
    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;
    for (unsigned int i = 0; i < tamanho; i++)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    return oss.str();
}

} // namespace fazai
