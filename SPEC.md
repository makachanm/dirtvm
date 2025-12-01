# DirtVM Instruction Set Spec

DirtVM은 2바이트 바이트코드를 사용합니다.
```
Bytecode Spec

One-Type
[ 6-bit Opcode ] [         10-bit Oprand1          ]
/ ------------------- 16bit wide ------------------/

```

DirtVM은 2개의 메인 스택을 가지고 있는 언어입니다. <br>
호출 스택은 함수의 호출 스택에 해당합니다. 이 스택은 자동으로 관리됩니다.
인자 스택은 데이터를 쌓을 수 있는 스택에 해당합니다. 이 스택은 수동으로 접근할 수 있습니다. <br>

메모리는 지역-주소 모델을 사용합니다.

전역 메모리는 128비트 주소를 가질 수 있습니다.
지역 메모리는 10비트의 필드 지역 태그를 가지고, 128비트의 주소를 가질 수 있습니다.

### Basic Instruction Sets
```
add [000001] - One Type Instruction
No Arguments.

인자 스택에서 두 값을 가져와 더한 뒤 다시 집어넣습니다.
```
```
sub [000010] - One Type Instruction
No Arguments.

인자 스택에서 두 값을 가져와 뺀 뒤 다시 집어넣습니다. (두 번째 값 - 첫 번째 값)
```
```
mul [000011] - One Type Instruction
No Arguments.

인자 스택에서 두 값을 가져와 곱한 뒤 다시 집어넣습니다.
```
```
div [000100] - One Type Instruction
No Arguments.

인자 스택에서 두 값을 가져와 나눈 뒤 다시 집어넣습니다. (두 번째 값 / 첫 번째 값)
```
```
pop [000110] - One Type Instruction
No Arguments.

인자 스택에서 값을 하나 가져옵니다.
```
```
dup [000111] - One Type Instruction
No Arguments.

인자 스택의 최상위 값을 복제하여 다시 집어넣습니다.
```
```
jmp [001000] - One Type Instruction
The 10-bit operand is ignored. A 128-bit (16-byte) address must follow this instruction.

명령어 뒤에 따라오는 128비트 주소로 실행 흐름을 점프합니다.
```
```
jz [001001] - One Type Instruction
The 10-bit operand is ignored. A 128-bit (16-byte) address must follow this instruction.

인자 스택에서 값을 하나 가져와 0인 경우, 명령어 뒤에 따라오는 128비트 주소로 점프합니다.
```
```
jnz [001010] - One Type Instruction
The 10-bit operand is ignored. A 128-bit (16-byte) address must follow this instruction.

인자 스택에서 값을 하나 가져와 0이 아닌 경우, 명령어 뒤에 따라오는 128비트 주소로 점프합니다.
```
```
call [001011] - One Type Instruction
The 10-bit operand is ignored. A 128-bit (16-byte) address must follow this instruction.

명령어 뒤에 따라오는 128비트 주소에 있는 함수를 호출합니다. 현재 IP + 9 (1 for instruction word + 8 for address)을 호출 스택에 저장합니다.
```
```
ret [001100] - One Type Instruction
No Arguments.

호출 스택에서 주소를 가져와 해당 주소로 복귀합니다.
```
```
eq [001101] - One Type Instruction
No Arguments.

인자 스택에서 두 값을 가져와 같은지 비교합니다. 같으면 1, 다르면 0을 스택에 집어넣습니다.
```
```
lt [001110] - One Type Instruction
No Arguments.

인자 스택에서 두 값을 가져와 비교합니다. 두 번째 값이 첫 번째 값보다 작으면 1, 아니면 0을 스택에 집어넣습니다.
```
```
gt [001111] - One Type Instruction
No Arguments.

인자 스택에서 두 값을 가져와 비교합니다. 두 번째 값이 첫 번째 값보다 크면 1, 아니면 0을 스택에 집어넣습니다.
```

### Memory Instructions
```
gload [010000] - One Type Instruction
No Argument.
인자 스택에 있는 128비트 주소값을 가져와 스택에 불러옵니다.
```
```
gstore [010001] - One Type Instruction
No Argument.
첫번째 인자 스택에 있는 128비트 주소값을 가져와 두번째 스택 인자를 저장합니다.
```
```
lload [010010] - One Type Instruction
Argument 1: 10-bit local field tag

첫번째 스택 값인 128비트 주소값을 가져와 Oprand1 인자로 지정된 태그가 지정하는 주소에 있는 128비트 메모리 값을 인자 스택에 불러옵니다.
```
```
lstore [010011] - One Type Instruction
Argument 1: 10-bit local field tag

첫번째 스택 값인 128비트 메모리 주소값을 가져와 Oprand1 인자로 지정된 태그가 지정하는 주소에 두번째 스택 인자를 저장합니다.
```

### Data Instructions
These instructions push data that follows them in the bytecode stream onto the argument stack. The 10-bit operand of these One-Type instructions is ignored.

```
pushd8 [010100] - One Type Instruction
No arguments in the instruction word.

명령어 뒤에 따라오는 8비트(1바이트) 데이터를 인자 스택에 집어넣습니다. 데이터는 16비트 워드로 패딩되어 저장됩니다. IP는 2워드(명령어 1워드 + 데이터 1워드)만큼 증가합니다.
```
```
pushd16 [010101] - One Type Instruction
No arguments in the instruction word.

명령어 뒤에 따라오는 16비트(2바이트) 데이터를 인자 스택에 집어넣습니다. IP는 1워드만큼 증가합니다.
```
```
pushd32 [010110] - One Type Instruction
No arguments in the instruction word.

명령어 뒤에 따라오는 32비트(4바이트) 데이터를 인자 스택에 집어넣습니다. IP는 2워드만큼 증가합니다.
```
```
pushd64 [010111] - One Type Instruction
No arguments in the instruction word.

명령어 뒤에 따라오는 64비트(8바이트) 데이터를 인자 스택에 집어넣습니다. IP는 4워드만큼 증가합니다.
```
```
pushd128 [011000] - One Type Instruction
No arguments in the instruction word.

명령어 뒤에 따라오는 128비트(16바이트) 데이터를 인자 스택에 집어넣습니다. IP는 8워드만큼 증가합니다.
```

### System Call Instructions
```
syscall [011001] - One Type Instruction
Argument 1: 10-bit system call number

인자 스택에서 시스템 콜 인자들을 가져와 Oprand1 인자로 지정된 시스템 콜을 호출합니다.
시스템 콜의 반환 값은 인자 스택에 다시 집어넣습니다.
```
