#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// =====================================================
// 1. CONFIGURACAO DO DISPLAY LCD
// =====================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);

// =====================================================
// 2. MAPEAMENTO DOS PINOS
// =====================================================

const int trigPin = 2;
const int echoPin = 3;

const int buzzerPin = 6;

const int ledVerde = 8;
const int ledAmarelo = 9;
const int ledVermelho = 10;

// =====================================================
// 3. CONFIGURACAO DO RESERVATORIO
// =====================================================

// Altura util do reservatorio em centimetros.
const float alturaTotalReservatorio = 100.0;

// Distancia entre o sensor e o fundo.
// Pode ser alterada pela calibracao.
float distanciaSensorFundo = 100.0;

// Intervalo entre as medicoes.
const unsigned long intervalo = 1000;

// Tempo maximo de espera pelo eco.
const unsigned long tempoLimiteEco = 30000;

unsigned long ultimaMedicao = 0;
unsigned long inicioFase = 0;
unsigned long inicioEco = 0;
unsigned long inicioEsperaEco = 0;

// =====================================================
// 4. ESTADOS DE OPERACAO
// =====================================================

enum Estado {
  INICIALIZANDO,
  ALTO,
  MEDIO,
  BAIXO,
  FALHA_SEM_ECO,
  FALHA_DISTANCIA
};

Estado estadoAtual = INICIALIZANDO;

// Estados internos da medicao ultrassonica.

enum FaseMedicao {
  OCIOSO,
  TRIG_BAIXO,
  TRIG_ALTO,
  ESPERA_SUBIDA,
  ESPERA_DESCIDA
};

FaseMedicao fase = OCIOSO;

// =====================================================
// 5. VARIAVEIS DO FILTRO E CALIBRACAO
// =====================================================

float leituras[3];

int indiceLeitura = 0;

bool filtroInicializado = false;
bool calibrar = false;

// =====================================================
// 6. CONFIGURACAO INICIAL
// =====================================================

void setup() {

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(buzzerPin, OUTPUT);

  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVermelho, OUTPUT);

  digitalWrite(trigPin, LOW);

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  escreverLCD(0, "Sistema Nivel");
  escreverLCD(1, "Iniciando...");

  Serial.println("Sistema iniciado.");
  Serial.println("Digite C para calibrar");
  Serial.println("com o reservatorio vazio.");

  ultimaMedicao = millis();
}

// =====================================================
// 7. FUNCAO PRINCIPAL
// =====================================================

void loop() {

  // Processa comandos e inicia novas medicoes
  // somente quando o sensor estiver ocioso.

  if (fase == OCIOSO) {

    verificarComandos();

    unsigned long agora = millis();

    if (agora - ultimaMedicao >= intervalo) {

      ultimaMedicao = agora;

      digitalWrite(trigPin, LOW);

      inicioFase = micros();

      fase = TRIG_BAIXO;
    }
  }

  // Executa a medicao sem bloquear o programa.
  atualizarSensor();
}

// =====================================================
// 8. MEDICAO ULTRASSONICA NAO BLOQUEANTE
// =====================================================

void atualizarSensor() {

  unsigned long agora = micros();

  switch (fase) {

    case OCIOSO:
      break;

    // Mantem TRIG desligado por 2 microssegundos.

    case TRIG_BAIXO:

      if (agora - inicioFase >= 2) {

        digitalWrite(trigPin, HIGH);

        inicioFase = micros();

        fase = TRIG_ALTO;
      }

      break;

    // Envia um pulso de pelo menos 10 microssegundos.

    case TRIG_ALTO:

      if (agora - inicioFase >= 10) {

        digitalWrite(trigPin, LOW);

        inicioEsperaEco = micros();

        fase = ESPERA_SUBIDA;
      }

      break;

    // Aguarda o inicio do sinal de retorno.

    case ESPERA_SUBIDA:

      if (agora - inicioEsperaEco >= tempoLimiteEco) {

        fase = OCIOSO;

        tratarFalha(FALHA_SEM_ECO);

      } else if (digitalRead(echoPin) == HIGH) {

        inicioEco = micros();

        fase = ESPERA_DESCIDA;
      }

      break;

    // Mede a duracao do sinal de retorno.

    case ESPERA_DESCIDA:

      if (agora - inicioEco >= tempoLimiteEco) {

        fase = OCIOSO;

        tratarFalha(FALHA_SEM_ECO);

      } else if (digitalRead(echoPin) == LOW) {

        unsigned long duracao = micros() - inicioEco;

        fase = OCIOSO;

        // Converte o tempo em distancia.

        float distancia = duracao * 0.034 / 2.0;

        processarLeitura(distancia);
      }

      break;
  }
}

