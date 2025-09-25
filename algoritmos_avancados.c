#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Definição do tamanho da tabela hash
#define HASH_SIZE 10

// Estrutura para os cômodos da mansão (Árvore Binária)
typedef struct Sala {
    char nome[50];
    char pista[256];
    struct Sala* esquerda;
    struct Sala* direita;
} Sala;

// Estrutura para a Pilha de Histórico de Salas
typedef struct StackNode {
    Sala* sala;
    struct StackNode* proximo;
} StackNode;

// --- NOVAS ESTRUTURAS PARA A TABELA HASH ---

// Estrutura para o nó da lista de pistas de um suspeito
typedef struct PistaSuspeitoNode {
    char pista[256];
    struct PistaSuspeitoNode* proximo;
} PistaSuspeitoNode;

// Estrutura para o nó da tabela hash (o "Suspeito")
typedef struct SuspeitoNode {
    char nome[50];
    PistaSuspeitoNode* pistas; // Lista encadeada de pistas associadas
    int contagem; // Contador de pistas coletadas
    struct SuspeitoNode* proximo; // Para tratar colisões
} SuspeitoNode;

SuspeitoNode* hashTable[HASH_SIZE];

// --- FIM DAS NOVAS ESTRUTURAS ---

//---
// Protótipos das funções
//---
void inicializarHash();
void inserirNaHash(const char* suspeito, const char* pista);
void listarAssociacoes();
void exibirPistasColetadas(SuspeitoNode* raiz);
const char* encontrarSuspeitoMaisCitado();
void verificarSuspeitoFinal(SuspeitoNode* raiz);

//---
// criarSala()
// Cria, de forma dinâmica, um novo cômodo com nome e uma pista associada.
//---
Sala* criarSala(const char* nome, const char* pista) {
    Sala* novaSala = (Sala*)malloc(sizeof(Sala));
    if (novaSala == NULL) {
        printf("Erro de alocacao de memoria para Sala!\n");
        exit(1);
    }
    strcpy(novaSala->nome, nome);
    strcpy(novaSala->pista, pista);
    novaSala->esquerda = NULL;
    novaSala->direita = NULL;
    return novaSala;
}

//---
// push() e pop() - Funções da Pilha
//---
void push(StackNode** top, Sala* sala) {
    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
    if (newNode == NULL) {
        printf("Erro de alocacao de memoria para StackNode!\n");
        exit(1);
    }
    newNode->sala = sala;
    newNode->proximo = *top;
    *top = newNode;
}

Sala* pop(StackNode** top) {
    if (*top == NULL) {
        return NULL;
    }
    StackNode* temp = *top;
    Sala* sala = temp->sala;
    *top = temp->proximo;
    free(temp);
    return sala;
}

//---
// hashFunction()
// Função de hash simples para calcular o índice de uma chave (nome do suspeito) na tabela.
//---
int hashFunction(const char* key) {
    int hash = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        hash += key[i];
    }
    return hash % HASH_SIZE;
}

//---
// inicializarHash() - NOVA FUNÇÃO
// Inicializa a tabela hash, definindo todos os ponteiros como NULL.
//---
void inicializarHash() {
    for (int i = 0; i < HASH_SIZE; i++) {
        hashTable[i] = NULL;
    }
}

//---
// inserirNaHash() - FUNÇÃO REESCRITA
// Insere uma nova associação suspeito/pista na tabela hash.
// Lida com colisões e atualiza a lista de pistas para um suspeito existente.
//---
void inserirNaHash(const char* suspeito, const char* pista) {
    int index = hashFunction(suspeito);
    
    // Procura por um nó de suspeito existente
    SuspeitoNode* current = hashTable[index];
    while (current != NULL) {
        if (strcmp(current->nome, suspeito) == 0) {
            break;
        }
        current = current->proximo;
    }
    
    // Se o suspeito não existe, cria um novo nó para ele
    if (current == NULL) {
        SuspeitoNode* novoSuspeito = (SuspeitoNode*)malloc(sizeof(SuspeitoNode));
        if (novoSuspeito == NULL) {
            printf("Erro de alocacao de memoria para SuspeitoNode!\n");
            exit(1);
        }
        strcpy(novoSuspeito->nome, suspeito);
        novoSuspeito->contagem = 0;
        novoSuspeito->pistas = NULL;
        
        // Insere o novo suspeito no início da lista encadeada do hash
        novoSuspeito->proximo = hashTable[index];
        hashTable[index] = novoSuspeito;
        current = novoSuspeito;
    }
    
    // Adiciona a pista à lista de pistas do suspeito
    PistaSuspeitoNode* novaPistaNode = (PistaSuspeitoNode*)malloc(sizeof(PistaSuspeitoNode));
    if (novaPistaNode == NULL) {
        printf("Erro de alocacao de memoria para PistaSuspeitoNode!\n");
        exit(1);
    }
    strcpy(novaPistaNode->pista, pista);
    novaPistaNode->proximo = current->pistas;
    current->pistas = novaPistaNode;
}

