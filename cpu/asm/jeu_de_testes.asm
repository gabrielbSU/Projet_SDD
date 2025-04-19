; programme pour tester les differentes instructions

.DATA
; declaration de varaibles
x DW 42
arr DB 20,21,22,23
y DB 10

.CODE
; copie de valeurs
MOV AX,5
MOV BX,3

; additions
; addition 2 e AX
ADD AX,2
; addition de la valeur de BX avec AX
ADD AX,BX
; addition de la valeur de x avec AX 
ADD AX,[x]
; addition de la valeur de x avec AX (ajout dans x)
ADD [x],AX

; comparaisons
MOV AX,5
MOV BX,5
; on a AX == BX qui est vrai
CMP AX,BX
; on saute dans le label "succes"
JZ succes
; sinon on termine le programme
HALT

succes:
; testes de l'extra segment
; testons la methode d'allocation dynamique 1 
MOV BX,1
; allocation de taille 8
MOV AX,8
ALLOC
; stockage de la valeur 10 dans l'extra segment
MOV AX,5
MOV [ES:AX],10
; recuperation de la valeur 10 dans CX
MOV CX [ES:AX]
; liberation d' l'extra segment
FREE

; testons la pile
MOV AX,42
PUSH AX
PUSH AX
PUSH AX
; BX sera egal e 42
POP BX
; fin du programme
HALT