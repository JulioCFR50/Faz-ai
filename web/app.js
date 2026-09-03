// app.js - frontend do Faz Ai (consome a API do servidor C++)
const API = ""; // mesmo domínio do servidor

const estado = {
  usuario: null // {id, nome, email, tipo}
};

// ---------- utilidades ----------
function $(sel) { return document.querySelector(sel); }
function $all(sel) { return document.querySelectorAll(sel); }

function mostrarTela(id) {
  $all(".container").forEach(el => el.classList.add("hidden"));
  $("#" + id).classList.remove("hidden");
  $("#btnSair").classList.toggle("hidden", !estado.usuario);
}

function mostrarErro(elId, msg) {
  const el = $("#" + elId);
  el.textContent = msg;
  el.style.display = "block";
}
function esconderErro(elId) {
  const el = $("#" + elId);
  el.style.display = "none";
}
function mostrarOk(elId, msg) {
  const el = $("#" + elId);
  el.textContent = msg;
  el.style.display = "block";
}

async function api(metodo, caminho, dados) {
  const opcoes = { method: metodo, headers: { "Content-Type": "application/json" } };
  if (dados) opcoes.body = JSON.stringify(dados);
  const resp = await fetch(API + caminho, opcoes);
  return resp.json();
}

function salvarSessao(usuario) {
  estado.usuario = usuario;
  try { localStorage.setItem("fazai_sessao", JSON.stringify(usuario)); } catch (e) {}
}
function limparSessao() {
  estado.usuario = null;
  try { localStorage.removeItem("fazai_sessao"); } catch (e) {}
}
function carregarSessao() {
  try {
    const raw = localStorage.getItem("fazai_sessao");
    if (raw) estado.usuario = JSON.parse(raw);
  } catch (e) {}
}

// ---------- profissões (preenche selects) ----------
async function preencherProfissoes() {
  const r = await api("GET", "/api/profissoes");
  if (!r.ok) return;
  const opcoesHtml = '<option value="">Selecione...</option>' +
    r.profissoes.map(p => `<option value="${p}">${p}</option>`).join("");
  ["#cadProfissao", "#buscaProfissao", "#ofProfissao"].forEach(sel => {
    const el = $(sel);
    if (el) el.innerHTML = opcoesHtml;
  });
}

// ---------- alternância login / cadastro ----------
$("#linkIrCadastro").addEventListener("click", () => {
  $("#formLogin").classList.add("hidden");
  $("#formCadastro").classList.remove("hidden");
  $("#trocaTextoLogin").classList.add("hidden");
  $("#trocaTextoCadastro").classList.remove("hidden");
  $("#authTitulo").textContent = "Criar conta";
  $("#authSubtitulo").textContent = "Cadastre-se como cliente ou profissional.";
});
$("#linkIrLogin").addEventListener("click", () => {
  $("#formCadastro").classList.add("hidden");
  $("#formLogin").classList.remove("hidden");
  $("#trocaTextoCadastro").classList.add("hidden");
  $("#trocaTextoLogin").classList.remove("hidden");
  $("#authTitulo").textContent = "Entrar";
  $("#authSubtitulo").textContent = "Encontre ou ofereça serviços na sua cidade.";
});

$all('input[name="tipoConta"]').forEach(radio => {
  radio.addEventListener("change", () => {
    $all(".tipo-toggle .opcao").forEach(l => l.classList.remove("ativo"));
    radio.closest(".opcao").classList.add("ativo");
    $("#blocoProfissional").classList.toggle("hidden", radio.value !== "profissional");
  });
});

// ---------- login ----------
$("#formLogin").addEventListener("submit", async (e) => {
  e.preventDefault();
  esconderErro("loginErro");
  const email = $("#loginEmail").value.trim();
  const senha = $("#loginSenha").value;
  const r = await api("POST", "/api/login", { email, senha });
  if (!r.ok) { mostrarErro("loginErro", r.erro); return; }
  salvarSessao(r.usuario);
  entrarNoMenu();
});

// ---------- cadastro ----------
$("#formCadastro").addEventListener("submit", async (e) => {
  e.preventDefault();
  esconderErro("cadastroErro");
  const tipo = $('input[name="tipoConta"]:checked').value;
  const payload = {
    nome: $("#cadNome").value.trim(),
    email: $("#cadEmail").value.trim(),
    senha: $("#cadSenha").value,
    tipo
  };
  if (tipo === "profissional") {
    Object.assign(payload, {
      cpf: $("#cadCpf").value.trim(),
      data_nascimento: $("#cadNascimento").value.trim(),
      profissao: $("#cadProfissao").value,
      descricao: $("#cadDescricao").value.trim(),
      cidade: $("#cadCidade").value.trim(),
      telefone: $("#cadTelefone").value.trim(),
      valor: $("#cadValor").value.trim()
    });
  }
  const r = await api("POST", "/api/cadastro", payload);
  if (!r.ok) { mostrarErro("cadastroErro", r.erro); return; }
  salvarSessao(r.usuario);
  entrarNoMenu();
});

// ---------- menu principal ----------
function entrarNoMenu() {
  $("#menuNome").textContent = estado.usuario.nome;
  $("#itemOferecer").classList.toggle("hidden", estado.usuario.tipo !== "profissional");
  mostrarTela("telaMenu");
}

$all(".menu-item").forEach(item => {
  item.addEventListener("click", () => {
    const acao = item.dataset.acao;
    if (acao === "buscar") mostrarTela("telaBuscar");
    else if (acao === "oferecer") abrirOferecer();
    else if (acao === "perfil") abrirPerfil();
  });
});

$all("[data-voltar]").forEach(btn => {
  btn.addEventListener("click", () => mostrarTela("telaMenu"));
});

