# Faz Ai — versão web

Este projeto transforma o programa de console **Faz Ai** (`faz_ai_1.cpp`) em um site, mantendo a linguagem C/C++ e trocando os arquivos `usuarios.txt` / `profissionais.txt` por um banco de dados **PostgreSQL**. A partir do site é possível gerar um **APK Android instalável** (veja a seção final).

## Como o projeto foi organizado

```
faz-ai/
├── server/          Servidor C++ (API + validações + acesso ao banco)
│   ├── server.cpp
│   ├── regras.hpp   Validações portadas do faz_ai_1.cpp (e-mail, CPF, telefone, data, valor)
│   └── httplib.h     Biblioteca HTTP (cpp-httplib), baixada automaticamente no build
├── web/             Frontend (HTML/CSS/JS) servido pelo próprio servidor C++
│   ├── index.html, styles.css, app.js
│   └── manifest.json, sw.js, icon-*.png   (deixam o site "instalável" como PWA)
├── db/
│   └── schema.sql   Tabelas usuarios/profissionais + dados de exemplo migrados dos seus .txt
├── Dockerfile
└── docker-compose.yml
```

A lógica de negócio (validar e-mail, CPF, telefone, data de nascimento, valor do serviço, lista de profissões) é a mesma do programa original, só que agora atendida por requisições HTTP em vez do teclado. As senhas passaram a ser guardadas com hash SHA-256 (no `.txt` original ficavam em texto puro).

## Rodando localmente (com Docker — caminho mais simples)

Pré-requisitos: [Docker](https://docs.docker.com/get-docker/) instalado.

```bash
docker compose up --build
```

Isso sobe o PostgreSQL (já criando as tabelas e os dados de exemplo a partir de `db/schema.sql`) e o servidor C++, disponível em **http://localhost:8080**.

Login de exemplo (migrado do seu `usuarios.txt`/`profissionais.txt`):
- E-mail: `albertobatista@gmail.com` — Senha: `beto995906` (conta profissional, "Mecanico" em Recife)

## Rodando sem Docker

```bash
sudo apt-get install -y g++ libpq-dev libpqxx-dev libssl-dev nlohmann-json3-dev postgresql
sudo service postgresql start
sudo -u postgres psql -c "CREATE USER fazai WITH PASSWORD 'fazai_dev_pw';"
sudo -u postgres psql -c "CREATE DATABASE fazai_db OWNER fazai;"
sudo -u postgres psql -d fazai_db -f db/schema.sql
sudo -u postgres psql -d fazai_db -c "GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO fazai;"

curl -sL https://raw.githubusercontent.com/yhirose/cpp-httplib/v0.15.3/httplib.h -o server/httplib.h
g++ -std=c++17 -O2 -pthread server/server.cpp -o fazai_server $(pkg-config --cflags --libs libpqxx) -lssl -lcrypto

DATABASE_URL="dbname=fazai_db user=fazai password=fazai_dev_pw host=127.0.0.1 port=5432" ./fazai_server
```

O servidor foi testado (cadastro de cliente, cadastro de profissional com validação de CPF/data/telefone/valor, login, busca por profissão + cidade, atualização de perfil profissional) e todos os fluxos responderam corretamente.

## Colocando no ar (para ter uma URL pública)

Qualquer serviço que rode containers Docker funciona, por exemplo **Railway** ou **Render**:

1. Suba este projeto para um repositório Git (GitHub/GitLab).
2. Crie um novo serviço "Docker" apontando para o repositório — ele vai usar o `Dockerfile` automaticamente.
3. Crie um banco PostgreSQL no mesmo serviço (Railway e Render oferecem isso "gerenciado") e copie a variável de conexão para `DATABASE_URL` do app.
4. Na primeira execução, rode `db/schema.sql` contra esse banco (as plataformas normalmente têm um "console"/"shell" para isso, ou você aponta um cliente `psql` para a `DATABASE_URL` fornecida).
5. Ao final você terá uma URL pública em HTTPS, por exemplo `https://faz-ai.up.railway.app`.

Essa URL pública é o que você vai usar no passo do APK abaixo — o Android exige que o site esteja em HTTPS.

## Do site para o APK (Android)

O site já vem com `manifest.json` e `sw.js`, ou seja, já é um **PWA** (Progressive Web App) — pode ser "instalado" direto do navegador. Para gerar um **arquivo .apk** de verdade a partir dele, o caminho mais simples e sem escrever nenhuma linha de código extra é:

1. Acesse **https://www.pwabuilder.com**.
2. Cole a URL pública do site (do passo anterior) e clique em "Start".
3. O PWABuilder vai analisar o `manifest.json` e o `sw.js` automaticamente.
4. Escolha a opção **Android** e clique em "Generate Package" — ele empacota o site como um app Android real (tecnicamente uma *Trusted Web Activity*), usando o mesmo site por dentro.
5. Baixe o `.apk` (ou `.aab` para publicar na Google Play) gerado.

Esse processo usa por trás a ferramenta [Bubblewrap](https://github.com/GoogleChromeLabs/bubblewrap) do próprio Google, então é o caminho oficialmente recomendado para "site → Android". Se no futuro quiser um app 100% nativo (não baseado no site), o próximo passo seria reescrever a interface em Flutter ou React Native consumindo a mesma API C++ — a API não precisa mudar.

## Pontos de atenção antes de usar "de verdade"

- Troque a senha do banco (`fazai_dev_pw`) e o "salt" das senhas em `server/regras.hpp` por valores próprios e secretos.
- Não há hoje um sistema de sessão/token (login apenas confirma e-mail/senha) — para produção, vale adicionar JWT ou cookies de sessão.
- O hash de senha usado é SHA-256 com salt fixo, o que já é bem melhor que texto puro, mas para produção o ideal é usar bcrypt ou Argon2.
- Faça backup regular do banco PostgreSQL (ele agora é a fonte de verdade dos dados, no lugar dos arquivos `.txt`).
