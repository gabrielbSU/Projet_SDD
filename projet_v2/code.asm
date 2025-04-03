.DATA
x DW 42
arr DB 20,21,22,23
y DB 10

.CODE
start: MOV AX,[x]
loop: ADD AX,y
JMP loop