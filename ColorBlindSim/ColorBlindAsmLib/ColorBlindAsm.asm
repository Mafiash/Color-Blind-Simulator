; ==============================================================================
; Temat projektu:   Symulator Daltonizmu
; Opis algorytmu:   Implementacja przetwarzania obrazu:
;                   1. Czêœæ g³ówna: Równoleg³a pêtla wektorowa (AVX2) przetwarzaj¹ca
;                      4 piksele jednoczeœnie.
;                   2. Koñcówka: Pêtla skalarna obs³uguj¹ca resztki pikseli
;                      z wykorzystaniem zaawansowanego trybu adresowania [Base+Index*Scale].
;                      Wykorzystuje arytmetykê sta³oprzecinkow¹ (Fixed Point Q10).
;
; Semestr/Rok:      Semestr Zimowy 2025/2026
; Autor:            Mateusz Smuda
;
; Wersja:           2.1
; Historia zmian:
;   v1.0 - Implementacja podstawowa pêtli wektorowej AVX2.
;   v1.5 - Dodanie saturacji (Clamp) i obs³ugi liczb ujemnych.
;   v2.0 - Dodanie obs³ugi skalarnej (CheckRemainder i LoopScalar) dla obrazów o dowolnej szerokoœci.
;   v2.1 - Implementacja zaawansowanego trybu adresowania w pêtli skalarnej.
;
; ==============================================================================
.data
ALIGN 16  ; Wyrównanie pamiêci do 16 bajtów - optymalizacja ³adowania do rejestrów XMM/YMM

; ==============================================================================
; Sta³e: Wspó³czynniki macierzy Machado (Fixed Point Q10)
; Opis: Wartoœci zmiennoprzecinkowe przemno¿one przez 1024.
;       Format: 16-bitowe liczby ze znakiem.
;       Uk³ad: 4 powtórzenia tej samej wartoœci, aby pasowa³y do rejestrów wektorowych.
; ==============================================================================

; -- DEUTERANOPIA (Niewra¿liwoœæ na zieleñ) --
Deuter_B dw 4 dup(992, 44, -12, 0)    
Deuter_G dw 4 dup(48, 689, 287, 0)    
Deuter_R dw 4 dup(-233, 882, 376, 0) 

; -- PROTANOPIA (Niewra¿liwoœæ na czerwieñ) --
Protan_B dw 4 dup(1077, -49, -3, 0)
Protan_G dw 4 dup(-101, 805, 118, 0)
Protan_R dw 4 dup(-210, 1077, 156, 0)

; -- TRITANOPIA (Niewra¿liwoœæ na niebieski) --
Tritan_B dw 4 dup(-311, 708, 5, 0)
Tritan_G dw 4 dup(152, 953, -80, 0)
Tritan_R dw 4 dup(-183, -79, 1286, 0)

; -- TABLICA ADRESÓW --
; S³u¿y do szybkiego wyboru zestawu wspó³czynników bez instrukcji warunkowych.
CoeffTable dq Deuter_B, Deuter_G, Deuter_R
           dq Protan_B, Protan_G, Protan_R
           dq Tritan_B, Tritan_G, Tritan_R

PUBLIC RunAsm

.code

