# Операционные системы. Лабораторная работа 01

С помощью инструмента Sourcer был получен исходный код обработчика прерывания `08h`:

```asm
020A:0746  E8 0070				call	sub_15
020A:0749  06					push	es
020A:074A  1E					push	ds
020A:074B  50					push	ax
020A:074C  52					push	dx
020A:074D  B8 0040				mov	ax,40h
020A:0750  8E D8				mov	ds,ax
020A:0752  33 C0				xor	ax,ax
020A:0754  8E C0				mov	es,ax
020A:0756  FF 06 006C			inc	word ptr ds:[6Ch]
020A:075A  75 04				jnz	loc_67
020A:075C  FF 06 006E			inc	word ptr ds:[6Eh]
020A:0760			loc_67:
020A:0760  83 3E 006E 18		cmp	word ptr ds:[6Eh],18h
020A:0765  75 15				jne	loc_68
020A:0767  81 3E 006C 00B0		cmp	word ptr ds:[6Ch],0B0h
020A:076D  75 0D				jne	loc_68
020A:076F  A3 006E				mov	word ptr ds:[6Eh],ax
020A:0772  A3 006C				mov	word ptr ds:[6Ch],ax
020A:0775  C6 06 0070 01		mov	byte ptr ds:[70h],1
020A:077A  0C 08				or	al,8
020A:077C			loc_68:
020A:077C  50					push	ax
020A:077D  FE 0E 0040			dec	byte ptr ds:[40h]
020A:0781  75 0B				jnz	loc_69
020A:0783  80 26 003F F0		and	byte ptr ds:[3Fh],0F0h
020A:0788  B0 0C				mov	al,0Ch
020A:078A  BA 03F2				mov	dx,3F2h
020A:078D  EE					out	dx,al
020A:078E			loc_69:
020A:078E  58					pop	ax
020A:078F  F7 06 0314 0004		test	word ptr ds:[314h],4
020A:0795  75 0C				jnz	loc_70
020A:0797  9F					lahf
020A:0798  86 E0				xchg	ah,al
020A:079A  50					push	ax
020A:079B  26: FF 1E 0070		call	dword ptr es:[70h]
020A:07A0  EB 03				jmp	short loc_71
020A:07A2  90					nop
020A:07A3			loc_70:
020A:07A3  CD 1C				int	1Ch
020A:07A5			loc_71:
020A:07A5  E8 0011				call	sub_15
020A:07A8  B0 20				mov	al,20h
020A:07AA  E6 20				out	20h,al
020A:07AC  5A					pop	dx
020A:07AD  58					pop	ax
020A:07AE  1F					pop	ds
020A:07AF  07					pop	es
020A:07B0  E9 FE99				jmp	loc_50
```

Метка `loc_50` здесь отсылает к команде `iret`:

```asm
020A:064C			loc_50:
020A:064C  1E					push	ds
020A:064D  50					push	ax
...
020A:06AA  58					pop	ax
020A:06AB  1F					pop	ds
020A:06AC  CF					iret
				sub_10		endp
```

Также был получен исходный код подпрограммы `sub_15`:

```asm
sub_15 proc near
; Сохранить значения регистров DS, AX
020A:07B9 push ds
020A:07BA push ax
; Поместить в регистр DS слово 0040h
020A:07BB mov ax, 0040h
020A:07BE mov ds, ax
; Сохранить младший байт регистра FLAGS в AH
020A:07C0 lahf
; Установлен DF или старший бит IOPL?
020A:07C1 test word ptr ds:[314h], 2400h
020A:07C7 jnz loc_73
; Сбростить IF с блокировкой шины данных
020A:07C9 lock and word ptr ds:[314h], 0FDFFh
020A:07D0 loc_72:
; Восстановить младший байт регистра FLAGS из AH
020A:07D0 sahf
; Восстановить значения регистров AX, DS
020A:07D1 pop ax
020A:07D2 pop ds
020A:07D3 jmp short loc_74
020A:07D5 loc_73:
; Запретить маскируемые прерывания
020A:07D5 cli
020A:07D6 jmp short loc_72
020A:07D8 loc_74:
020A:07D8 retn
sub_15 endp
```

По исходному коду обработчика прерывания `08h` была составлена схема его алгоритма:

![Рисунок 1. Схема алгоритма обработчика прерывания `08h`](img/fig-01.png)

Также была составлена схема алгоритма подпрограммы `sub_15`:

![Рисунок 2. Схема алгоритма подпрограммы `sub_15`](img/fig-02.png)