//---
// listarAssociacoes() - NOVA FUNÇÃO
// Percorre a tabela hash e imprime todas as associações de suspeitos e suas pistas.
//---
void listarAssociacoes() {
    printf("\n--- ASSOCIACOES DE SUSPEITOS E PISTAS ---\n");
    for (int i = 0; i < HASH_SIZE; i++) {
        SuspeitoNode* currentSuspeito = hashTable[i];
        while (currentSuspeito != NULL) {
            printf("- Suspeito: %s\n", currentSuspeito->nome);
            PistaSuspeitoNode* currentPista = currentSuspeito->pistas;
            while (currentPista != NULL) {
                printf("  -> Pista: \"%s\"\n", currentPista->pista);
                currentPista = currentPista->proximo;
            }
            currentSuspeito = currentSuspeito->proximo;
        }
    }
}

//---
// explorarSalas() - FUNÇÃO MODIFICADA
// Agora, em vez de uma BST, ela atualiza a contagem de pistas na tabela hash.
//---
void explorarSalas(Sala* salaAtual, SuspeitoNode** raizPistas) {
    char escolha;
    StackNode* historico = NULL;
    
    printf("\nVoce esta em: %s\n", salaAtual->nome);
    
    while (1) {
        if (strlen(salaAtual->pista) > 0) {
            printf("Pista encontrada: \"%s\"\n", salaAtual->pista);
            // Procura o suspeito para esta pista na hash e incrementa o contador
            const char* suspeito = encontrarSuspeito(salaAtual->pista);
            if (suspeito != NULL) {
                 int index = hashFunction(suspeito);
                 SuspeitoNode* current = hashTable[index];
                 while (current != NULL) {
                    if (strcmp(current->nome, suspeito) == 0) {
                        current->contagem++;
                        break;
                    }
                    current = current->proximo;
                 }
            }
        }
        
        printf("Para onde voce quer ir? (e - esquerda, d - direita, v - voltar, l - listar pistas, s - sair): ");
        scanf(" %c", &escolha);
        
        Sala* proximaSala = NULL;
        switch (escolha) {
            case 'e':
            case 'E':
                if (salaAtual->esquerda != NULL) {
                    push(&historico, salaAtual);
                    proximaSala = salaAtual->esquerda;
                } else {
                    printf("Nao ha caminho para a esquerda.\n");
                }
                break;
            case 'd':
            case 'D':
                if (salaAtual->direita != NULL) {
                    push(&historico, salaAtual);
                    proximaSala = salaAtual->direita;
                } else {
                    printf("Nao ha caminho para a direita.\n");
                }
                break;
            case 'v':
            case 'V':
                proximaSala = pop(&historico);
                if (proximaSala == NULL) {
                    printf("Voce ja esta no Hall de Entrada. Nao e possivel voltar mais.\n");
                }
                break;
            case 'l':
            case 'L':
                // Exibe as pistas coletadas por suspeito
                exibirPistasColetadas(NULL); // A lógica é agora na hash
                break;
            case 's':
            case 'S':
                printf("Fim da exploracao.\n");
                return;
            default:
                printf("Opcao invalida. Por favor, escolha 'e', 'd', 'v', 'l' ou 's'.\n");
                break;
        }

        if (proximaSala != NULL) {
            salaAtual = proximaSala;
            printf("\nVoce esta em: %s\n", salaAtual->nome);
        }
    }
}

//---
// exibirPistasColetadas() - NOVA FUNÇÃO
// Percorre a tabela hash e imprime as pistas de suspeitos que tiveram pistas coletadas.
//---
void exibirPistasColetadas(SuspeitoNode* raiz) {
    printf("\n--- SUAS PISTAS COLETADAS ---\n");
    for (int i = 0; i < HASH_SIZE; i++) {
        SuspeitoNode* currentSuspeito = hashTable[i];
        while (currentSuspeito != NULL) {
            if (currentSuspeito->contagem > 0) {
                printf("- Pistas de %s (%d): \n", currentSuspeito->nome, currentSuspeito->contagem);
                PistaSuspeitoNode* currentPista = currentSuspeito->pistas;
                while (currentPista != NULL) {
                    printf("  - \"%s\"\n", currentPista->pista);
                    currentPista = currentPista->proximo;
                }
            }
            currentSuspeito = currentSuspeito->proximo;
        }
    }
}