// ---------- tema claro/escuro escolhido pelo usuário ----------
function temaAtual() {
  const escolhido = document.documentElement.getAttribute("data-theme");
  if (escolhido === "light" || escolhido === "dark") return escolhido;
  return window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light";
}
function atualizarIconeTema() {
  $("#btnTema").textContent = temaAtual() === "dark" ? "☀️" : "🌙";
}
atualizarIconeTema();
$("#btnTema").addEventListener("click", () => {
  const novoTema = temaAtual() === "dark" ? "light" : "dark";
  document.documentElement.setAttribute("data-theme", novoTema);
  try { localStorage.setItem("fazai-tema", novoTema); } catch (e) {}
  atualizarIconeTema();
});

$("#btnSair").addEventListener("click", () => {
  limparSessao();
  $("#formCadastro").classList.add("hidden");
  $("#formLogin").classList.remove("hidden");
  $("#loginEmail").value = "";
  $("#loginSenha").value = "";
  mostrarTela("telaAuth");
});

// ---------- buscar serviço ----------
$("#btnBuscar").addEventListener("click", async () => {
  const profissao = $("#buscaProfissao").value;
  const cidade = $("#buscaCidade").value.trim();
  const resultadosEl = $("#resultados");
  if (!profissao || !cidade) {
    resultadosEl.innerHTML = '<div class="vazio">Escolha a profissão e digite a cidade.</div>';
    return;
  }
  const r = await api("GET", `/api/buscar?profissao=${encodeURIComponent(profissao)}&cidade=${encodeURIComponent(cidade)}`);
  if (!r.ok || r.resultados.length === 0) {
    resultadosEl.innerHTML = `<div class="vazio">Nenhum profissional de ${profissao} encontrado em ${cidade}.</div>`;
    return;
  }
  resultadosEl.innerHTML = r.resultados.map(p => `
    <div class="resultado-card">
      <div class="nome">${p.nome}</div>
      <div class="profissao-tag">${p.profissao}</div>
      <div class="linha">${p.descricao}</div>
      <div class="linha">📍 ${p.cidade} &nbsp;·&nbsp; <a class="tel" href="tel:${p.telefone}">${p.telefone}</a></div>
      <div class="linha">Valor: <span class="valor">R$ ${Number(p.valor).toFixed(2).replace(".", ",")}</span></div>
    </div>
  `).join("");
});

// ---------- oferecer serviços ----------
async function abrirOferecer() {
  esconderErro("oferecerErro");
  $("#oferecerOk").style.display = "none";
  const r = await api("GET", `/api/perfil?email=${encodeURIComponent(estado.usuario.email)}`);
  if (r.ok && r.usuario.profissional) {
    const p = r.usuario.profissional;
    $("#ofProfissao").value = p.profissao;
    $("#ofDescricao").value = p.descricao;
    $("#ofCidade").value = p.cidade;
    $("#ofTelefone").value = p.telefone;
    $("#ofValor").value = String(p.valor).replace(".", ",");
  }
  mostrarTela("telaOferecer");
}

$("#formOferecer").addEventListener("submit", async (e) => {
  e.preventDefault();
  esconderErro("oferecerErro");
  $("#oferecerOk").style.display = "none";
  const payload = {
    email: estado.usuario.email,
    cpf: $("#ofCpf").value.trim(),
    data_nascimento: $("#ofNascimento").value.trim(),
    profissao: $("#ofProfissao").value,
    descricao: $("#ofDescricao").value.trim(),
    cidade: $("#ofCidade").value.trim(),
    telefone: $("#ofTelefone").value.trim(),
    valor: $("#ofValor").value.trim()
  };
  const r = await api("POST", "/api/profissional", payload);
  if (!r.ok) { mostrarErro("oferecerErro", r.erro); return; }
  mostrarOk("oferecerOk", "Dados salvos com sucesso!");
});

// ---------- perfil ----------
async function abrirPerfil() {
  const r = await api("GET", `/api/perfil?email=${encodeURIComponent(estado.usuario.email)}`);
  const card = $("#perfilCard");
  if (!r.ok) {
    card.innerHTML = '<h1 class="titulo">Meu perfil</h1><div class="vazio">Não foi possível carregar o perfil.</div>';
    mostrarTela("telaPerfil");
    return;
  }
  const u = r.usuario;
  let html = `<h1 class="titulo">Meu perfil</h1>
    <div class="perfil-linha"><span class="rot">Nome</span><span class="val">${u.nome}</span></div>
    <div class="perfil-linha"><span class="rot">E-mail</span><span class="val">${u.email}</span></div>
    <div class="perfil-linha"><span class="rot">Tipo de conta</span><span class="val">${u.tipo === "profissional" ? "Profissional" : "Cliente"}</span></div>`;
  if (u.profissional) {
    const p = u.profissional;
    html += `
    <div class="perfil-linha"><span class="rot">Profissão</span><span class="val">${p.profissao}</span></div>
    <div class="perfil-linha"><span class="rot">Cidade</span><span class="val">${p.cidade}</span></div>
    <div class="perfil-linha"><span class="rot">Telefone</span><span class="val">${p.telefone}</span></div>
    <div class="perfil-linha"><span class="rot">Valor</span><span class="val">R$ ${Number(p.valor).toFixed(2).replace(".", ",")}</span></div>`;
  }
  card.innerHTML = html;
  mostrarTela("telaPerfil");
}

// ---------- inicialização ----------
(async function iniciar() {
  await preencherProfissoes();
  carregarSessao();
  if (estado.usuario) entrarNoMenu();
  else mostrarTela("telaAuth");

  if ("serviceWorker" in navigator) {
    navigator.serviceWorker.register("/sw.js").catch(() => {});
  }
})();
