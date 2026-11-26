# Sistema-de-Monitoramento-do-N-vel-de-gua-em-Reservat-rios
**Descrição:**  Sistema desenvolvido para monitorar o nível de água em reservatórios, utilizando Arduino e sensores para identificar a quantidade de água armazenada e auxiliar no controle do abastecimento, evitando transbordamentos e falta de água.

## Sobre o projeto

Projeto desenvolvido na disciplina de Sistemas Embarcados, do curso de Desenvolvimento de Sistemas da EEEP Professora Maria Célia Pinheiro Falcão.

O sistema utiliza Arduino Uno e um sensor ultrassônico para monitorar o nível de água em reservatórios, apresentando informações em um display LCD e emitindo alertas visuais e sonoros.

## Objetivo

Facilitar o monitoramento do nível de água, reduzir a necessidade de verificações manuais e ajudar a prevenir a falta de abastecimento.

## Público-alvo

- Residências;
- Produtores rurais;
- Estabelecimentos comerciais.

## Componentes utilizados

| Componente | Quantidade |
|---|---:|
| Arduino Uno R3 | 1 |
| Sensor ultrassônico HC-SR04 | 1 |
| Display LCD 16x2 I2C | 1 |
| LED verde | 1 |
| LED amarelo | 1 |
| LED vermelho | 1 |
| Buzzer | 1 |
| Resistores de 330 Ω | 3 |
| Protoboard | 1 |
| Jumpers | Diversos |

## Funcionamento

O sensor ultrassônico mede a distância até a superfície da água. O Arduino processa essa informação, calcula a altura da água e classifica o nível do reservatório.

| Estado | Nível | LED | Buzzer |
|---|---|---|---|
| Alto | 51% a 100% | Verde | Desligado |
| Médio | 21% a 50% | Amarelo | Desligado |
| Baixo | 0% a 20% | Vermelho | Ligado |

O display LCD apresenta a classificação do nível e a altura da água em centímetros.

## Tratamento de falhas

- Ausência de eco: o sensor não recebe o sinal de retorno dentro do tempo limite.
- Distância inválida: a leitura está fora dos limites aceitos pelo sistema.

Em ambas as situações, o sistema apresenta uma mensagem de erro no LCD e aciona o LED vermelho e o buzzer.

## Tecnologias utilizadas

- Arduino
- C/C++
- Tinkercad Circuits
- GitHub

## Links do projeto

- [Simulação no Tinkercad](INSERIR_LINK)
- [Vídeo de demonstração](INSERIR_LINK)
- [Documentação do projeto](INSERIR_LINK)

## Integrantes da equipe

| Integrante | Função |
|---|---|
| Antônio Lourenço Aquino Moreira | Programação |
| David Lucas Carlos Sousa | Gerenciamento dos kits |
| Fabricio Gonçalves da Silva | Documentação |
| Fernando Aquilis Desiderio Pinheiro | Montagem física |
| Kaick Aires Fortunato | Coordenação e montagem no Tinkercad |
| Pedro Emanuel Soares Regis | Montagem física |

## Professor orientador

Daniel Mota Vidal

## Etapas do projeto

- [x] Parte 1 — Desenvolvimento e simulação no Tinkercad.
- [ ] Parte 2 — Construção e validação do produto físico.

---

EEEP Professora Maria Célia Pinheiro Falcão

Desenvolvimento de Sistemas — 2026
