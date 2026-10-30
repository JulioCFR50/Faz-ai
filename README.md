# Faz Ai

**Plataforma Digital de Conexão e Agendamento de Serviços Domésticos**

Projeto Integrador – Senac

O Faz Ai conecta quem precisa de um serviço doméstico a profissionais autônomos de todo o Brasil. O cliente busca por profissão, estado e cidade, vê o preço antes de contratar e fala direto com o profissional. O profissional ganha uma vitrine gratuita, com descrição, cidade e valor do serviço.

🔗 **Demonstração online:** https://juliocfr50.github.io/Faz-ai/
📄 **Apresentação:** [apresentacao/Faz Ai - Projeto Integrador.pdf](apresentacao/Faz%20Ai%20-%20Projeto%20Integrador.pdf)

---

## Definição do tema

**Contexto.** Muitos profissionais autônomos não têm um canal simples para divulgar seus serviços. Quem precisa de ajuda depende de indicação, grupos de WhatsApp e redes sociais.

**Problema.** É difícil encontrar profissionais confiáveis para serviços como encanamento, elétrica, limpeza e pequenos reparos.

**Solução.** Uma plataforma digital que facilita a busca, a comparação e o agendamento de serviços por profissão, estado e cidade, em todo o Brasil.

**Motivação.** O projeto começou como um programa de console em C++ e evoluiu para um site com banco de dados, que também pode ser instalado no celular como aplicativo.

## Proposta de valor

**Público-alvo:** moradores de todo o Brasil que precisam de serviços domésticos e profissionais autônomos de 50 profissões.

**Partes interessadas:** clientes, profissionais, equipe do projeto e Senac.

| Para o cliente | Para o profissional |
|---|---|
| Busca por profissão, estado e cidade | Perfil com descrição, cidade e preço |
| Vê o preço antes de contratar | Mais oportunidades de clientes |
| Menos tempo perdido procurando indicação | Contato direto, sem comissão |
| Profissionais com CPF e telefone validados | Organização de horários com agendamento *(próxima etapa)* |

**Diferencial:** juntar busca, comparação e agendamento em um só aplicativo simples.

## Análise de viabilidade

| Dimensão | Por que é viável |
|---|---|
| **Técnica** | Já existe um protótipo funcionando, feito com C++, PostgreSQL e tecnologias web, e ele também roda como app Android. |
| **Econômica** | Baixo custo inicial: hospedagem gratuita ou barata e geração do APK sem custo. |
| **Operacional** | Interface simples, pensada para qualquer pessoa usar pelo celular. |
| **Social** | Facilita o acesso a serviços domésticos e gera renda para profissionais autônomos. |

### Mercado e concorrência

- **Mercado:** moradores de todo o Brasil que contratam serviços domésticos e autônomos que buscam clientes.
- **Concorrentes diretos:** GetNinjas e Parafuzo.
- **Concorrentes indiretos:** grupos de WhatsApp e Facebook, OLX e indicação boca a boca.
- **Diferencial:** atende os 27 estados, com contato direto sem comissão e uso simples.

### Recursos necessários

- Equipe de desenvolvimento, design e divulgação
- Servidor e banco de dados PostgreSQL
- Domínio com HTTPS

---

## O que o sistema faz

- Cadastro de **cliente** ou **profissional**
- Login com senha guardada em hash (SHA-256)
- Cadastro do profissional com validação de **CPF**, data de nascimento, telefone e valor do serviço
- 50 profissões disponíveis (eletricista, encanador, diarista, manicure, técnico de informática e outras)
- **Busca por profissão e estado**, com cidade opcional. A busca ignora acentos e maiúsculas, então "sao paulo" encontra "São Paulo".
- Perfil do profissional editável
- Tema claro e escuro
- Funciona no navegador do computador, no celular e como app Android

## Tecnologias

