package com.juliocfr.fazai;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.webkit.JavascriptInterface;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.FrameLayout;

/**
 * App Android do Faz Ai: abre o site servido pelo servidor C++ (pasta web/ do projeto)
 * dentro de uma WebView. O endereço do servidor vem de gradle.properties (fazai.serverUrl).
 */
public class MainActivity extends Activity {

    private static final String SERVER_URL = BuildConfig.SERVER_URL;

    private WebView web;

    @SuppressLint("SetJavaScriptEnabled")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // O FrameLayout com fitsSystemWindows deixa o conteúdo fora da barra de status
        // e da barra de navegação (o Android 15+ desenha o app de ponta a ponta).
        FrameLayout raiz = new FrameLayout(this);
        raiz.setFitsSystemWindows(true);
        raiz.setBackgroundColor(Color.parseColor("#1F7A4D"));

        web = new WebView(this);
        raiz.addView(web, new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT));
        setContentView(raiz);

        WebSettings s = web.getSettings();
        s.setJavaScriptEnabled(true);
        s.setDomStorageEnabled(true); // o site guarda a sessão e o tema em localStorage
        s.setUserAgentString(s.getUserAgentString() + " FazAiApp");

        web.addJavascriptInterface(new Ponte(), "FazAiApp");
        web.setWebViewClient(new Cliente());

        if (savedInstanceState != null) {
            web.restoreState(savedInstanceState);
        } else {
            web.loadUrl(SERVER_URL);
        }
    }

    private class Cliente extends WebViewClient {
        @Override
        public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request) {
            Uri url = request.getUrl();
            if (url.toString().startsWith(SERVER_URL)) return false;
            // Links de telefone (tel:) e sites externos abrem no app certo do celular
            try {
                startActivity(new Intent(Intent.ACTION_VIEW, url));
            } catch (ActivityNotFoundException e) {
                // nenhum app para abrir esse link: ignora
            }
            return true;
        }

        @Override
        public void onReceivedError(WebView view, WebResourceRequest request, WebResourceError error) {
            if (request.isForMainFrame()) mostrarSemConexao();
        }
    }

    /** Usada pelo botão "Tentar de novo" da tela sem conexão. */
    private class Ponte {
        @JavascriptInterface
        public void tentarDeNovo() {
            runOnUiThread(() -> web.loadUrl(SERVER_URL));
        }
    }

    private void mostrarSemConexao() {
        String html = "<!doctype html><html lang='pt-BR'><head><meta charset='utf-8'>"
                + "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                + "<style>body{margin:0;font-family:sans-serif;background:#f4f8f4;color:#152018;"
                + "display:flex;align-items:center;justify-content:center;min-height:100vh;text-align:center}"
                + "div{padding:24px;max-width:320px}h1{font-size:1.3rem;color:#12522f}"
                + "p{color:#5c6f61;line-height:1.5}"
                + "button{margin-top:12px;padding:12px 20px;border:0;border-radius:8px;background:#1f7a4d;"
                + "color:#fff;font-size:1rem}</style></head><body><div>"
                + "<h1>Não foi possível conectar</h1>"
                + "<p>Verifique sua internet. Se estiver testando, confira se o servidor do Faz Ai está rodando em "
                + SERVER_URL + "</p>"
                + "<button onclick='FazAiApp.tentarDeNovo()'>Tentar de novo</button>"
                + "</div></body></html>";
        web.loadDataWithBaseURL(null, html, "text/html", "UTF-8", null);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        web.saveState(outState);
    }

    @Override
    public void onBackPressed() {
        if (web.canGoBack()) {
            web.goBack();
        } else {
            super.onBackPressed();
        }
    }
}
