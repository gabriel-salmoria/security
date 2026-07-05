# IBE Mail

Aplicação de correio eletrônico que demonstra a **Criptografia Baseada em Identidade (IBE)**, especificamente o esquema **FullIdent de Boneh-Franklin**. Escrito em C++17 com interface gráfica via Dear ImGui e operações criptográficas sobre a biblioteca PBC.

## O que o trabalho demonstra

O IBE Mail implementa os **4 algoritmos** do esquema FullIdent (Setup, Extract, Encrypt, Decrypt) com a transformação de Fujisaki-Okamoto para segurança CCA2. A aplicação demonstra na prática:

- **Cifrar para uma identidade sem chave pública prévia:** qualquer string (e-mail, e-mail com data, etc.) pode ser usada como chave pública.
- **Extração de chave privada pelo PKG:** um administrador central aprova identidades e gera as chaves privadas matemáticas.
- **Decifragem com verificação de consistência:** o teste $U \stackrel{?}{=} r' \cdot P$ rejeita cifrotextos alterados.
- **Identidades temporais:** cifrar para `alice@ufsc.br|2026-06` permite revogação natural por período.
- **Revogação de identidades:** o PKG pode revogar um usuário, apagando sua chave do disco e bloqueando novas extrações.

## Requisitos e Bibliotecas

O projeto depende das seguintes ferramentas e bibliotecas:
- **C++17 Compiler** (`g++`) e **Make**.
- **GMP (GNU Multiple Precision Arithmetic):** Utilizada pela PBC para lidar com números de grande porte.
- **OpenSSL (`libcrypto`):** Fornece a função de hash SHA-256 usada nos oráculos aleatórios H1–H4.
- **SDL2 & OpenGL 3:** Gerenciamento de janela e aceleração gráfica da interface.
- **PBC (Pairing-Based Cryptography):** Biblioteca acadêmica para emparelhamentos bilineares em curvas elípticas supersingulares. Deve ser compilada a partir da fonte.
- **Dear ImGui:** Biblioteca de interface gráfica imediata (já incluída no repositório como submódulo em `implementation/imgui/`).

## Instalação e Configuração

Siga o passo a passo abaixo (ambiente testado em Ubuntu/Debian):

### 1. Instalar as dependências do sistema
```bash
sudo apt-get update
sudo apt-get install build-essential flex bison libgmp-dev libssl-dev libsdl2-dev wget
```

### 2. Instalar a biblioteca PBC
A biblioteca PBC exige compilação manual:
```bash
wget https://crypto.stanford.edu/pbc/files/pbc-0.5.14.tar.gz
tar -xvf pbc-0.5.14.tar.gz
cd pbc-0.5.14
./configure
make
sudo make install
sudo ldconfig
cd ..
```

### 3. Compilar a aplicação
Na raiz do repositório (pasta `security`), execute:
```bash
make build_app
```
*(Nota: `make` sem argumentos também compilará os relatórios LaTeX. Use `make build_app` para compilar apenas a aplicação.)*

### 4. Rodar o executável
```bash
./implementation/ibemail_app
```

### 5. Limpar artefatos de compilação
```bash
make clean_app   # Limpa apenas os objetos e o executável da aplicação
make clean        # Limpa tudo (aplicação + relatórios)
```

## Estrutura do Projeto

```
implementation/
├── src/
│   ├── common/          # Primitivas criptográficas
│   │   ├── BonehFranklin.cpp/hpp   # Setup, Extract, Encrypt, Decrypt
│   │   ├── Hash.cpp/hpp            # H1, H2, H3, H4, SHA-256
│   │   └── Utils.cpp/hpp           # UUID v4, Base64, XOR
│   ├── core/            # Lógica de negócio
│   │   ├── PKG.cpp/hpp             # Registro, autenticação, extração
│   │   └── MailSystem.cpp/hpp      # Formato .ibemail, roteamento
│   ├── gui/             # Interface gráfica
│   │   └── App.cpp/hpp             # Abas: Auth, Inbox, Sent, Compose, PKG Admin
│   └── main.cpp         # Ponto de entrada, inicialização SDL2/ImGui
├── imgui/               # Dear ImGui (submódulo)
└── data/                # Dados gerados em tempo de execução (ignorados pelo git)
    ├── master.sec       # Segredo mestre do PKG (Base64)
    ├── params.pub       # Parâmetros públicos P||P_pub (Base64)
    ├── registry.dat     # Registro de identidades (ID \t hash \t status)
    ├── keys/            # Chaves privadas extraídas (ID.prv)
    └── mail/
        ├── inbox/       # Mensagens recebidas por identidade
        └── sent/        # Cópias das mensagens enviadas
```

