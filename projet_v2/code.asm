.DATA
x DW 42
arr DB 20,21,22,23
y DB 10

.CODE
start: MOV BX,1
loop: ADD x,2
JMP loop