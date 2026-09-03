// sw.js - Service Worker do Faz Ai
// Cacheia os arquivos estáticos (app shell) para permitir instalação como PWA
// e abertura rápida/offline da interface. As chamadas /api/* sempre vão para a rede.
const CACHE = "fazai-v1";
const ARQUIVOS = [
  "/", "/index.html", "/styles.css", "/app.js", "/manifest.json",
  "/icon-192.png", "/icon-512.png", "/icon-maskable-512.png"
];

self.addEventListener("install", (event) => {
  event.waitUntil(
    caches.open(CACHE).then((cache) => cache.addAll(ARQUIVOS))
  );
  self.skipWaiting();
});

self.addEventListener("activate", (event) => {
  event.waitUntil(
    caches.keys().then((chaves) =>
      Promise.all(chaves.filter((c) => c !== CACHE).map((c) => caches.delete(c)))
    )
  );
  self.clients.claim();
});

self.addEventListener("fetch", (event) => {
  const url = new URL(event.request.url);
  if (url.pathname.startsWith("/api/")) return; // nunca cacheia a API

  event.respondWith(
    caches.match(event.request).then((cache) => {
      return cache || fetch(event.request).then((resp) => {
        const copia = resp.clone();
        caches.open(CACHE).then((c) => c.put(event.request, copia));
        return resp;
      }).catch(() => cache);
    })
  );
});
