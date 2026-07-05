# IBE Mail

Trabalho final da disciplina de Segurança em Computação (UFSC) — implementação de
esquemas/protocolos criptográficos. O tema escolhido foi a **Criptografia Baseada em
Identidade (IBE)**, especificamente o esquema **FullIdent de Boneh-Franklin** (2001).

O projeto contém três partes:

- **`implementation/`** — o código-fonte do IBE Mail, um cliente de e-mail com interface
  gráfica que demonstra o esquema FullIdent de ponta a ponta.
- **`rel-1/`** e **`rel-2/`** — os relatórios em LaTeX entregues nas duas etapas do
  trabalho (parcial e final).
- **`docs/`** — a definição do trabalho, a literatura de referência (Boneh-Franklin 2001,
  slides do NIST, introdução a pairing-based crypto) e o plano técnico da implementação.

## O que o IBE Mail demonstra

O IBE Mail implementa os **4 algoritmos** do esquema FullIdent (Setup, Extract, Encrypt,
Decrypt) com a transformação de Fujisaki-Okamoto para segurança CCA2. A aplicação roda
localmente, com múltiplas identidades na mesma máquina e um PKG (*Private Key Generator*)
embutido, e mostra na prática:

- **Cifrar para uma identidade sem chave pública prévia:** qualquer string (e-mail, e-mail
  com data, etc.) pode ser usada como chave pública.
- **Extração de chave privada pelo PKG:** um administrador central aprova identidades e
  gera as chaves privadas matemáticas.
- **Decifragem com verificação de consistência:** o teste $U \stackrel{?}{=} r' \cdot P$
  rejeita cifrotextos alterados.
- **Identidades temporais:** cifrar para `alice@ufsc.br|2026-06` permite revogação natural
  por período.
- **Revogação de identidades:** o PKG pode revogar um usuário, apagando sua chave do disco
  e bloqueando novas extrações.

## Requisitos

- **C++17 Compiler** (`g++`) e **Make**.
- **GMP (GNU Multiple Precision Arithmetic):** usada pela PBC para números de grande porte.
- **OpenSSL (`libcrypto`):** fornece o SHA-256 usado nos oráculos aleatórios H1–H4.
- **SDL2 & OpenGL 3:** janela e aceleração gráfica da interface.
- **PBC (Pairing-Based Cryptography):** biblioteca acadêmica para emparelhamentos
  bilineares sobre curvas elípticas supersingulares. Não costuma estar nos repositórios
  oficiais das distros — veja `make setup` abaixo.
- **Dear ImGui:** já incluída no repositório em `implementation/imgui/`.

## Instalação e Compilação

### 1. Dependências do sistema

**Debian/Ubuntu:**
```bash
sudo apt-get update
sudo apt-get install build-essential flex bison libgmp-dev libssl-dev libsdl2-dev wget
```

**Arch Linux:**
```bash
sudo pacman -S base-devel gmp openssl sdl2
```

### 2. Instalar a biblioteca PBC

A PBC não está nos repositórios oficiais de nenhuma distro comum. Em vez de pedir para
instalar via AUR/PPA, o projeto resolve isso sozinho: **`make build` detecta se falta a
PBC e, antes de compilar, avisa e a instala automaticamente**, baixando o código-fonte e
compilando `pbc-0.5.14` **localmente dentro do próprio projeto**, em
`implementation/third_party/local/` — nada é instalado no sistema e não é preciso `sudo`
para esse passo (a GMP em si é usada a partir do pacote de sistema instalado no passo 1).

Isso também pode ser disparado manualmente:
```bash
make setup
```

`setup` roda `scripts/setup-pbc.sh`, que pula o trabalho todo se a PBC já estiver
disponível (local ou no sistema). O `Makefile` já aponta `-I`/`-L` para esse prefixo
local, então a compilação funciona sem nenhum passo extra.

### 3. Compilar

Na raiz do repositório:
```bash
make build
```

Isso compila **apenas o código da aplicação** (não os relatórios LaTeX). A saída mostra
cada arquivo compilado, no estilo do kernel Linux:
```
 CXX    implementation/src/gui/App.cpp
 CXX    implementation/src/common/BonehFranklin.cpp
 ...
 LD     implementation/ibemail_app
```

### 4. Rodar

```bash
make run
```

Compila (se necessário) e executa `implementation/ibemail_app`. Também é possível rodar o
binário diretamente depois de compilado:
```bash
./implementation/ibemail_app
```

### 5. Compilar os relatórios (opcional)

```bash
make reports      # compila rel-1/ e rel-2/ (LaTeX)
make rel-1        # só o relatório parcial
make rel-2        # só o relatório final
```

### 6. Limpar artefatos

```bash
make clean            # objetos e binário da aplicação
make clean-reports    # artefatos de compilação do LaTeX
make veryclean         # tudo (aplicação + relatórios)
```

## Usuários de demonstração pré-configurados

Na primeira execução (quando `implementation/data/` ainda não tem `master.sec`), o PKG
gera os parâmetros do sistema e já registra, aprova e extrai a chave privada de dois
usuários, para que a demonstração comece sem precisar passar pelo fluxo de registro:

| Identidade        | Senha    | Status   |
|-------------------|----------|----------|
| `alice@ufsc.br`   | `123123` | APPROVED |
| `bob@ufsc.br`     | `123123` | APPROVED |

