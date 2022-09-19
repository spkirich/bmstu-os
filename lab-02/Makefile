.PHONY: all clean

all: task-01 task-02

task-%: task-%.c
	$(CC) $< -o out/$@

clean:
	$(RM) out/*
