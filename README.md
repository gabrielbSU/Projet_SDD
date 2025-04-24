# Simulateur de CPU

Ce projet est un simulateur de CPU (Central Processing Unit).
Il s'agit de concevoir une version simplifiée d'un processeur qui
est capable d'exécuter un programme écrit en langage assembleur.

---

## Compilation et lancement
Dans le répertoire `cpu`, le projet utilise un Makefile pour la compilation.

- Commande `make` pour la compilation du projet.

- Commande `make clean` pour supprimer les fichiers issues de la compilation.

- Commande `./main "chemin/fichier/assembleur.asm"` pour exécuter les instructions assembleurs d'un fichier.

## Exemples
- La suite de fibonacci :
```bash
make && ./main asm/fibonacci.asm
```
Calcule la valeur de la sequence de fibonacci
pour n = "max" et stocke le résultat dans la variable "resultat" dans le segment de données.


- Jeu de testes :
```bash
make && ./main asm/jeu_de_testes.asm
```
Teste les différentes instructions assembleurs (voir les commentaires du fichier pour la description des jeux d'essais).
## Contenu

Le programme est capable d’exécuter les instructions assembleur suivantes :

- `MOV dest,src` : transfère la valeur de la source vers la destination.  
- `ADD dest,src` : additionne les valeurs de la source et de la destination, le résultat est stocké dans la destination.  
- `CMP dest,src` : effectue une soustraction logique `dest - src` (sans modifier les opérandes) et met à jour les drapeaux :
  - `ZF` (Zero Flag) est mis à 1 si `dest == src`,
  - `SF` (Sign Flag) est mis à 1 si `dest < src`.
- `JMP address` : modifie le pointeur d’instruction `IP` pour sauter à l'adresse donnée : `IP ← address`.  
- `JZ address` : effectue un saut conditionnel si le drapeau `ZF` vaut 1 (le résultat précédent était nul) : `IP ← address`.  
- `JNZ address` : effectue un saut conditionnel si le drapeau `ZF` vaut 0 (le résultat précédent était non nul) : `IP ← address`.  
- `HALT` : termine l’exécution en positionnant le pointeur d’instruction `IP` à la fin du segment de code.  
- `PUSH registre` : empile la valeur contenue dans un registre sur la pile (AX par défaut).  
- `POP registre` : dépile la valeur du sommet de la pile et la place dans un registre (AX par défaut).  
- `ALLOC` : alloue dynamiquement un segment mémoire `ES` de taille contenue dans le registre `AX`, selon la stratégie spécifiée dans `BX` :
  - `BX = 0` : First Fit  
  - `BX = 1` : Best Fit  
  - `BX = 2` : Worst Fit  
- `FREE` : libère le segment mémoire pointé par `ES`.

L'exécution de chaque instruction revoie un code d'erreur : 0 si l'instruction s'est bien exécutée, une autre valeur sinon.

## Description du code
le code est divisé en plusieurs fichiers :
- `makefile` : fichier de compilation du projet.
- `cpu.c` : contient la structure du CPU et les fonctions de gestion de la mémoire.
- `cpu.h` : contient la déclaration de la structure du CPU et des fonctions de gestion de la mémoire.
- `dynamique.c` : contient les fonctions de gestion de segment de mémoire dynamique.
- `dynamique.h` : contient la déclaration des fonctions de gestion de segment de mémoire dynamique.
- `hachage.c` : contient les fonctions de gestion de la table de hachage.
- `hachage.h` : contient la déclaration des fonctions de gestion de la table de hachage.
- `parser.c` : contient les fonctions de parsing du fichier assembleur et de gestion des instructions.
- `parser.h` : contient la déclaration des fonctions de parsing du fichier assembleur et de gestion des instructions.
- `logger.h` : contient les fonctions de gestion des logs (affichage des erreurs).
- `main.c` : contient la fonction principale qui lit le fichier assembleur et exécute les instructions.
- `asm/` : contient les fichiers assembleurs d'exemple.
- `asm/fibonacci.asm` : contient le code assembleur de la suite de fibonacci.
- `asm/jeu_de_testes.asm` : contient le code assembleur de testes des instructions.

### Table de hachage
Le simulateur utilise une table de hachage avec résolution des collisions par un mécanisme de sondage/probing linéaire pour stocker les valeurs des registres, des constantes, labels etc.
```c
// Structure pour représenter une entrée de la table de hachage
typedef struct hashentry {
    char *key;
    void *value;
} HashEntry;

// Structure pour représenter la table de hachage
typedef struct hashmap {
    int size;
    HashEntry *table;
} HashMap;
```
### Mémoire dynamique
Le simulateur utilise une mémoire dynamique pour stocker les divers segments de mémoire.
```c
// Structure pour représenter un segment de mémoire
typedef struct segment {
    int start; // Position de début (adresse) du segment dans la mémoire
    int size;  // Taille du segment en unités de mémoire
    struct segment *next; // Pointeur vers le segment suivant dans la liste chaînée
} Segment;

// Structure pour gérer la mémoire
typedef struct memoryHandler {
    void **memory; // Tableau de pointeurs vers la mémoire allouée
    int total_size; // Taille totale de la mémoire gérée
    Segment *free_list; // Liste chaînée des segments de mémoire libres
    HashMap *allocated; // Table de hachage (nom, segment)
} MemoryHandler;
```

### Parsage du fichier assembleur
Le fichier assembleur est parsé par la fonction `parse` qui renvoie un `ParserResult` :

```c
ParserResult *res = parse("chemin/fichier/assembleur.asm");
```
Le résultat du parseur est une structure contenant les instructions du segment de données et du segment de code, ainsi que les labels et les variables.
```c
// Structure pour représenter une instruction
typedef struct {
    char *mnemonic; // Instruction mnemonic (ou nom de variable pour .DATA)
    char *operand1; // Premier operande (ou type pour .DATA)
    char *operand2; // Second operande (ou initialisation pour .DATA)
} Instruction;

// Structure pour représenter le résultat du parseur
typedef struct {
    Instruction **data_instructions; // Tableau d’instructions .DATA
    int data_count; // Nombre d’instructions .DATA
    Instruction **code_instructions; // Tableau d’instructions .CODE
    int code_count; // Nombre d’instructions .CODE
    HashMap *labels;    // labels -> indices dans code_instructions
    HashMap *memory_locations; // noms de variables -> adresse memoire
} ParserResult;
```

### Structure du CPU
La structure du CPU est définie dans le fichier `cpu.h` et contient les champs suivants :

```c
typedef struct {
    MemoryHandler* memory_handler; //Gestionnaire de memoire
    HashMap *context; //Registres
    HashMap *constant_pool; //Table de hachage pour stocker les valeurs immediates
} CPU;
```
Le champ `memory_handler` est un pointeur vers une structure de type `MemoryHandler` qui gère la mémoire dynamique.  
Le champ `context` est un pointeur vers une structure de type `HashMap` qui contient les registres du CPU. Les clées sont les noms des registres et les valeurs sont les valeurs contenues dans les registres.  
Le champ `constant_pool` est un pointeur vers une structure de type `HashMap` qui contient les valeurs immédiates du fichier assembleur. Les clées sont les noms des valeurs constantes et les valeurs sont les valeurs des constantes. (Exemple : dans l'instruction `MOV AX,10` le nombre 10 est une valeur constante).
On utilise 
```c
CPU *cpu_init(int memory_size)
```
pour initialiser la structure du CPU. Le paramètre `memory_size` est la taille de la mémoire à allouer pour le simulateur.  
Ensuite, on appelle à la suite les fonctions suivantes
```c
int resolve_constants(ParserResult *result); // Résout les constantes du fichier assembleur
void allocate_variables(CPU *cpu, Instruction **data_instructions,int data_count); // Alloue le segment de données dans la mémoire
void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count); // Alloue le segment de code dans la mémoire
```
pour initialiser la mémoire du CPU qui contiennent les données du parsage du fichier assembleur.  
Pour libérer la mémoire, on utilise la fonction suivante :
```c
// Libere le resultat du parseur
void free_parser_result(ParserResult *result);
// Libere la memoire du CPU
void cpu_destroy(CPU *cpu);
```
### Exécution des instructions
Pour exécuter les instructions, on utilise la fonction `fetch_next_instruction` qui renvoie un pointeur vers la prochaine instruction à exécuter.
```c
Instruction *fetch_next_instruction(CPU *cpu)
```
Ensuite, on utilise la fonction `execute_instruction` qui prend en paramètre le CPU et l'instruction à exécuter.
```c
int execute_instruction(CPU *cpu, Instruction *instr){
    void* src =  resolve_adressing(cpu, instr->operand2); // On resout l'adressage de l'operande 2
    void* dest =  resolve_adressing(cpu, instr->operand1); // On resout l'adressage de l'operande 1
    return handle_instruction(cpu, instr, src, dest); // On exécute l'instruction
}
```
La fonction `handle_instruction` permet de gérer l'exécution de l'instruction. (voir section **Contenu**)  
La fonction `resolve_adressing` permet de résoudre l'adressage des opérandes. Elle renvoie un pointeur vers la valeur de l'opérande.  
Voici les différents types d'adressage gérés :
- `immediate` : valeur immédiate (ex: `MOV AX,10`)
- `register` : registre (ex: `MOV AX,BX`)
- `memory_direct` : adresse mémoire directe (ex: `MOV AX,[10]`)
- `register_indirect` : adresse mémoire indirecte (ex: `MOV AX,[BX]`)
- `segment_override` : adresse mémoire avec segment (ex: `MOV [ES:AX],42`)
```c
void *immediate_addressing(CPU *cpu, const char *operand);
void *register_addressing(CPU *cpu, const char *operand);
void *memory_direct_addressing(CPU *cpu, const char *operand);
void *register_indirect_addressing(CPU *cpu, const char *operand);
void *segment_override_addressing(CPU* cpu, const char* operand);
```

### Instructions de gestion de la pile

Deux fonctions permettent de manipuler la pile via le registre `SP` (Stack Pointer) et le segment `SS` (Stack Segment) :

```c
int push_value(CPU *cpu, int value);
int pop_value(CPU *cpu, int *dest);
```
La taille de la pile est par défaut de 128 (`STACK_SIZE`)  
La fonction `push_value` empile la valeur `value` sur la pile et met à jour le registre `SP`.  
La fonction `pop_value` dépile la valeur du sommet de la pile et la place dans `dest`, puis met à jour le registre `SP`.  
Les fonctions sont exécutées par les instructions assembleurs `PUSH` et `POP` respectivement.

### Instructions de gestion de la mémoire dynamique

Il existe deux instructions assembleurs pour gérer la mémoire dynamique : `ALLOC` et `FREE`. Ces instructions sont directement géré par la fonction `handle_instruction`.  
Ainsi, avant d'exécuter l'instruction `ALLOC`, il faut initialiser le registre `BX` pour choisir la méthode d'allocation dynamique.
Le registre `BX` peut prendre les valeurs suivantes :
- `0` : First Fit (première case libre trouvée)
- `1` : Best Fit (meilleure case libre trouvée)
- `2` : Worst Fit (plus grande case libre trouvée)

Ensuite, il faut initialiser le registre `AX` avec la taille de la mémoire à allouer.
La fonction `ALLOC` renvoie un code d'erreur : 0 si l'allocation s'est bien passée, une autre valeur sinon.  
L'instruction `FREE` libère le segment mémoire pointé par le registre `ES` (Extra Segment).

Exemple :
```asm
; testons la méthode d'allocation dynamique 1 
MOV BX,1
; allocation de taille 8
MOV AX,8
ALLOC
; stockage de la valeur 10 dans l'extra segment
MOV AX,5
MOV [ES:AX],10
; récupération de la valeur 10 dans CX
MOV CX [ES:AX]
; libération d' l'extra segment
FREE
```