Basta abrir o app, fazer login como uma delas e já dá pra compor/ler e-mails cifrados.
Novos usuários continuam podendo ser registrados normalmente pela aba de autenticação, e
precisam de aprovação manual no PKG Admin como antes.

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
├── imgui/               # Dear ImGui (vendored)
└── data/                # Dados gerados em tempo de execução (ignorados pelo git)
    ├── master.sec       # Segredo mestre do PKG (Base64)
    ├── params.pub       # Parâmetros públicos P||P_pub (Base64)
    ├── registry.dat     # Registro de identidades (ID \t hash \t status)
    ├── keys/            # Chaves privadas extraídas (ID.prv)
    └── mail/
        ├── inbox/       # Mensagens recebidas por identidade
        └── sent/        # Cópias das mensagens enviadas

docs/
├── problem/             # Definição do trabalho (enunciado do professor)
├── literature/          # Boneh-Franklin 2001, slides NIST, Menezes 2009
├── examples/            # Exemplos de trabalhos de anos anteriores
└── plan/                # Design técnico da implementação

rel-1/                   # Relatório parcial (LaTeX + PDF)
rel-2/                   # Relatório final (LaTeX + PDF)

scripts/
└── setup-pbc.sh         # Instalador da biblioteca PBC (usado por `make setup`)
```

## Guia de Uso — Primeiro Uso Completo

### Passo 1: Login com um usuário pré-configurado
1. Abra o aplicativo (`make run`).
2. Na aba **Authentication / Registration**, entre com `alice@ufsc.br` / `123123` (ou
   `bob@ufsc.br` / `123123`).
3. Clique em **Login / Extract Key**. A barra de status ficará verde:
   *"Authenticated as alice@ufsc.br"*.
4. As abas **Inbox**, **Sent** e **Compose** aparecerão automaticamente.

### Passo 2: Registrar um novo usuário (fluxo completo, opcional)
1. Preencha o campo **ID (Email)** com uma identidade qualquer (ex: `carol@ufsc.br`).
2. Preencha o campo **Password** com uma senha.
3. Clique em **Register**. A barra de status mostrará: *"Registered successfully. Waiting
   for PKG approval."*
4. Clique na aba **PKG Admin (Debug)** e clique em **Approve** ao lado do usuário.
5. Volte para a aba de autenticação e faça login normalmente.

### Passo 3: Enviar um e-mail cifrado
1. Clique na aba **Compose**.
2. Preencha o campo **To** com a identidade do destinatário (pode ser `bob@ufsc.br`, outro
   usuário registrado, ou você mesmo).
3. Preencha **Subject** e **Body**.
4. Clique em **Send Encrypted Email**. A mensagem será cifrada com o esquema FullIdent e
   salva em disco.

### Passo 4: Ler um e-mail na Inbox
1. Clique na aba **Inbox**.
2. Na lista da esquerda, clique no ID da mensagem.
3. O painel direito mostrará a mensagem **decifrada** (verde), seguida do **arquivo
   .ibemail bruto** (laranja) mostrando os blocos cifrados $\langle U, V, W \rangle$ em
   Base64.

### Passo 5: Verificar a aba Sent
1. Clique na aba **Sent** para ver as mensagens enviadas.
2. Os cabeçalhos (To, Date) são exibidos, mas o conteúdo permanece cifrado — somente o
   destinatário pode decifrá-lo.

## Roteiro de Testes Práticos (Validação)

### Teste 1: Envio e recebimento básico
1. Faça login como `alice@ufsc.br` e envie um e-mail para `bob@ufsc.br`.
2. Faça logout, faça login como `bob@ufsc.br` e verifique que o e-mail aparece na Inbox e
   é decifrado corretamente.

### Teste 2: Chave errada (segurança do esquema)
1. Envie um e-mail para `bob@ufsc.br` usando `alice@ufsc.br`.
2. Faça login como um terceiro usuário (registre um novo, ex: `eve@ufsc.br`), que possui
   uma chave privada diferente.
3. Eve **não** terá a mensagem na sua Inbox (ela foi roteada para a pasta de Bob). Isso
   demonstra que apenas o detentor da chave privada correta pode acessar as mensagens.

### Teste 3: Identidades Temporais (Time-based Identities)
A grande vantagem do IBE é poder cifrar mensagens para chaves que **ainda não existem**.
1. Faça login como `alice@ufsc.br` e envie um e-mail para `bob@ufsc.br|2026-12` — uma
   identidade que ainda não foi registrada.
2. A mensagem será cifrada perfeitamente usando apenas a string do destinatário como chave
   pública.
3. Agora registre e aprove `bob@ufsc.br|2026-12` no PKG Admin.
4. Faça login como `bob@ufsc.br|2026-12` e verifique que o e-mail é decifrado
   retroativamente!

### Teste 4: Rejeição CCA2 (Ciphertext Indistinguishability)
O esquema FullIdent é imune a ataques de texto cifrado escolhido (IND-ID-CCA).
1. Envie um e-mail para você mesmo. Na aba Inbox, clique na mensagem — ela será decifrada
   com sucesso.
2. Use um editor de texto para abrir o arquivo `.ibemail` correspondente em
   `implementation/data/mail/inbox/[seu-email]/[ID].ibemail`.
3. Altere **um único caractere** na linha `U:` (troque uma letra qualquer). Salve o
   arquivo.
4. Volte para o aplicativo e clique novamente sobre a mensagem na Inbox.
5. O aplicativo **rejeitará** o cifrotexto corrompido, exibindo a mensagem vermelha
   *"Decryption error (Invalid Ciphertext or Wrong Key)"*, provando a integridade CCA2 da
   transformação de Fujisaki-Okamoto.

### Teste 5: Revogação de identidades
1. No **PKG Admin**, clique em **Revoke** ao lado de um usuário aprovado.
2. O status muda para **REVOKED** e a chave privada do disco (`data/keys/ID.prv`) é
   apagada automaticamente.
3. Tente fazer login novamente com esse usuário — a extração de chave será bloqueada pelo
   PKG.