//---
// encontrarSuspeitoMaisCitado() - NOVA FUNÇÃO
// Percorre a tabela hash e retorna o nome do suspeito com a maior contagem.
//---
const char* encontrarSuspeitoMaisCitado() {
    int maxContagem = -1;
    SuspeitoNode* suspeitoMaisCitado = NULL;

    for (int i = 0; i < HASH_SIZE; i++) {
        SuspeitoNode* currentSuspeito = hashTable[i];
        while (currentSuspeito != NULL) {
            if (currentSuspeito->contagem > maxContagem) {
                maxContagem = currentSuspeito->contagem;
                suspeitoMaisCitado = currentSuspeito;
            }
            currentSuspeito = currentSuspeito->proximo;
        }
    }

    if (suspeitoMaisCitado != NULL) {
        return suspeitoMaisCitado->nome;
    }
    
    return "Nenhum";
}


//---
// verificarSuspeitoFinal() - FUNÇÃO REESCRITA
// Conduz a fase de julgamento final usando a nova estrutura.
//---
void verificarSuspeitoFinal(SuspeitoNode* pistasColetadas) {
    char suspeitoAcusado[50];
    int pistasContadas = 0;
    
    printf("\n--- FIM DA EXPLORACAO ---\n");
    printf("\nCom as pistas coletadas, e hora de fazer a sua acusacao.\n");
    
    printf("\nSuas pistas coletadas sao:\n");
    exibirPistasColetadas(NULL); // Passando NULL pois a função não usa mais a BST
    
    printf("\n--- ANALISE DE EVIDENCIAS ---\n");
    const char* suspeitoMaisCitado = encontrarSuspeitoMaisCitado();
    printf("O suspeito mais citado nas pistas e: %s\n", suspeitoMaisCitado);
    
    printf("\nQuem voce acusa? (Digite o nome do suspeito ou parte dele): ");
    getchar();
    fgets(suspeitoAcusado, sizeof(suspeitoAcusado), stdin);
    suspeitoAcusado[strcspn(suspeitoAcusado, "\n")] = '\0';
    
    printf("\nA sua acusacao e: %s\n", suspeitoAcusado);
    
    // Converte a entrada do usuário para minúsculas
    char acusacaoMinuscula[50];
    strcpy(acusacaoMinuscula, suspeitoAcusado);
    for (int i = 0; acusacaoMinuscula[i]; i++) {
        acusacaoMinuscula[i] = tolower(acusacaoMinuscula[i]);
    }

    // Procura o suspeito acusado na tabela hash e pega a contagem de pistas
    for (int i = 0; i < HASH_SIZE; i++) {
        SuspeitoNode* current = hashTable[i];
        while (current != NULL) {
            char nomeMinusculo[50];
            strcpy(nomeMinusculo, current->nome);
            for (int j = 0; nomeMinusculo[j]; j++) {
                nomeMinusculo[j] = tolower(nomeMinusculo[j]);
            }
            
            if (strstr(nomeMinusculo, acusacaoMinuscula) != NULL) {
                pistasContadas = current->contagem;
                break;
            }
            current = current->proximo;
        }
    }
    
    printf("\n--- VEREDICTO ---\n");
    if (pistasContadas >= 2) {
        printf("Prova substancial! Ha %d pistas que apontam para o(a) suspeito(a).\n", pistasContadas);
        printf("Sua acusacao se sustenta. O caso esta resolvido!\n");
    } else {
        printf("Prova insuficiente. Apenas %d pista(s) aponta(m) para o(a) suspeito(a).\n", pistasContadas);
        printf("Sem provas concretas, o suspeito nao pode ser condenado. O caso permanece um misterio.\n");
    }
}

