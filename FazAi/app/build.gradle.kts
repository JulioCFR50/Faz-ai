plugins {
    alias(libs.plugins.android.application)
}

base {
    archivesName = "FazAi"
}

val serverUrl = providers.gradleProperty("fazai.serverUrl").get()

android {
    namespace = "com.juliocfr.fazai"
    compileSdk {
        version = release(37)
    }

    defaultConfig {
        applicationId = "com.juliocfr.fazai"
        minSdk = 24
        targetSdk = 37
        versionCode = 1
        versionName = "1.0"


        buildConfigField("String", "SERVER_URL", "\"$serverUrl\"")
    }

    buildFeatures {
        buildConfig = true
    }

    buildTypes {
        release {
            optimization {
                enable = false
            }
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
}

dependencies {
    implementation(libs.appcompat)
    implementation(libs.material)
}