; ==============================================================================
; Nazwa procedury:  RunAsm
; Opis:             G³ówna procedura przetwarzaj¹ca obraz. Implementuje mno¿enie
;                   macierzowe RGB * Matrix.
;
; Parametry wejœciowe:
;   RCX - (unsigned char*) WskaŸnik do bufora danych obrazu (Format BGRA).
;   RDX - (int) Ca³kowita liczba pikseli do przetworzenia.
;   R8  - (int) Typ symulacji (0=Deuteranopia, 1=Protanopia, 2=Tritanopia).
;
; Parametry wyjœciowe:
;   Brak. Procedura modyfikuje pamiêæ wskazywan¹ przez RCX.
;
; Zmieniane rejestry i flagi:
;   Rejestry ogólne: RAX, RBX, RCX, RDX, RSI, RDI, R8-R15.
;   Rejestry wektorowe: YMM0 - YMM15.
;   Flagi: OF, SF, ZF, AF, PF, CF (modyfikowane przez instrukcje arytmetyczne).
;   Rejestry (RBX, RSI, RDI, R12-R15) s¹ zachowywane na stosie.
; ==============================================================================
RunAsm proc
    ; -- Zabezpieczenie rejestrów --
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15

    ; Wyczyszczenie górnych 128 bitów rejestrów YMM.
    vzeroupper 

    ; -- WALIDACJA DANYCH WEJŒCIOWYCH --
    test rcx, rcx             ; Sprawdzenie czy wskaŸnik imgData jest NULL
    jz Done                   ; Jeœli NULL, bezpieczne wyjœcie
    
    ; Zabezpieczenie liczników i typów
    mov r8d, r8d              ; Zerowanie górnych 32 bitów R8 (typ symulacji)
    mov eax, edx              ; Kopia liczby pikseli do EAX
    and eax, 7FFFFFFFh        ; Maskowanie bitu znaku (zabezpieczenie przed ujemn¹ liczb¹ pikseli)

    test eax, eax             ; Sprawdzenie czy liczba pikseli > 0
    jle Done

    ; -- PRZYGOTOWANIE DANYCH --
    mov r9d, eax              ; Kopia ca³kowitej liczby pikseli do R9D

    ; Wybór odpowiedniej macierzy z tablicy CoeffTable
    imul r8, 24               ; Obliczenie offsetu: Typ * 24 bajty (3 wskaŸniki * 8 bajtów)
    lea r10, CoeffTable       ; Za³adowanie adresu bazowego tablicy
    
    mov r11, [r10 + r8]       ; Pobranie adresu wspó³czynników Blue
    mov r12, [r10 + r8 + 8]   ; Pobranie adresu wspó³czynników Green
    mov r13, [r10 + r8 + 16]  ; Pobranie adresu wspó³czynników Red

    ; £adowanie wspó³czynników do rejestrów AVX (YMM)
    ; U¿ycie vmovdqu, co jest bezpieczne dla dowolnego wyrównania pamiêci.
    vmovdqu ymm13, YMMWORD PTR [r11] ; YMM13 = Macierz dla Blue
    vmovdqu ymm14, YMMWORD PTR [r12] ; YMM14 = Macierz dla Green
    vmovdqu ymm15, YMMWORD PTR [r13] ; YMM15 = Macierz dla Red

    mov rsi, rcx              ; RSI jest wskaŸnikiem bie¿¹cym do pikseli

    ; -- PÊTLA WEKTOROWA (AVX2) --
    shr eax, 2                ; Dzielenie liczby pikseli przez 4 (bo przetwarzane s¹ 4 naraz)
    jz CheckRemainder         ; Jeœli pikseli jest mniej ni¿ 4, pomijamy AVX i idziemy do sprawdzanie reszty
    mov ecx, eax              ; ECX jest licznikiem pêtli g³ównej

