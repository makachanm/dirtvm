
; =================================================================
; DirtVM 종합 데모 (Comprehensive Demo for DirtVM)
; =================================================================
; 이 프로그램은 DirtVM의 거의 모든 명령어를 시연합니다.
; 각 섹션은 특정 명령어 그룹의 사용법을 보여줍니다.

; --- 데이터 섹션 ---
; .string 지시어를 사용하여 전역 메모리에 문자열을 미리 로드합니다.
.string 0 "Welcome to the DirtVM Demo!\n"
.string 30 "Result of 10*5 is: 50\n"
.string 55 "Value from global memory: 1337\n"
.string 90 "Value from local memory (tag 5): 42\n"
.string 130 "This message is from a function.\n"
.string 165 "Comparison: 10 > 5 is true.\n"
.string 195 "Final message before exit.\n"

; --- 프로그램 시작 ---

; 1. Syscall 데모: 시작 메시지 출력
; ---------------------------------------------
pushd8 28       ; 문자열 길이 (Welcome to the DirtVM Demo!\n)
pushd128 0      ; 문자열 시작 주소
pushd8 1        ; 파일 디스크립터 (1 = stdout)
syscall 1       ; SYS_write 호출
pop             ; syscall 반환 값 제거

; 2. 산술 연산 및 스택 조작 데모
; ---------------------------------------------
; 10 * 5 를 계산합니다.
pushd16 10
pushd16 5
mul             ; 10 * 5 = 50

; dup: 스택의 최상단 값을 복제 (50)
dup

; gstore: 전역 메모리 주소 51에 결과(50) 저장
pushd128 51     ; 주소
gstore

; pop: 복제된 값(50)을 스택에서 제거
pop

; 결과 메시지 출력
pushd8 24       ; 문자열 길이
pushd128 30     ; 문자열 주소
pushd8 1        ; fd
syscall 1
pop

; 3. 전역 메모리 데모 (gstore, gload)
; ---------------------------------------------
pushd32 1337    ; 저장할 값
pushd128 80     ; 저장할 주소
gstore          ; global_memory[80] = 1337

pushd128 80     ; 로드할 주소
gload           ; 스택에 global_memory[80]의 값을 푸시
pop             ; 지금은 값을 사용하지 않으므로 pop

; 메시지 출력
pushd8 33       ; 문자열 길이
pushd128 55     ; 문자열 주소
pushd8 1        ; fd
syscall 1
pop

; 4. 지역 메모리 데모 (lstore, lload)
; ---------------------------------------------
pushd64 42      ; 저장할 값
pushd128 10     ; 저장할 주소
lstore 5        ; 태그 5를 사용하는 지역 메모리에 저장

pushd128 10     ; 로드할 주소
lload 5         ; 태그 5 지역 메모리에서 로드
pop             ; 지금은 값을 사용하지 않으므로 pop

; 메시지 출력
pushd8 39       ; 문자열 길이
pushd128 90     ; 문자열 주소
pushd8 1        ; fd
syscall 1
pop

; 5. 제어 흐름 데모 (call, ret, jmp)
; ---------------------------------------------
call 200        ; 200번 주소에 있는 함수 호출
jmp 250         ; 250번 주소로 점프 (함수 실행 후)

; --- 함수 정의 (주소 200) ---
; 이 코드는 call 명령어로만 실행됩니다.
200:
pushd8 33       ; 문자열 길이
pushd128 130    ; 문자열 주소
pushd8 1        ; fd
syscall 1
pop
ret             ; 호출한 곳으로 복귀

; 6. 비교 및 조건부 점프 데모 (eq, gt, lt, jz, jnz)
; ---------------------------------------------
250:
pushd16 10
pushd16 5
gt              ; 10 > 5, 결과는 1 (true)
dup             ; 스택의 값을 복제합니다.

; jz (Jump if Zero) 데모: 결과가 0이 아니므로 점프하지 않음
jz 300          ; 이 점프는 실행되지 않음

; jnz (Jump if Not Zero) 데모: 결과가 0이 아니므로 점프함
jnz 350         ; 이 점프는 실행됨

300: ; 이 코드는 실행되지 않음
pushd8 1
pushd128 0
pushd8 1
syscall 1
pop

350:
; 비교 결과 메시지 출력
pushd8 29       ; 문자열 길이
pushd128 165    ; 문자열 주소
pushd8 1        ; fd
syscall 1
pop

; 7. 프로그램 종료
; ---------------------------------------------
; 마지막 메시지 출력
pushd8 27       ; 문자열 길이
pushd128 195    ; 문자열 주소
pushd8 1        ; fd
syscall 1
pop

; SYS_exit 호출
pushd8 0        ; 종료 코드 0
syscall 60      ; SYS_exit 호출

; --- 프로그램 끝 ---
