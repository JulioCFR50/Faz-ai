-- Faz Ai - schema PostgreSQL
-- Substitui os arquivos usuarios.txt / profissionais.txt do programa original em C++

-- unaccent: permite buscar cidades ignorando acentos ("sao paulo" = "São Paulo")
CREATE EXTENSION IF NOT EXISTS unaccent;

CREATE TABLE IF NOT EXISTS usuarios (
    id          SERIAL PRIMARY KEY,
    nome        VARCHAR(150) NOT NULL,
    email       VARCHAR(150) NOT NULL UNIQUE,
    senha_hash  VARCHAR(64)  NOT NULL,          -- SHA-256 (senha + salt da aplicação)
    tipo        VARCHAR(20)  NOT NULL CHECK (tipo IN ('cliente', 'profissional')),
    criado_em   TIMESTAMP NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS profissionais (
    id               SERIAL PRIMARY KEY,
    usuario_id       INTEGER NOT NULL REFERENCES usuarios(id) ON DELETE CASCADE,
    nome             VARCHAR(150) NOT NULL,
    cpf              VARCHAR(11)  NOT NULL UNIQUE,
    data_nascimento  DATE         NOT NULL,
    profissao        VARCHAR(80)  NOT NULL,
    descricao        TEXT         NOT NULL,
    estado           CHAR(2)      NOT NULL,      -- sigla da UF (SP, RJ, PE...)
    cidade           VARCHAR(100) NOT NULL,
    telefone         VARCHAR(20)  NOT NULL,
    valor            NUMERIC(10,2) NOT NULL,
    criado_em        TIMESTAMP NOT NULL DEFAULT now()
);

-- Bancos criados antes da coluna estado: adiciona a coluna e marca os registros antigos como PE
ALTER TABLE profissionais ADD COLUMN IF NOT EXISTS estado CHAR(2);
UPDATE profissionais SET estado = 'PE' WHERE estado IS NULL;
ALTER TABLE profissionais ALTER COLUMN estado SET NOT NULL;

DROP INDEX IF EXISTS idx_profissionais_busca;
CREATE INDEX IF NOT EXISTS idx_profissionais_busca ON profissionais (profissao, estado);
CREATE UNIQUE INDEX IF NOT EXISTS idx_profissionais_usuario ON profissionais (usuario_id);

-- Dados de exemplo migrados dos arquivos usuarios.txt / profissionais.txt originais
-- (senha original de ambos: beto995906 -- hash = sha256(senha + 'faz_ai_salt_2026'))
INSERT INTO usuarios (nome, email, senha_hash, tipo) VALUES
    ('alberto', 'albertobatista@gmail.com', '18562497ced30db1ddf73e117ec8d6ebcb4f63e9aad3549cb930fe8c00282c43', 'profissional'),
    ('alberto da silva', 'albertobatista88@gmail.com', '18562497ced30db1ddf73e117ec8d6ebcb4f63e9aad3549cb930fe8c00282c43', 'cliente')
ON CONFLICT (email) DO NOTHING;

INSERT INTO profissionais (usuario_id, nome, cpf, data_nascimento, profissao, descricao, estado, cidade, telefone, valor)
SELECT u.id, 'alberto da silva batista', '09807476437', DATE '1994-08-07', 'Mecanico',
       'serviço de mecanico em geral', 'PE', 'recife', '81995248740', 300.00
FROM usuarios u WHERE u.email = 'albertobatista@gmail.com'
ON CONFLICT (cpf) DO NOTHING;