LoopVectors:
    ; £ADOWANIE I ROZPAKOWANIE
    ; Pobranie 4 pikseli (16 bajtów) i rozszerzenie bajtów (0-255) do s³ów (16-bit).
    ; Konieczne do unikniêcia przepe³nienia podczas mno¿enia przez 1024.
    vpmovzxbw ymm0, XMMWORD PTR [rsi] 
    vmovdqa ymm1, ymm0        ; Kopia danych dla kana³u Green
    vmovdqa ymm2, ymm0        ; Kopia danych dla kana³u Red

    ; OBLICZENIA MACIERZOWE
    ; Wzór: NewColor = (OldR*c1 + OldG*c2 + OldB*c3) / 1024
    
    ; Kana³ Blue
    vpmaddwd ymm0, ymm0, ymm13 ; Mno¿enie par liczb i sumowanie s¹siadów
    vphaddd  ymm0, ymm0, ymm0  ; Sumowanie poziome (zakoñczenie iloczynu skalarnego)
    vpsrad   ymm0, ymm0, 10    ; Dzielenie przez 1024 (Przesuniêcie arytmetyczne w prawo z zachowaniem znaku)

    ; Kana³ Green
    vpmaddwd ymm1, ymm1, ymm14
    vphaddd  ymm1, ymm1, ymm1
    vpsrad   ymm1, ymm1, 10

    ; Kana³ Red
    vpmaddwd ymm2, ymm2, ymm15
    vphaddd  ymm2, ymm2, ymm2
    vpsrad   ymm2, ymm2, 10

    ; ZAPIS WYNIKÓW (Ekstrakcja i Saturacja)
    ; Procesor nie pozwala na bezpoœredni zapis bajtów z YMM, wiêc wyci¹gane s¹ do EAX.
    
    ; -- PIKSEL 1 (Indeks wektora: 0) --
    vpextrd eax, xmm0, 0      ; Pobranie wyniku Blue (32-bit int)
    call Clamp8               ; Saturacja do zakresu 0-255
    mov BYTE PTR [rsi], al    ; Zapis bajtu do pamiêci
    vpextrd eax, xmm1, 0      ; Green
    call Clamp8
    mov BYTE PTR [rsi+1], al
    vpextrd eax, xmm2, 0      ; Red
    call Clamp8
    mov BYTE PTR [rsi+2], al

    ; -- PIKSEL 2 (Indeks wektora: 1) --
    vpextrd eax, xmm0, 1
    call Clamp8
    mov BYTE PTR [rsi+4], al
    vpextrd eax, xmm1, 1
    call Clamp8
    mov BYTE PTR [rsi+5], al
    vpextrd eax, xmm2, 1
    call Clamp8
    mov BYTE PTR [rsi+6], al

    ; Prze³¹czenie na górn¹ po³ówkê rejestrów YMM (dla pikseli 3 i 4)
    vextracti128 xmm3, ymm0, 1
    vextracti128 xmm4, ymm1, 1
    vextracti128 xmm5, ymm2, 1

    ; -- PIKSEL 3 --
    vpextrd eax, xmm3, 0   
    call Clamp8
    mov BYTE PTR [rsi+8], al
    vpextrd eax, xmm4, 0   
    call Clamp8
    mov BYTE PTR [rsi+9], al
    vpextrd eax, xmm5, 0   
    call Clamp8
    mov BYTE PTR [rsi+10], al

    ; -- PIKSEL 4 --
    vpextrd eax, xmm3, 1   
    call Clamp8
    mov BYTE PTR [rsi+12], al
    vpextrd eax, xmm4, 1   
    call Clamp8
    mov BYTE PTR [rsi+13], al
    vpextrd eax, xmm5, 1   
    call Clamp8
    mov BYTE PTR [rsi+14], al

    add rsi, 16               ; Przesuniêcie wskaŸnika o 4 piksele (16 bajtów)
    dec ecx                   ; Zmniejszenie licznika pêtli
    jnz LoopVectors           ; Skok na pocz¹tek, jeœli licznik > 0

; ==============================================================================
; Sekcja: CheckRemainder
; Opis:   Obs³uguje piksele, które nie zmieœci³y siê w pêtli wektorowej (np. 501 piksel).
;         Wykorzystuje zaawansowane adresowanie [Base + Index*Scale + Disp].
; ==============================================================================
CheckRemainder:
    and r9d, 3                ; Obliczenie reszty z dzielenia (numPixels % 4)
    jz Done                   ; Jeœli reszta wynosi 0, zakoñcz pracê

    ; Konfiguracja pêtli skalarnej
    mov r10d, r9d             ; R10D staje siê limitem pêtli (liczba pozosta³ych pikseli)
    xor r9d, r9d              ; Zerujemy R9D, który pos³u¿y jako INDEKS (0, 1, 2)