//---
// main() com o novo mapa da mansão
//---
int main() {
    inicializarHash(); // Inicializa a tabela hash com a nova função
    
    // Nível 1: Hall
    Sala* hallEntrada = criarSala("Hall de Entrada", "O mapa do labirinto foi encontrado em um vaso de flores. O Professor Plum estava na frente, sorrindo.");
    // Nível 2: Áreas sociais
    Sala* salaEstar = criarSala("Sala de Estar", "Um cofre aberto sob o carpete revelou um recibo de lavanderia datado de ontem, em nome da Senhorita Scarlet.");
    Sala* salaJantar = criarSala("Sala de Jantar", "Uma garrafa de vinho com as iniciais do Coronel Mostarda estava sobre a mesa.");
    // Nível 3: Cozinha e Escritório
    Sala* cozinha = criarSala("Cozinha", "Um pote de sal está virado, com um pequeno fragmento de papiro no meio, que indica um local misterioso.");
    Sala* escritorio = criarSala("Escritorio", "Um livro sobre a história da família do Sr. Green estava fora do lugar, com a pagina 37 marcada com uma anotação.");
    // Nível 4: Áreas de lazer internas
    Sala* salaoFestas = criarSala("Salão de Festas", "Um bilhete de aviao para um voo a tarde foi achado embaixo da mesa de DJ.");
    Sala* adega = criarSala("Adega", "A etiqueta de um vinho raro de 1978 foi rasgada, sugerindo que foi bebida por alguém que não se importava com o valor histórico.");
    // Nível 5: Dormitórios
    Sala* suitePrincipal = criarSala("Suite Principal", "Uma agenda revela uma reunião secreta do Sr. Green e do Professor Plum para discutir a compra de um terreno.");
    Sala* quartoHospedes = criarSala("Quarto de Hospedes", "Na mesinha de cabeceira, a Miss Scarlet deixou uma foto com uma inscrição romântica.");
    // Nível 6: Áreas de serviço e externas
    Sala* lavanderia = criarSala("Lavanderia", "A conta de luz é surpreendentemente alta.");
    Sala* academia = criarSala("Academia", "Aparelhos de ginastica mostram um uso recente, indicando uma pessoa atlética.");
    Sala* piscina = criarSala("Piscina", "Uma chave dourada foi encontrada no fundo da piscina. O Coronel Mostarda foi a ultima pessoa a usa-la.");
    
    // Conectando os cômodos para formar a árvore
    hallEntrada->esquerda = salaEstar;
    hallEntrada->direita = salaJantar;
    salaEstar->esquerda = cozinha;
    salaEstar->direita = escritorio;
    salaJantar->esquerda = salaoFestas;
    salaJantar->direita = adega;
    cozinha->esquerda = suitePrincipal;
    cozinha->direita = quartoHospedes;
    escritorio->direita = lavanderia;
    salaoFestas->esquerda = academia;
    academia->direita = piscina;

    // Popula a Tabela Hash com as associações Suspeito -> Pista
    inserirNaHash("Professor Plum", "O mapa do labirinto foi encontrado em um vaso de flores. O Professor Plum estava na frente, sorrindo.");
    inserirNaHash("Senhorita Scarlet", "Um cofre aberto sob o carpete revelou um recibo de lavanderia datado de ontem, em nome da Senhorita Scarlet.");
    inserirNaHash("Coronel Mostarda", "Uma garrafa de vinho com as iniciais do Coronel Mostarda estava sobre a mesa.");
    inserirNaHash("Senhorita Scarlet", "Um bilhete de aviao para um voo a tarde foi achado embaixo da mesa de DJ.");
    inserirNaHash("Sr. Green", "Um livro sobre a história da família do Sr. Green estava fora do lugar, com a pagina 37 marcada com uma anotação.");
    inserirNaHash("Coronel Mostarda", "A etiqueta de um vinho raro de 1978 foi rasgada, sugerindo que foi bebida por alguém que não se importava com o valor histórico.");
    inserirNaHash("Sr. Green", "Uma agenda revela uma reunião secreta do Sr. Green e do Professor Plum para discutir a compra de um terreno.");
    inserirNaHash("Senhorita Scarlet", "Na mesinha de cabeceira, a Miss Scarlet deixou uma foto com uma inscrição romântica.");
    inserirNaHash("Coronel Mostarda", "Uma chave dourada foi encontrada no fundo da piscina. O Coronel Mostarda foi a ultima pessoa a usa-la.");
    inserirNaHash("Sr. Green", "Um pote de sal está virado, com um pequeno fragmento de papiro no meio, que indica um local misterioso.");

    // EXIBIÇÃO DE ASSOCIAÇÕES PARA CONFERIR O REQUISITO
    listarAssociacoes();

    // Começa a exploração da mansão
    explorarSalas(hallEntrada, NULL);
    
    verificarSuspeitoFinal(NULL);
    
    return 0;
}