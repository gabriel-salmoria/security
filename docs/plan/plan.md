# IBE Mail — Technical Design

## What we are building

A graphical email client that demonstrates the Boneh-Franklin IBE scheme end-to-end.
One application, one window. It simulates a multi-user environment: multiple identities
live on the same machine and can be switched in the UI. An embedded PKG handles
registration, authentication, key extraction, and revocation — no separate server process.

The point of the demo is to show that:
- Any party can encrypt to an identity string without that person's certificate or prior contact
- Only the holder of the correct private key (issued by the PKG after authentication) can decrypt
- Wrong key → decryption fails visibly
- Revoking an identity at the PKG prevents future key extraction while existing keys keep working
- Temporal identities (`alice@ufsc.br|2026-06`) give time-scoped access for free

---

## IBE Scheme

**FullIdent** (Boneh-Franklin §4.2) — IND-ID-CCA secure in the random oracle model under BDH.

Four hash functions:
- `H1 : {0,1}* → G1*`  — identity string → curve point (via `element_from_hash`)
- `H2 : GT → {0,1}^n`  — pairing output → byte string (via `element_to_bytes` + SHA-256)
- `H3 : {0,1}^n × {0,1}^n → Zq*` — (σ, M) → scalar (via SHA-256, interpreted mod q)
- `H4 : {0,1}^n → {0,1}^n`  — bitstring → bitstring (SHA-256)

**Ciphertext**: `C = ⟨U, V, W⟩` where:
- `r = H3(σ, M)`, σ random in `{0,1}^n`
- `U = rP`
- `V = σ ⊕ H2(e(Q_ID, P_pub)^r)`
- `W = M ⊕ H4(σ)`

**Decryption** recovers σ from V using `e(d_ID, U)`, recovers M from W, then verifies
`r = H3(σ, M)` and `U = rP` — rejects if either check fails (this is the CCA hardening).

**Curve**: Type A (supersingular, `y² = x³ + x` over `F_p`, embedding degree k=2).
Symmetric pairing — `G1 = G2`. Parameters loaded from PBC's bundled `a.param`.

---

## Tech Stack

| Concern        | Library / Tool         |
|----------------|------------------------|
| Pairing math   | PBC (libpbc)           |
| Big integers   | GMP (libgmp)           |
| SHA-256        | OpenSSL (libcrypto)    |
| GUI            | Dear ImGui             |
| Window/input   | SDL2                   |
| Rendering      | OpenGL 3               |
| Build          | Makefile               |
| Language       | C++17                  |

Compile:
```
g++ src/**/*.cpp third_party/imgui/*.cpp -o ibemail \
    -lpbc -lgmp -lcrypto -lSDL2 -lGL -ldl -Ithird_party/imgui
```

---

## UI Layout

```
┌─────────────────────────────────────────────────────────────────┐
│  IBE Mail                                  PKG: Online  ■ ■ □  │
├───────────┬────────────────────────┬────────────────────────────┤
│ Users     │ Inbox — bob@ufsc.br  3 │ Message                    │
│           │                        │                            │
│ ● alice   │ alice   Oi Bob!  14:30 │ From:  alice@ufsc.br       │
│ ● bob  ◄  │ carol   Reunião  13:15 │ To:    bob@ufsc.br         │
│ ● carol   │ alice   Arquivo  12:00 │ Date:  2026-06-21 14:30    │
│           │                        │ Subj:  Oi Bob!             │
│ [+ Reg]   │                        │ ────────────────────────── │
│           │                        │ Tudo bem por aqui.         │
├───────────┤                        │ Precisamos falar sobre o   │
│ PKG       │                        │ projeto de segurança...    │
│           │                        │                            │
│ [Approve] │                        │  [Reply]  [Delete]         │
│ [Revoke]  │                        │                            │
│ [Keys]    │                        │ 🔒 Decrypted with          │
│           │                        │    bob@ufsc.br             │
└───────────┴────────────────────────┴────────────────────────────┘
│  To: [alice@ufsc.br      ]  Subject: [Re: Projeto  ]  [Compose] │
└─────────────────────────────────────────────────────────────────┘
```

Three-panel layout (fixed):
- **Left**: user list with status badges (has key / no key / revoked), PKG control buttons
- **Center**: message list for the active user's inbox or sent folder, tab to switch
- **Right**: decrypted message view, or failure notice if wrong/missing key
- **Bottom bar**: quick compose fields always visible

Modals:
- **Compose** — full editor: To (free text identity string), Subject, Body, Send button
- **PKG Management** — table of registered users with status; approve/revoke/view key buttons
- **Register** — new identity + password fields → submits to PKG registry

---

## Data Layout (runtime, created on first run)

```
data/
  params.pub          — system public parameters (base64 text)
  master.sec          — PKG master key (base64 text, PKG eyes only)
  registry.dat        — tab-separated: identity | password_sha256 | status
  keys/
    alice@ufsc.br.prv — private key for alice (base64 encoded G1 point)
    bob@ufsc.br.prv
  mail/
    inbox/
      alice@ufsc.br/
        <uuid>.ibemail
      bob@ufsc.br/
        <uuid>.ibemail
    sent/
      alice@ufsc.br/
        <uuid>.ibemail
```

