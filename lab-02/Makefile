.PHONY: all clean

all: task-01 task-02 task-03 task-04

task-%: task-%.c
	$(CC) $< -o out/$@

clean:
	$(RM) out/*
