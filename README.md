# Caça ao Tesouro — CI1058 Redes 1 (2025/1)

**Universidade Federal do Paraná**  
**Curso:** Bacharelado em Ciência da Computação  
**Disciplina:** CI1058 - Redes 1  
**Professores:** Luiz Albini e Eduardo Todt  
**Trabalho T1 — Versão 20250424**  

---

## Descrição
Este projeto implementa um jogo de caça ao tesouro em rede, utilizando um modelo **cliente-servidor** com comunicação em **nível 2 (camada de enlace)** via *raw sockets*.  

- **Servidor:** Controla o mapa, sorteia/manda tesouros e registra movimentos.  
- **Cliente:** Exibe um grid 8x8 onde o jogador navega para encontrar tesouros (arquivos de texto, imagem ou vídeo).  

**Requisitos:**  
- Execução em máquinas distintas conectadas por cabo de rede.  
- Tesouros são arquivos armazenados em `./objetos/` (formato: `1.txt`, `2.mp4`, etc.).  

---

## Objetivos Técnicos
1. **Protocolo Kermit Customizado:**  
   - Comunicação confiável com controle de fluxo *stop-and-wait*.  
   - Tipos de mensagem definidos (ACK, NACK, dados, movimentos, etc.).  

2. **Transferência de Arquivos:**  
   - Tesouros são transferidos do servidor para o cliente e exibidos conforme o tipo:  
     - `.txt`: Aberto em editor (ex: `xed`).  
     - `.jpg`: Exibido com visualizador (ex: `eog`).  
     - `.mp4`: Reproduzido com player (ex: `vlc`).  

3. **Interface do Jogo:**  
   - Grid 8x8 com marcação de posições visitadas (`X` = jogador, `1` = não visitado, `2` = visitado, `3` = tesouro encontrado).  

---

## Regras do Jogo
- **Mapa:** Grid 8x8 com origem em `(0,0)` (canto inferior esquerdo).  
- **Tesouros:** 8 arquivos posicionados aleatoriamente pelo servidor.  
- **Movimentos:**  
  - Teclas `W` (cima), `A` (esquerda), `S` (baixo), `D` (direita).  
  - Movimentos inválidos (fora do grid) são ignorados.  

---

## Protocolo de Comunicação
### Estrutura do Frame (Kermit)
| Campo          | Tamanho  | Descrição                          |
|----------------|----------|------------------------------------|
| Marcador       | 8 bits   | `01111110` (0x7E)                 |
| Tamanho        | 7 bits   | Tamanho dos dados (0-127 bytes)   |
| Sequência      | 5 bits   | Número de sequência               |
| Tipo           | 4 bits   | Tipo da mensagem (ACK, dados, etc.) |
| Checksum       | 8 bits   | Soma de verificação               |
| Dados          | 0-127 Bytes  | Conteúdo da mensagem              |

### Tipos de Mensagem
| Código | Tipo de Mensagem          | Descrição                                                                 |
|--------|---------------------------|---------------------------------------------------------------------------|
| 0      | ACK                       | Confirmação de recebimento (Acknowledgment)                              |
| 1      | NACK                      | Confirmação negativa (Negative Acknowledgment)                           |
| 2      | OK_AND_ACK                  | Confirmação positiva + ACK                                              |
| 3      | STOP_GAME                     | Finalização de jogo                                              |
| 4      | FILE_SIZE                  | Indica o tamanho de um arquivo (em bytes)                               |
| 5      | DATA                    | Contém dados do arquivo sendo transferido                               |
| 6      | TXT_FILE_NAME_ACK        | Nome do arquivo de texto + confirmação                                  |
| 7      | VIDEO_FILE_NAME_ACK        | Nome do arquivo de vídeo + confirmação                                  |
| 8      | IMG_FILE_NAME_ACK       | Nome do arquivo de imagem + confirmação                                 |
| 9      |END_OF_FILE            | Indica o término da transferência de um arquivo                         |
| 10     |MOVE_RIGHT             | Comando para mover o jogador para a direita no grid                     |
| 11     |  MOVE_UP               | Comando para mover o jogador para cima no grid                          |
| 12     | MOVE_DOWN               | Comando para mover o jogador para baixo no grid                         |
| 13     | MOVE_LEFT            | Comando para mover o jogador para a esquerda no grid                    |
| 14     | INVERT                    | Inverte a ordem de recebimento de mensagens                                          |
| 15     | ERROR                    | Indica um erro (ex: sem permissão, espaço insuficiente, etc.)           |

---

## Implementação
### Arquivos Principais
- **`Server.cpp`/`Server.hpp`:** Lógica do servidor.  
- **`Client.cpp`/`Client.hpp`:** Interface do jogador e exibição de tesouros.  
- **`KermitProtocol.cpp`/`KermitProtocol.hpp`:** Implementação do protocolo customizado e comunicação.  
- **`Buffer.cpp`/`Buffer.hpp`:** Manipulação de buffers para transferência de arquivos.  

### Funcionalidades Chave
- **Controle de Fluxo:** stop-and-wait com timeout.  
- **Tratamento de Erros:** Verificação de checksum e retransmissão.  

---

## Execução
1. **Servidor:**  
   ```bash
   sudo ./bin/src/server

2. **Cliente:**  
    ```bash
    sudo ./bin/src/client