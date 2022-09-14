컴파일 환경: Microsoft C++
실행방법:
'>' 이후에 input 입력 (실제 scheme에서 사용하는 것과 같이 입력)
pseudocode에 나온대로 +, -, *, number?, symbol?, null?, eq?, equal?, cons, cond, car, cdr, define, ' 입력 가능

define할 경우, 아무것도 출력하지 않으며 출력이 필요한 input의 경우 ']' 후에 알맞은 값이 출력된다.

입력 중에 hash의 개수 또는 garbage collection을 했음에도 불구하고 node의 개수가 처음에 정의한 값을 초과할 경우 에러 메시지를 출력하며 프로그램 종료

cond를 사용할 때 else를 입력하지 않으면 에러 메시지를 출력하며 프로그램 종료

입력 중에 undefined variable을 사용할 경우, 어떤 input이 undefined variable인지 알려주는 에러 메시지와 함께 프로그램 종료

node, hash, token 개수는 3~5 line에서 define 값을 조절하여 변경 가능