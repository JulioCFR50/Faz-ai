package com.juliocfr.fazai;

import android.os.Bundle;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.FrameLayout;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Tela que segura o site, sem ficar embaixo da barra de status
        FrameLayout tela = new FrameLayout(this);
        tela.setFitsSystemWindows(true);

        // "Navegador" dentro do app
        WebView web = new WebView(this);
        web.getSettings().setJavaScriptEnabled(true);   // o site precisa de JavaScript
        web.getSettings().setDomStorageEnabled(true);   // o site guarda os dados no celular
        web.getSettings().setUserAgentString(web.getSettings().getUserAgentString() + " FazAiApp");
        web.setWebViewClient(new WebViewClient());      // abre os links dentro do app

        tela.addView(web);
        setContentView(tela);

        web.loadUrl(BuildConfig.SERVER_URL);

    }
}
