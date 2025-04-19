; Calcule la valeur de la sequence de fibonacci
; pour n = "max" (10 dans cette exemple) et stocke le résultat dans "resultat"

.DATA
max DW 10
resultat DW 0

.CODE
; Initialisation des variables
MOV CX,0
MOV BX,1
MOV DX,2

; On repete en boucle
loop: ADD AX,BX
ADD AX,CX
MOV CX,BX
MOV BX,AX

; Si DX == max alors on rentre dans le label "stop"
CMP DX,[max]
JZ stop

ADD DX,1
MOV AX,0
JMP loop

; On stocke le resultat dans "resultat" et on termine le programme avec "HALT"
stop: MOV [resultat] AX
HALT