// =====================================================
// 9. PROCESSAMENTO DA LEITURA
// =====================================================

void processarLeitura(float distancia) {

  // Verifica a faixa fisica prevista para o sensor.

  if (distancia < 2.0 || distancia > 400.0) {

    tratarFalha(FALHA_DISTANCIA);

    return;
  }

  // Executa a calibracao, caso tenha sido solicitada.

  if (calibrar) {

    calibrar = false;

    if (distancia >= alturaTotalReservatorio) {

      distanciaSensorFundo = distancia;

      filtroInicializado = false;

      Serial.print("Calibracao concluida: ");
      Serial.print(distanciaSensorFundo);
      Serial.println(" cm");

    } else {

      Serial.println("Calibracao recusada.");
      Serial.println("Confira se o tanque esta vazio.");
    }
  }

  // Verifica se a distancia esta fora da faixa
  // esperada para o reservatorio.
  // Margem de 1 cm para pequenas imprecisoes.

  if (distancia > distanciaSensorFundo + 1.0) {

    tratarFalha(FALHA_DISTANCIA);

    return;
  }

  // Aplica o filtro de ruido.

  float distanciaFiltrada = filtrarLeitura(distancia);

  // Calcula a altura da agua.

  float nivelAgua = distanciaSensorFundo
                   - distanciaFiltrada;

  // Limita a altura entre 0 e 100 cm.

  if (nivelAgua < 0) {

    nivelAgua = 0;
  }

  if (nivelAgua > alturaTotalReservatorio) {

    nivelAgua = alturaTotalReservatorio;
  }

  // Calcula a porcentagem interna.

  int porcentagem =
    (nivelAgua / alturaTotalReservatorio) * 100;

  // Classifica o nivel de agua.

  if (porcentagem > 50) {

    definirEstado(ALTO);

  } else if (porcentagem > 20) {

    definirEstado(MEDIO);

  } else {

    definirEstado(BAIXO);
  }

  mostrarAltura(nivelAgua);

  // Registra as informacoes no monitor serial.

  Serial.print("Altura: ");
  Serial.print(nivelAgua, 1);
  Serial.print(" cm | Nivel: ");
  Serial.print(porcentagem);
  Serial.println("%");
}

// =====================================================
// 10. FILTRO DE MEDIANA
// =====================================================

// Utiliza tres leituras para reduzir a influencia
// de valores isolados e instaveis.

float filtrarLeitura(float valor) {

  // Inicializa o filtro com a primeira leitura valida.

  if (!filtroInicializado) {

    for (int i = 0; i < 3; i++) {

      leituras[i] = valor;
    }

    indiceLeitura = 0;

    filtroInicializado = true;
  }

  leituras[indiceLeitura] = valor;

  indiceLeitura = (indiceLeitura + 1) % 3;

  float a = leituras[0];
  float b = leituras[1];
  float c = leituras[2];

  // Organiza os tres valores em ordem crescente.

  if (a > b) {

    float aux = a;
    a = b;
    b = aux;
  }

  if (b > c) {

    float aux = b;
    b = c;
    c = aux;
  }

  if (a > b) {

    float aux = a;
    a = b;
    b = aux;
  }

  // Retorna o valor central.

  return b;
}

