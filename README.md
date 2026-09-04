# Z33 Emulator

Émulateur et assembleur pour l'architecture Zorglub 33 (Z33). Les programmes
sont chargés puis exécutés à partir de l'adresse `1000`.

## Compiler et lancer

```sh
make
./z33 tests/factoriel
```

Terminez chaque programme par `reset`. Une cellule vide ou contenant une donnée
ne peut pas être exécutée : elle déclenche l'exception `EX_INVALID_INSTRUCTION`.

```asm
main:
    ld 42, %a
    reset
```

## Sortie du programme et débogage

Les caractères produits par un programme Z33 (`out ..., [111]`) utilisent la
**sortie standard** (`stdout`, descripteur `1`). Les traces de chargement et
d'exécution, erreurs et état final de l'émulateur utilisent la **sortie d'erreur**
(`stderr`, descripteur `2`).

Afficher la sortie du programme dans le terminal et enregistrer le débogage :

```sh
./z33 tests/helloworld 2> debug.log
```

Enregistrer la sortie du programme et le débogage dans deux fichiers :

```sh
./z33 tests/helloworld > program.log 2> debug.log
```

## Registres et exécution

| Registre | Rôle | Valeur initiale |
| --- | --- | --- |
| `%a`, `%b` | Registres généraux | `0` |
| `%pc` | Compteur ordinal | `1000` |
| `%sp` | Pointeur de pile | `10000` |
| `%sr` | Registre d'état | superviseur (`S=1`) |

La mémoire contient 10 000 cellules, de `0` à `9999`. Après le fetch,
l'émulateur incrémente `%pc` avant l'exécution : `call` sauvegarde donc la bonne
adresse de retour. Le compteur `cycles` ajoute un cycle de base par instruction
et un cycle par opérande mémoire (direct, indirect ou indexé).

Les indicateurs de `%sr` sont `C`, `Z`, `N`, `O`, `IE` et `S`. `in`, `out`,
`rti` et toute écriture dans `%sr` exigent le mode superviseur.

## Assembleur

```asm
start: alias: ld 1, %a   // labels multiples et commentaire
```

- Les mnémotechniques sont insensibles à la casse.
- Les labels sont sensibles à la casse ; ils commencent par une lettre ou `_`.
- Les modes mémoire sont `[adresse]`, `[%registre]` et `[%registre+decalage]`.
- Les expressions sont acceptées pour les valeurs numériques et les adresses.

Instructions :

```text
ld st | add sub mul div neg | and or xor not shl shr | cmp
jmp jeq jne jlt jle jgt jge | swap push pop | call rtn trap rti reset nop
fas | in out
```

`jeq`/`jne` testent `Z`, `jlt`/`jge` testent `N`, et `jle`/`jgt` combinent `N`
et `Z`.

### Directives et préprocesseur

| Directive | Effet |
| --- | --- |
| `.addr expression` | Positionne l'adresse d'assemblage. |
| `.space expression` | Réserve des cellules vides. |
| `.word expression` | Écrit un mot. |
| `.string "texte"` | Écrit les caractères et le terminateur nul. |

Les chaînes reconnaissent `\n`, `\t`, `\"` et `\\`. Les expressions supportent
`+ - * / %`, `<< >>`, `& | ^ ~`, les comparaisons, `!`, `&&` et `||`, avec des
littéraux décimaux, hexadécimaux, binaires ou octaux.

Le préprocesseur fournit `#define`, `#undefine`, `#include`, `#if`, `#elif`,
`#else`, `#endif` et `#error`. Les blocs inactifs sont ignorés.

## Exceptions et interruptions

Une exception sauvegarde `%pc`, `%sr` et son code aux adresses `100`, `101` et
`102`, puis saute à l'adresse `200` en mode superviseur. Le gestionnaire utilise
`rti` pour restaurer l'état. Les codes notables sont : `0` interruption
matérielle, `1` division par zéro, `2` instruction invalide, `3` instruction
privilégiée, `4` `trap` et `5` adresse mémoire invalide.

## Entrées/sorties

Les ports non mappés renvoient `0` en lecture et ignorent les écritures.

| Port | Lecture | Écriture |
| --- | --- | --- |
| `110` | Statut série : `R` (bit 0), `T` toujours à 1 (bit 1), `I` (bit 2). La lecture efface `I`. | Bit 0 : active les interruptions de réception. |
| `111` | Dépile un octet reçu, ou `0` si la file est vide. | Émet les 8 bits de poids faible. |

L'entrée de l'hôte est placée dans une file non bloquante :

```sh
printf 'Z' | ./z33 tests/serial
```

Quand les interruptions série et `IE` sont activées, l'interruption matérielle
est livrée entre deux instructions. Le gestionnaire doit lire le statut puis
vider la file avant `rti`.

Les ports clavier (20–21) et disque (50–100) ne sont pas implémentés : ils sont
donc non mappés.

## Exemples

- `tests/factoriel` : récursion, pile et sous-programmes.
- `tests/helloworld` : écriture sur le port série.
- `tests/assembly-language` : labels, directives, chaînes et expressions.
- `tests/serial` et `tests/serial-interrupt` : I/O série.

> Dans VS Code, vérifiez que `%` n'a pas été remplacé par `‰` dans les fichiers
> assembleur : cet encodage invalide les opérandes de registre.