LoopScalar:
    ; -- DEMONSTRACJA ZAAWANSOWANEGO ADRESOWANIA --
    ; Adres = Base(RSI) + Index(R9) * Scale(4) + Displacement(0/1/2)
    ; Skala *4 wynika z faktu, ¿e piksel ma 4 bajty.
    
    ; Pobranie piksela do rejestrów 32-bitowych (Zero Extend)
    movzx ebx, byte ptr [rsi + r9*4]     ; Old Blue
    movzx edx, byte ptr [rsi + r9*4 + 1] ; Old Green
    movzx edi, byte ptr [rsi + r9*4 + 2] ; Old Red

    ; Obliczenie Blue (Skalarnie)
    ; Wzór: (c0*B + c1*G + c2*R) >> 10
    movsx eax, word ptr [r11]     ; Pobranie c0 (z rozszerzeniem znaku)
    imul eax, ebx                 ; Mno¿enie przez Blue
    movsx r8d, word ptr [r11+2]   ; Pobranie c1
    imul r8d, edx                 ; Mno¿enie przez Green
    add eax, r8d                  ; Sumowanie
    movsx r8d, word ptr [r11+4]   ; Pobranie c2
    imul r8d, edi                 ; Mno¿enie przez Red
    add eax, r8d                  ; Sumowanie
    sar eax, 10                   ; Dzielenie przez 1024 (Arytmetyczne)
    call Clamp8                   ; Saturacja
    
    ; Zapis wyniku Blue (z u¿yciem skalowania)
    mov byte ptr [rsi + r9*4], al 

    ; Obliczenie Green (Skalarnie)
    movsx eax, word ptr [r12]     
    imul eax, ebx                 
    movsx r8d, word ptr [r12+2]   
    imul r8d, edx                 
    add eax, r8d
    movsx r8d, word ptr [r12+4]   
    imul r8d, edi                 
    add eax, r8d
    sar eax, 10
    call Clamp8
    
    ; Zapis wyniku Green
    mov byte ptr [rsi + r9*4 + 1], al

    ; Obliczenie Red (Skalarnie)
    movsx eax, word ptr [r13]     
    imul eax, ebx                 
    movsx r8d, word ptr [r13+2]   
    imul r8d, edx                 
    add eax, r8d
    movsx r8d, word ptr [r13+4]   
    imul r8d, edi                 
    add eax, r8d
    sar eax, 10
    call Clamp8
    
    ; Zapis wyniku Red
    mov byte ptr [rsi + r9*4 + 2], al

    ; -- Sterowanie pêtl¹ skalarn¹ --
    inc r9d                   ; Zwiêksz indeks (kolejny piksel)
    cmp r9d, r10d             ; SprawdŸ czy osi¹gnêliœmy limit
    jl LoopScalar             ; Jeœli indeks < limit, powtórz

Done:
    vzeroupper                ; Sprz¹tanie po instrukcjach AVX
    
    ; -- Przywracanie rejestrów --
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret
RunAsm endp

; ==============================================================================
; Nazwa procedury:  Clamp8
; Opis:             Funkcja pomocnicza dokonuj¹ca saturacji (przyciêcia) liczby.
;                   Zabezpiecza przed przekroczeniem zakresu bajta [0, 255].
;                   Konieczna dla symulacji Tritanopii, gdzie wyniki mog¹ byæ ujemne.
;
; Parametry wejœciowe:
;   EAX - (int) Liczba ze znakiem do sprawdzenia.
;
; Parametry wyjœciowe:
;   EAX - (int) Liczba w zakresie 0-255 (wynik w AL).
;   Flagi: SF, ZF, OF - modyfikowane.
; ==============================================================================
Clamp8 proc
    test eax, eax
    js IsNegative             ; Skok jeœli bit znaku (Sign Flag) jest ustawiony -> liczba ujemna
    cmp eax, 255
    jg IsTooBig               ; Skok jeœli liczba > 255
    ret                       ; Jeœli 0-255, wracamy
IsNegative:
    xor eax, eax              ; Ustawienie wartoœci 0
    ret
IsTooBig:
    mov eax, 255              ; Ustawienie wartoœci 255
    ret
Clamp8 endp

end