### `.ibemail` format

Plain text file, headers + base64 ciphertext blocks:

```
IBE-MAIL/1.0
Message-ID: 550e8400-e29b-41d4-a716-446655440000
From: alice@ufsc.br
To: bob@ufsc.br
Date: 2026-06-21T14:30:22
---
U: <base64 of G1 point>
V: <base64 of n bytes>
W: <base64 of n bytes>
```

Subject and body are both inside the encrypted payload — nothing leaks from the envelope
except sender, recipient, and timestamp. This is a deliberate choice noted in the report.

### `registry.dat` format

```
alice@ufsc.br   a665a45920422f9d417e4867efdc4fb8a04a1f3fff1fa07e998e86f7f7a27ae3   approved
bob@ufsc.br     5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8   approved
carol@ufsc.br   <hash>   pending
```

---

## Source File Structure

```
implementation/
  src/
    common/
      ibe.hpp / ibe.cpp           — Setup, Extract, Encrypt, Decrypt (FullIdent)
      crypto.hpp / crypto.cpp     — H1, H2, H3, H4
      serialize.hpp / serialize.cpp — base64, element_t ↔ bytes, .ibemail parse/write
      utils.hpp / utils.cpp       — XOR, UUID, timestamp, file I/O helpers

    core/
      pkg.hpp / pkg.cpp           — PKG logic: setup, load params, extract, revoke
      registry.hpp / registry.cpp — registry.dat: load, save, register, approve, revoke, lookup
      mailbox.hpp / mailbox.cpp   — drop message into inbox, list folder, fetch by id, delete
      message.hpp / message.cpp   — Envelope struct, parse .ibemail, render headers

    gui/
      app.hpp / app.cpp           — AppState struct, main render() call, layout orchestration
      panel_users.hpp / panel_users.cpp   — left panel
      panel_inbox.hpp / panel_inbox.cpp   — center panel
      panel_reader.hpp / panel_reader.cpp — right panel
      panel_compose.hpp / panel_compose.cpp — compose modal
      panel_pkg.hpp / panel_pkg.cpp       — PKG management modal
      widgets.hpp / widgets.cpp           — StatusBadge, KeyIndicator, Toast notifications

    main.cpp   — SDL2 + OpenGL3 init, ImGui context, event loop, shutdown

  third_party/
    imgui/     — vendored: imgui.h, imgui.cpp, imgui_draw.cpp, imgui_tables.cpp,
                           imgui_widgets.cpp, backends/imgui_impl_sdl2.cpp,
                           backends/imgui_impl_opengl3.cpp

  Makefile
```

---

## Application State (`AppState`)

Central struct owned by `app.cpp`, passed by pointer to every panel:

```cpp
struct AppState {
    // PKG
    Params      params;        // public system parameters
    MasterKey   master;        // master secret (s)
    Registry    registry;      // loaded from registry.dat

    // Active session
    std::string active_user;   // identity currently "logged in"
    PrivateKey  active_key;    // d_ID for active_user (empty if not extracted)

    // UI state
    int         selected_msg;  // index in current folder, -1 if none
    bool        inbox_tab;     // true = inbox, false = sent
    bool        compose_open;
    bool        pkg_open;
    bool        register_open;

    // Notification queue
    std::vector<Toast> toasts;
};
```

---

## Key Operations Summary

| Operation | Where | What happens |
|-----------|-------|--------------|
| Setup     | PKG panel, first run | Generates `params.pub`, `master.sec` |
| Register  | Register modal | Writes pending entry to `registry.dat` |
| Approve   | PKG panel | Changes status to `approved` in registry |
| Get Key   | Left panel | Auth check → Extract → writes `keys/<id>.prv` |
| Compose   | Compose modal | H1(To) → Encrypt → writes `.ibemail` to inbox + sent |
| Read      | Right panel | Loads `.ibemail` → Decrypt with `active_key` → render |
| Read (wrong key) | Right panel | Decrypt fails rejection check → shows error |
| Revoke    | PKG panel | Changes status to `revoked`, deletes `.prv` if present |

---

## What we implement vs what PBC provides

| We write | PBC provides |
|----------|-------------|
| H1 (SHA-256 pre-hash → element_from_hash) | element_from_hash |
| H2 (element_to_bytes → SHA-256) | element_to_bytes |
| H3, H4 (SHA-256 with domain separation) | — |
| XOR for V and W | — |
| FullIdent rejection check | — |
| .ibemail format, parse/write | — |
| registry.dat, auth logic | — |
| mailbox folder management | — |
| entire GUI | — |
| base64 encode/decode | — |
| UUID generation for Message-ID | — |
| serialization of all persistent state | element_to/from_bytes |
| Makefile, demo scenario | — |
