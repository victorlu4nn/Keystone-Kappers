ALLEGRO_VERSION=5.0.10
PATH_ALLEGRO=C:\allegro-$(ALLEGRO_VERSION)-mingw-4.7.0
LIB_ALLEGRO=\lib\liballegro-$(ALLEGRO_VERSION)-monolith-mt.a
INCLUDE_ALLEGRO=\include

all: kappers.exe 

kappers.exe: kappers.o 
	gcc -o kappers.exe kappers.o $(PATH_ALLEGRO)$(LIB_ALLEGRO)	
	
kappers.o: kappers.c 
	gcc -I $(PATH_ALLEGRO)$(INCLUDE_ALLEGRO) -c kappers.c

clean:
	del kappers.o 
	del kappers.exe