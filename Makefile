build: one_cond two_cond two_cond_wait

one_cond: one_cond.c pokemon.c 
	gcc -o one_cond -pthread one_cond.c pokemon.c

two_cond: two_cond.c pokemon.c 
	gcc -o two_cond -pthread two_cond.c pokemon.c

two_cond_wait: two_cond_wait.c pokemon.c 
	gcc -o two_cond_wait -pthread two_cond_wait.c pokemon.c

clean:
	rm one_cond two_cond two_cond_wait