| Parte | Tecnologia |
|---|---|
| Servidor (API) | C++17 com [cpp-httplib](https://github.com/yhirose/cpp-httplib), [libpqxx](https://github.com/jtv/libpqxx) e [nlohmann/json](https://github.com/nlohmann/json) |
| Banco de dados | PostgreSQL |
| Site | HTML, CSS e JavaScript |
| App Android | Java + WebView (Android Studio) |
| Execução | Docker e Docker Compose |

## Estrutura do projeto

```text
Faz-ai/
├── server/              Servidor C++
│   ├── server.cpp       Rotas da API (cadastro, login, perfil, busca, estados, profissões)
│   └── regras.hpp       Regras de negócio: validação de e-mail, CPF, telefone, data, valor e UF
├── web/                 Site do sistema, servido pelo próprio servidor C++
├── db/
│   └── schema.sql       Tabelas usuarios e profissionais + dados de exemplo
├── docs/                Demonstração online (GitHub Pages), roda sem servidor
├── FazAi/               Projeto Android Studio que gera o APK
├── apresentacao/        Slides do Projeto Integrador (PDF e PowerPoint)
├── icone/               Ícone do app
├── Dockerfile
└── docker-compose.yml
```

## Como executar

### Opção 1 – Só a demonstração (sem instalar nada)

Abra https://juliocfr50.github.io/Faz-ai/ e entre com uma das contas de exemplo:

| Conta | E-mail | Senha |
|---|---|---|
| Cliente | `cliente@fazai.demo` | `demo1234` |
| Profissional | `mecanico@fazai.demo` | `demo1234` |

Na demonstração os dados ficam guardados no próprio navegador. Cada pessoa vê só o que ela mesma cadastrar.

### Opção 2 – Sistema completo com Docker

Pré-requisito: [Docker](https://docs.docker.com/get-docker/) instalado.

```bash
docker compose up --build
```

Esse comando sobe o PostgreSQL, cria as tabelas e os dados de exemplo a partir de `db/schema.sql` e inicia o servidor C++. O sistema fica em **http://localhost:8080**.

### Opção 3 – Sistema completo sem Docker (Linux)

```bash
sudo apt-get install -y g++ libpq-dev libpqxx-dev libssl-dev nlohmann-json3-dev postgresql postgresql-contrib
sudo service postgresql start
sudo -u postgres psql -c "CREATE USER fazai WITH PASSWORD 'fazai_dev_pw';"
sudo -u postgres psql -c "CREATE DATABASE fazai_db OWNER fazai;"
sudo -u postgres psql -d fazai_db -f db/schema.sql
sudo -u postgres psql -d fazai_db -c "GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO fazai;"

curl -sL https://raw.githubusercontent.com/yhirose/cpp-httplib/v0.15.3/httplib.h -o server/httplib.h
g++ -std=c++17 -O2 -pthread server/server.cpp -o fazai_server $(pkg-config --cflags --libs libpqxx) -lssl -lcrypto

DATABASE_URL="dbname=fazai_db user=fazai password=fazai_dev_pw host=127.0.0.1 port=5432" ./fazai_server
```

O pacote `postgresql-contrib` traz a extensão `unaccent`, que a busca usa para ignorar acentos.

> **Banco criado antes da busca por estado?** Rode `db/schema.sql` de novo. Ele adiciona a coluna `estado` e marca os cadastros antigos como PE.

## App Android

O app fica na pasta `FazAi/`. Ele abre o Faz Ai dentro de uma tela própria (WebView), em tela cheia.

1. Abra a pasta `FazAi` no Android Studio.
2. O endereço que o app abre está em `FazAi/gradle.properties`:

   ```properties
   fazai.serverUrl=https://juliocfr50.github.io/Faz-ai/
   ```

   Quando o servidor C++ for publicado, basta trocar essa linha pelo endereço novo.
3. Gere o APK em **Build → Generate App Bundles or APKs → Generate APKs**.
4. O arquivo sai em `FazAi/app/build/outputs/apk/debug/FazAi-debug.apk`. Para instalar, copie para o celular, abra o arquivo e permita a instalação de apps desconhecidos.

## Publicar o sistema completo

Qualquer serviço que rode Docker funciona, por exemplo **Railway** ou **Render**:

1. Crie um serviço "Docker" apontando para este repositório. Ele usa o `Dockerfile` automaticamente.
2. Crie um banco PostgreSQL no mesmo serviço e copie o endereço de conexão para a variável `DATABASE_URL`.
3. Rode `db/schema.sql` nesse banco.
4. Use o endereço HTTPS gerado (por exemplo `https://faz-ai.up.railway.app/`) no `gradle.properties` do app Android.

## Próximas etapas

- Agendamento de horários
- Avaliação dos profissionais
- Login com mais segurança (sessão ou token, senhas com bcrypt ou Argon2)
- Publicação do servidor com banco compartilhado entre todos os usuários

## Antes de usar em produção

- Troque a senha do banco (`fazai_dev_pw`) e o salt das senhas em `server/regras.hpp` por valores próprios e secretos.
- Hoje o login só confere e-mail e senha, sem sessão. Para produção, adicione JWT ou cookies de sessão.
- O hash SHA-256 com salt fixo é melhor que texto puro, mas o ideal é bcrypt ou Argon2.
- Faça backup regular do banco PostgreSQL.

## Referências

- GETNINJAS. Disponível em: https://www.getninjas.com.br. Acesso em: 29 out. 2026.
- PARAFUZO. Disponível em: https://www.parafuzo.com. Acesso em: 29 out. 2026.
- POSTGRESQL. Disponível em: https://www.postgresql.org. Acesso em: 29 out. 2026.
- ANDROID DEVELOPERS. WebView. Disponível em: https://developer.android.com/develop/ui/views/layout/webapps/webview. Acesso em: 30 out. 2026.