## Guia de Uso — Primeiro Uso Completo

### Passo 1: Registrar um usuário
1. Ao abrir o aplicativo, você verá a aba **Authentication / Registration**.
2. Preencha o campo **ID (Email)** com uma identidade qualquer (ex: `alice@ufsc.br`).
3. Preencha o campo **Password** com uma senha.
4. Clique em **Register**. A barra de status mostrará: *"Registered successfully. Waiting for PKG approval."*

### Passo 2: Aprovar o usuário no PKG
1. Clique na aba **PKG Admin (Debug)**.
2. Você verá o usuário recém-registrado com status **PENDING**.
3. Clique no botão **Approve** ao lado dele. O status mudará para **APPROVED**.

### Passo 3: Fazer login e extrair a chave privada
1. Volte para a aba **Authentication / Registration**.
2. Preencha o ID e senha do usuário aprovado.
3. Clique em **Login / Extract Key**. A barra de status ficará verde: *"Authenticated as alice@ufsc.br"*.
4. As abas **Inbox**, **Sent** e **Compose** aparecerão automaticamente.

### Passo 4: Enviar um e-mail cifrado
1. Clique na aba **Compose**.
2. Preencha o campo **To** com a identidade do destinatário (pode ser outro usuário ou você mesmo).
3. Preencha **Subject** e **Body**.
4. Clique em **Send Encrypted Email**. A mensagem será cifrada com o esquema FullIdent e salva em disco.

### Passo 5: Ler um e-mail na Inbox
1. Clique na aba **Inbox**.
2. Na lista da esquerda, clique no ID da mensagem.
3. O painel direito mostrará a mensagem **decifrada** (verde), seguida do **arquivo .ibemail bruto** (laranja) mostrando os blocos cifrados $\langle U, V, W \rangle$ em Base64.

### Passo 6: Verificar a aba Sent
1. Clique na aba **Sent** para ver as mensagens enviadas.
2. Os cabeçalhos (To, Date) são exibidos, mas o conteúdo permanece cifrado — somente o destinatário pode decifrá-lo.

## Roteiro de Testes Práticos (Validação)

### Teste 1: Envio e recebimento básico
1. Registre e aprove dois usuários: `alice@ufsc.br` e `bob@ufsc.br`.
2. Faça login como Alice e envie um e-mail para `bob@ufsc.br`.
3. Faça logout, faça login como Bob e verifique que o e-mail aparece na Inbox e é decifrado corretamente.

### Teste 2: Chave errada (segurança do esquema)
1. Envie um e-mail para `bob@ufsc.br` usando Alice.
2. Faça login como um terceiro usuário (ex: `eve@ufsc.br`), que possui uma chave privada diferente.
3. Eve **não** terá a mensagem na sua Inbox (ela foi roteada para a pasta de Bob). Isso demonstra que apenas o detentor da chave privada correta pode acessar as mensagens.

### Teste 3: Identidades Temporais (Time-based Identities)
A grande vantagem do IBE é poder cifrar mensagens para chaves que **ainda não existem**.
1. Faça login como Alice e envie um e-mail para `bob@ufsc.br|2026-12` — uma identidade que ainda não foi registrada.
2. A mensagem será cifrada perfeitamente usando apenas a string do destinatário como chave pública.
3. Agora registre e aprove `bob@ufsc.br|2026-12` no PKG Admin.
4. Faça login como `bob@ufsc.br|2026-12` e verifique que o e-mail é decifrado retroativamente!

### Teste 4: Rejeição CCA2 (Ciphertext Indistinguishability)
O esquema FullIdent é imune a ataques de texto cifrado escolhido (IND-ID-CCA).
1. Envie um e-mail para você mesmo. Na aba Inbox, clique na mensagem — ela será decifrada com sucesso.
2. Use um editor de texto para abrir o arquivo `.ibemail` correspondente em `implementation/data/mail/inbox/[seu-email]/[ID].ibemail`.
3. Altere **um único caractere** na linha `U:` (troque uma letra qualquer). Salve o arquivo.
4. Volte para o aplicativo e clique novamente sobre a mensagem na Inbox.
5. O aplicativo **rejeitará** o cifrotexto corrompido, exibindo a mensagem vermelha *"Decryption error (Invalid Ciphertext or Wrong Key)"*, provando a integridade CCA2 da transformação de Fujisaki-Okamoto.

### Teste 5: Revogação de identidades
1. No **PKG Admin**, clique em **Revoke** ao lado de um usuário aprovado.
2. O status muda para **REVOKED** e a chave privada do disco (`data/keys/ID.prv`) é apagada automaticamente.
3. Tente fazer login novamente com esse usuário — a extração de chave será bloqueada pelo PKG.
