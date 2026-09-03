// server.cpp - Faz Ai (versão web)
// Servidor HTTP em C++ (cpp-httplib) + PostgreSQL (libpqxx)
// Reaproveita as regras de negócio do programa de console original (regras.hpp)

#include "httplib.h"
#include "regras.hpp"
#include <pqxx/pqxx>
#include <nlohmann/json.hpp>
#include <iostream>
#include <cstdlib>
#include <mutex>

using json = nlohmann::json;
using namespace fazai;

static std::string connString()
{
    const char* env = std::getenv("DATABASE_URL");
    if (env && *env) return env;
    return "dbname=fazai_db user=fazai password=fazai_dev_pw host=127.0.0.1 port=5432";
}

// pqxx::connection não é thread-safe para uso concorrente sem cuidado;
// para manter o protótipo simples, usamos um mutex global protegendo o acesso ao banco.
static std::mutex dbMutex;

static json erro(const std::string& msg)
{
    return json{{"ok", false}, {"erro", msg}};
}

int main(int argc, char** argv)
{
    int porta = 8080;
    if (const char* p = std::getenv("PORT")) porta = std::atoi(p);

    httplib::Server svc;

    // Serve o frontend (HTML/CSS/JS) a partir da pasta ../web
    auto ret = svc.set_mount_point("/", "./web");
    if (!ret) std::cerr << "Aviso: pasta 'web' não encontrada ao lado do executável.\n";

    svc.set_default_headers({
        {"Access-Control-Allow-Origin", "*"}
    });

    // -------- GET /api/profissoes --------
    svc.Get("/api/profissoes", [](const httplib::Request&, httplib::Response& res) {
        json lista = json::array();
        for (auto& p : profissoes()) lista.push_back(p);
        res.set_content(json{{"ok", true}, {"profissoes", lista}}.dump(), "application/json");
    });

    // -------- POST /api/cadastro --------
    svc.Post("/api/cadastro", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(dbMutex);
        try {
            json body = json::parse(req.body);
            std::string nome  = body.value("nome", "");
            std::string email = body.value("email", "");
            std::string senha = body.value("senha", "");
            std::string tipo  = body.value("tipo", "");

            if (nome.empty()) { res.set_content(erro("O nome não pode ficar vazio.").dump(), "application/json"); return; }
            if (!emailValido(email)) { res.set_content(erro("E-mail inválido.").dump(), "application/json"); return; }
            if (senha.size() < 4) { res.set_content(erro("Senha muito curta (mínimo 4 caracteres).").dump(), "application/json"); return; }
            if (tipo != "cliente" && tipo != "profissional") { res.set_content(erro("Tipo de conta inválido.").dump(), "application/json"); return; }

            pqxx::connection conn(connString());
            pqxx::work tx(conn);

            std::string emailMin = paraMinusculo(email);
            auto existe = tx.exec_params("SELECT 1 FROM usuarios WHERE lower(email) = $1", emailMin);
            if (!existe.empty()) { res.set_content(erro("Este e-mail já está cadastrado.").dump(), "application/json"); return; }

            std::string hash = hashSenha(senha);
            auto rInsUser = tx.exec_params(
                "INSERT INTO usuarios (nome, email, senha_hash, tipo) VALUES ($1,$2,$3,$4) RETURNING id",
                nome, email, hash, tipo);
            long long usuarioId = rInsUser[0][0].as<long long>();

            if (tipo == "profissional") {
                std::string cpf = body.value("cpf", "");
                std::string dataNasc = body.value("data_nascimento", "");
                std::string profissao = body.value("profissao", "");
                std::string descricao = body.value("descricao", "");
                std::string cidade = body.value("cidade", "");
                std::string telefone = body.value("telefone", "");
                std::string valorTxt = body.value("valor", "");

                bool profissaoValida = false;
                for (auto& p : profissoes()) if (p == profissao) profissaoValida = true;

                if (!cpfValido(cpf))          { tx.abort(); res.set_content(erro("CPF inválido.").dump(), "application/json"); return; }
                if (!dataValida(dataNasc))    { tx.abort(); res.set_content(erro("Data de nascimento inválida.").dump(), "application/json"); return; }
                if (!profissaoValida)         { tx.abort(); res.set_content(erro("Profissão inválida.").dump(), "application/json"); return; }
                if (descricao.empty())        { tx.abort(); res.set_content(erro("A descrição não pode ficar vazia.").dump(), "application/json"); return; }
                if (cidade.empty())            { tx.abort(); res.set_content(erro("A cidade não pode ficar vazia.").dump(), "application/json"); return; }
                if (!telefoneValido(telefone)) { tx.abort(); res.set_content(erro("Telefone inválido. Digite 10 ou 11 números.").dump(), "application/json"); return; }
                if (!valorValido(valorTxt))    { tx.abort(); res.set_content(erro("Valor do serviço inválido.").dump(), "application/json"); return; }

                std::string cpfDigitos = apenasDigitos(cpf);
                auto cpfExiste = tx.exec_params("SELECT 1 FROM profissionais WHERE cpf = $1", cpfDigitos);
                if (!cpfExiste.empty()) { tx.abort(); res.set_content(erro("Este CPF já está cadastrado.").dump(), "application/json"); return; }

                tx.exec_params(
                    "INSERT INTO profissionais (usuario_id, nome, cpf, data_nascimento, profissao, descricao, cidade, telefone, valor) "
                    "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9)",
                    usuarioId, nome, cpfDigitos, dataParaISO(dataNasc), profissao, descricao, cidade, telefone,
                    valorParaNumero(valorTxt));
            }

            tx.commit();
            res.set_content(json{{"ok", true}, {"usuario", {{"id", usuarioId}, {"nome", nome}, {"email", email}, {"tipo", tipo}}}}.dump(),
                             "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(erro(std::string("Erro no cadastro: ") + e.what()).dump(), "application/json");
        }
    });

    // -------- POST /api/login --------
    svc.Post("/api/login", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(dbMutex);
        try {
            json body = json::parse(req.body);
            std::string email = paraMinusculo(body.value("email", ""));
            std::string senha = body.value("senha", "");
            std::string hash = hashSenha(senha);

            pqxx::connection conn(connString());
            pqxx::work tx(conn);
            auto r = tx.exec_params(
                "SELECT id, nome, email, tipo FROM usuarios WHERE lower(email) = $1 AND senha_hash = $2",
                email, hash);
            tx.commit();

            if (r.empty()) {
                res.set_content(erro("E-mail ou senha incorretos.").dump(), "application/json");
                return;
            }
            auto row = r[0];
            res.set_content(json{{"ok", true}, {"usuario", {
                {"id", row["id"].as<long long>()},
                {"nome", row["nome"].as<std::string>()},
                {"email", row["email"].as<std::string>()},
                {"tipo", row["tipo"].as<std::string>()}
            }}}.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(erro(std::string("Erro no login: ") + e.what()).dump(), "application/json");
        }
    });

    // -------- GET /api/perfil?email=...  --------
    svc.Get("/api/perfil", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(dbMutex);
        try {
            std::string email = paraMinusculo(req.get_param_value("email"));
            pqxx::connection conn(connString());
            pqxx::work tx(conn);
            auto r = tx.exec_params("SELECT id, nome, email, tipo FROM usuarios WHERE lower(email) = $1", email);
            if (r.empty()) { res.set_content(erro("Usuário não encontrado.").dump(), "application/json"); return; }
            auto row = r[0];
            json usuario = {
                {"id", row["id"].as<long long>()},
                {"nome", row["nome"].as<std::string>()},
                {"email", row["email"].as<std::string>()},
                {"tipo", row["tipo"].as<std::string>()}
            };

            if (row["tipo"].as<std::string>() == "profissional") {
                auto rp = tx.exec_params(
                    "SELECT p.nome, p.profissao, p.descricao, p.cidade, p.telefone, p.valor "
                    "FROM profissionais p WHERE p.usuario_id = $1", row["id"].as<long long>());
                if (!rp.empty()) {
                    auto pr = rp[0];
                    usuario["profissional"] = {
                        {"nome", pr["nome"].as<std::string>()},
                        {"profissao", pr["profissao"].as<std::string>()},
                        {"descricao", pr["descricao"].as<std::string>()},
                        {"cidade", pr["cidade"].as<std::string>()},
                        {"telefone", pr["telefone"].as<std::string>()},
                        {"valor", pr["valor"].as<double>()}
                    };
                }
            }
            tx.commit();
            res.set_content(json{{"ok", true}, {"usuario", usuario}}.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(erro(std::string("Erro ao buscar perfil: ") + e.what()).dump(), "application/json");
        }
    });

    // -------- POST /api/profissional (oferecer serviços) --------
    svc.Post("/api/profissional", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(dbMutex);
        try {
            json body = json::parse(req.body);
            std::string email = paraMinusculo(body.value("email", ""));

            pqxx::connection conn(connString());
            pqxx::work tx(conn);
            auto ru = tx.exec_params("SELECT id, nome, tipo FROM usuarios WHERE lower(email) = $1", email);
            if (ru.empty()) { res.set_content(erro("Usuário não encontrado.").dump(), "application/json"); return; }
            if (ru[0]["tipo"].as<std::string>() != "profissional") {
                res.set_content(erro("Apenas contas do tipo profissional podem oferecer serviços.").dump(), "application/json");
                return;
            }
            long long usuarioId = ru[0]["id"].as<long long>();
            std::string nome = ru[0]["nome"].as<std::string>();

            std::string cpf = body.value("cpf", "");
            std::string dataNasc = body.value("data_nascimento", "");
            std::string profissao = body.value("profissao", "");
            std::string descricao = body.value("descricao", "");
            std::string cidade = body.value("cidade", "");
            std::string telefone = body.value("telefone", "");
            std::string valorTxt = body.value("valor", "");

            bool profissaoValida = false;
            for (auto& p : profissoes()) if (p == profissao) profissaoValida = true;

            if (!cpfValido(cpf))           { res.set_content(erro("CPF inválido.").dump(), "application/json"); return; }
            if (!dataValida(dataNasc))     { res.set_content(erro("Data de nascimento inválida.").dump(), "application/json"); return; }
            if (!profissaoValida)          { res.set_content(erro("Profissão inválida.").dump(), "application/json"); return; }
            if (descricao.empty())         { res.set_content(erro("A descrição não pode ficar vazia.").dump(), "application/json"); return; }
            if (cidade.empty())            { res.set_content(erro("A cidade não pode ficar vazia.").dump(), "application/json"); return; }
            if (!telefoneValido(telefone)) { res.set_content(erro("Telefone inválido. Digite 10 ou 11 números.").dump(), "application/json"); return; }
            if (!valorValido(valorTxt))    { res.set_content(erro("Valor do serviço inválido.").dump(), "application/json"); return; }

            std::string cpfDigitos = apenasDigitos(cpf);
            auto cpfExiste = tx.exec_params(
                "SELECT 1 FROM profissionais WHERE cpf = $1 AND usuario_id <> $2", cpfDigitos, usuarioId);
            if (!cpfExiste.empty()) { res.set_content(erro("Este CPF já está cadastrado para outra conta.").dump(), "application/json"); return; }

            tx.exec_params(
                "INSERT INTO profissionais (usuario_id, nome, cpf, data_nascimento, profissao, descricao, cidade, telefone, valor) "
                "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9) "
                "ON CONFLICT (usuario_id) DO UPDATE SET nome=$2, cpf=$3, data_nascimento=$4, profissao=$5, "
                "descricao=$6, cidade=$7, telefone=$8, valor=$9",
                usuarioId, nome, cpfDigitos, dataParaISO(dataNasc), profissao, descricao, cidade, telefone,
                valorParaNumero(valorTxt));

            tx.commit();
            res.set_content(json{{"ok", true}}.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(erro(std::string("Erro ao cadastrar profissional: ") + e.what()).dump(), "application/json");
        }
    });

    // -------- GET /api/buscar?profissao=&cidade= --------
    svc.Get("/api/buscar", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(dbMutex);
        try {
            std::string profissao = req.get_param_value("profissao");
            std::string cidade = req.get_param_value("cidade");

            pqxx::connection conn(connString());
            pqxx::work tx(conn);
            auto r = tx.exec_params(
                "SELECT nome, profissao, descricao, cidade, telefone, valor FROM profissionais "
                "WHERE profissao = $1 AND lower(cidade) = lower($2) ORDER BY criado_em DESC",
                profissao, cidade);
            tx.commit();

            json resultados = json::array();
            for (auto row : r) {
                resultados.push_back({
                    {"nome", row["nome"].as<std::string>()},
                    {"profissao", row["profissao"].as<std::string>()},
                    {"descricao", row["descricao"].as<std::string>()},
                    {"cidade", row["cidade"].as<std::string>()},
                    {"telefone", row["telefone"].as<std::string>()},
                    {"valor", row["valor"].as<double>()}
                });
            }
            res.set_content(json{{"ok", true}, {"resultados", resultados}}.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(erro(std::string("Erro na busca: ") + e.what()).dump(), "application/json");
        }
    });

    std::cout << "Faz Ai rodando em http://0.0.0.0:" << porta << "\n";
    svc.listen("0.0.0.0", porta);
    return 0;
}
