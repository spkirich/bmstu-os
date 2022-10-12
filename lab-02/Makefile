HC=ghc

# Флаги компиляции Haskell
HFLAGS=-fforce-recomp -outputdir out

# Фиктивные цели
.PHONY: all clean

all: task-01 task-02 task-03 task-04 task-05 prog-01 prog-02

task-%: task-%.c
	$(CC) $(CFLAGS) $< -o out/$@

prog-%: prog-%.hs
	$(HC) $(HFLAGS) $< -o out/$@

clean:
	$(RM) out/*