// =====================================================
// 11. MAQUINA DE ESTADOS
// =====================================================

void definirEstado(Estado novoEstado) {

  // Evita atualizar os atuadores desnecessariamente.

  if (estadoAtual == novoEstado) {

    return;
  }

  estadoAtual = novoEstado;

  // Desliga todos os indicadores inicialmente.

  digitalWrite(ledVerde, LOW);
  digitalWrite(ledAmarelo, LOW);
  digitalWrite(ledVermelho, LOW);

  noTone(buzzerPin);

  switch (estadoAtual) {

    case ALTO:

      escreverLCD(0, "Nivel: ALTO");

      digitalWrite(ledVerde, HIGH);

      break;

    case MEDIO:

      escreverLCD(0, "Nivel: MEDIO");

      digitalWrite(ledAmarelo, HIGH);

      break;

    case BAIXO:

      escreverLCD(0, "Nivel: BAIXO");

      digitalWrite(ledVermelho, HIGH);

      tone(buzzerPin, 1000);

      break;

    case FALHA_SEM_ECO:

      escreverLCD(0, "Erro no sensor");
      escreverLCD(1, "Sem eco");

      digitalWrite(ledVermelho, HIGH);

      tone(buzzerPin, 1000);

      break;

    case FALHA_DISTANCIA:

      escreverLCD(0, "Erro no sensor");
      escreverLCD(1, "Fora da faixa");

      digitalWrite(ledVermelho, HIGH);

      tone(buzzerPin, 1000);

      break;

    case INICIALIZANDO:

      escreverLCD(0, "Sistema Nivel");
      escreverLCD(1, "Iniciando...");

      break;
  }
}

// =====================================================
// 12. TRATAMENTO DE FALHAS
// =====================================================

void tratarFalha(Estado falha) {

  // Reinicia o filtro para evitar o uso de
  // leituras antigas apos a recuperacao.

  filtroInicializado = false;

  if (calibrar) {

    calibrar = false;

    Serial.println("Falha durante a calibracao.");
  }

  definirEstado(falha);

  if (falha == FALHA_SEM_ECO) {

    Serial.println("ERRO: ausencia de eco");

  } else {

    Serial.println("ERRO: distancia fora da faixa");
  }
}

// =====================================================
// 13. EXIBICAO DA ALTURA NO DISPLAY
// =====================================================

void mostrarAltura(float altura) {

  char mensagem[17];

  // Converte a altura em decimos de centimetro.

  int decimos = (int)(altura * 10 + 0.5);

  // Formata o texto com uma casa decimal,
  // respeitando as 16 colunas do LCD.

  snprintf(
    mensagem,
    sizeof(mensagem),
    "Altura: %d.%d cm",
    decimos / 10,
    decimos % 10
  );

  escreverLCD(1, mensagem);
}

// =====================================================
// 14. ESCRITA SEGURA NO LCD
// =====================================================

// Completa a linha com espacos para evitar
// caracteres restantes de mensagens anteriores.

void escreverLCD(int linha, const char* texto) {

  lcd.setCursor(0, linha);

  bool terminou = false;

  for (int i = 0; i < 16; i++) {

    if (!terminou && texto[i] == '\0') {

      terminou = true;
    }

    if (terminou) {

      lcd.print(' ');

    } else {

      lcd.print(texto[i]);
    }
  }
}

// =====================================================
// 15. MODO DE CALIBRACAO
// =====================================================

// Ao receber a letra C pelo monitor serial,
// utiliza a proxima medicao valida para calibrar
// a distancia entre o sensor e o fundo do tanque.

void verificarComandos() {

  if (Serial.available() > 0) {

    char comando = Serial.read();

    if (comando == 'C' || comando == 'c') {

      calibrar = true;

      Serial.println("Calibracao solicitada.");
    }
  }
}

// =====================================================
// FIM DO PROGRAMA
// =====================================================