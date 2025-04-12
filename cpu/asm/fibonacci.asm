.DATA
max DW 10
i DW 2

.CODE
MOV CX,0
MOV BX,1

loop: ADD AX,BX
ADD AX,CX
MOV CX,BX
MOV BX,AX
CMP [i],[max]
JZ stop
ADD [i],1
MOV AX,0
JMP loop

stop: HALT