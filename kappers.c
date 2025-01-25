#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdlib.h>
#include <math.h>


#define PESSOA_H 50
#define PESSOA_W 30
#define POLICIAL_VEL 7
#define BURGLAR_VEL 0
#define PISO_H 30
#define NUM_SALAS 3
#define NUM_ANDARES 3
#define GRAVIDADE 0.1
#define JUMP_VEL 3
#define DESCONTO_VEL_LAMA 0.85
#define NUM_MUDS 9


const float FPS = 100;  

const int SCREEN_W = 960;
const int SCREEN_H = 540;

int score = 0;

//------------ Global Vars ------------------
ALLEGRO_COLOR BKG_COLOR;
ALLEGRO_FONT *size_32;  	
int FLOOR_H;
int MALL_W;
int ANDAR_H;

void init_global_vars() {
	BKG_COLOR = al_map_rgb(15,15,15);
	size_32 = al_load_font("arial.ttf", 32, 1);   	
	MALL_W = SCREEN_W * NUM_SALAS;
	FLOOR_H = SCREEN_H / NUM_ANDARES;
}

//----------------------- STRUCTS ---------------------------------------


typedef struct Pessoa {
	float x, y, y_vel, x_mall;
	int x_esq, x_dir;
	float vel;
	int sala;
    int pulo;
	int pulos_restantes; 
	int andar;
	int andarMud;
	int k;
	ALLEGRO_COLOR cor;
} Pessoa;

typedef struct {
    int x, y, w;
} Mud;

//----------------------- Funções ---------------------------------------

void initGlobais(){
    BKG_COLOR = al_map_rgb(0,0,0);
	size_32 = al_load_font("arial.ttf", 32, 1);   	
	MALL_W = NUM_SALAS * SCREEN_W;
	ANDAR_H = SCREEN_H / NUM_ANDARES;
}

void desenhaCenario(Pessoa cop, Mud obst[NUM_SALAS][NUM_ANDARES]){
    al_clear_to_color(BKG_COLOR);

    int delta_cor = cop.sala * (255/NUM_SALAS);

    int i, y_andar;
	for(i=0; i<NUM_ANDARES; i++) {
		y_andar = SCREEN_H - (i*ANDAR_H);
		al_draw_filled_rectangle(0, y_andar - PISO_H,
							     SCREEN_W, y_andar,
							     al_map_rgb(0 + delta_cor, 255 - delta_cor, 0 + delta_cor));

	}

	// DESENHAR AS POÇAS
	
	for (int j = 0; j < NUM_ANDARES; j++) {
        	y_andar = SCREEN_H - (j * ANDAR_H);
        	al_draw_filled_rectangle(obst[cop.sala][j].x, obst[cop.sala][j].y - y_andar,
                                 	 obst[cop.sala][j].x + obst[cop.sala][j].w, obst[cop.sala][j].y - y_andar + PISO_H,
                                 	 al_map_rgb(131, 105, 83));
    }

	char text_score[4];
	sprintf(text_score, "%d", score);
	al_draw_text(size_32, al_map_rgb(102,178,255), SCREEN_W - 70, 20, 0, text_score);   


	
}

void desenhaPessoa(Pessoa p){

    if (p.x_dir == p.x_esq){
        al_draw_filled_triangle(p.x - PESSOA_W/2, p.y,
								p.x + PESSOA_W/2, p.y,
								p.x, p.y - PESSOA_H,
								p.cor);
    }

    else {
        al_draw_filled_triangle(p.x, p.y,
							    p.x, p.y - PESSOA_W,
							    p.x + PESSOA_H * pow(-1, 2*p.x_dir + p.x_esq), p.y - PESSOA_W/2,
							    p.cor);
    }

}

void desenhaBurglar(Pessoa p, Pessoa w){

    if (p.sala == w.sala) {
        al_draw_filled_triangle(p.x, p.y,
							    p.x, p.y - PESSOA_W,
							    p.x + PESSOA_H * pow(-1, 2*p.x_dir + p.x_esq), p.y - PESSOA_W/2,
							    p.cor);
    }

}

void initBurglar(Pessoa *p){
	p->cor = al_map_rgb(45,60, 195);
	p->x = SCREEN_W - PESSOA_W;
	p->x_mall = p->x + SCREEN_W;
	p->y = SCREEN_H - PISO_H - (SCREEN_H/NUM_ANDARES);
	p->vel = BURGLAR_VEL + NUM_ANDARES/2;
	p->x_esq = 0;
    p->x_dir = 0;
	p->sala = 1; 
	p->andar = 1;
	p->y_vel = 0;
}

void initPolicial(Pessoa *p) {
    p->cor = al_map_rgb(102, 178, 255);
    p->x = PESSOA_W;
    p->x_mall = p->x;
    p->y = SCREEN_H - PISO_H;
    p->x_esq = 0;
    p->x_dir = 0;
    p->vel = POLICIAL_VEL;
    p->sala = 0; 
	p->andar = 0;
	p->andarMud = NUM_ANDARES - 1;
    p->pulo = 0;
    p->y_vel = 0;
	p->k = 1;
	p->pulos_restantes = 2;
}

void elevador(Pessoa *p) {
	printf("\nUma pessoa chegou no elevador");
	if (p->sala == 2 && (p->andar)%2 == 0){
		p->y -= SCREEN_H/NUM_ANDARES;
		p->andar += 1;
		p->andarMud -= 1;
		p->x_dir = 0;
		p->x_esq = 0;
	}

	else if (p->sala == 2 && (p->andar) % 2 != 0) {
    	p->y += SCREEN_H / NUM_ANDARES;
    	p->andar -= 1;
    	p->andarMud += 1;
		p->x_dir = 0;
		p->x_esq = 0;
	}
	
	else if (p->sala == 0 && (p->andar)%2 != 0){
		p->y -= SCREEN_H/NUM_ANDARES;
		p->andar += 1;
		p->andarMud -= 1;
		p->x_dir = 0;
		p->x_esq = 0;
	}

	else if (p->sala == 0 && (p->andar)%2 == 0 && p->andar != 0){
		p->y += SCREEN_H/NUM_ANDARES;
		p->andar -= 1;
		p->andarMud += 1;
		p->x_dir = 0;
		p->x_esq = 0;
	}
}

void atualizaBurglar(Pessoa *burglar){

	if ((burglar->andar)%2 != 0){
		burglar->x_esq = 1;
		if(burglar->x_mall - PESSOA_H > 0)
			burglar->x_mall -= (burglar->vel);
		else {
			elevador(burglar);
		}
	}

	if ((burglar->andar)%2 == 0){
		burglar->x_esq = 0;
		burglar->x_dir = 1;
		if(burglar->x_mall + PESSOA_H < MALL_W)
			burglar->x_mall += (burglar->vel);
		else {
			elevador(burglar);
		}
	}

	burglar->sala = burglar->x_mall / SCREEN_W;
	burglar->x = (int)(burglar->x_mall) % SCREEN_W;
}


void atualizaPolicial(Pessoa *cop, Mud obst[NUM_SALAS][NUM_ANDARES]){

    if(cop->x_esq) {
		if(cop->x_mall - PESSOA_H > 0)
			cop->x_mall -= (cop->vel);
		else {
			elevador(cop);
		}
	}

	if(cop->x_dir) {
		if(cop->x_mall + PESSOA_H < MALL_W)
			cop->x_mall += (cop->vel);
		else {
			elevador(cop);
		}
	}

    if (cop->pulo && cop->k == 1) {
        cop->y_vel += GRAVIDADE; // Aplica gravidade
        cop->y += cop->y_vel;   // Atualiza posição vertical

        // Detecta colisão com o chão
        if (cop->y >= cop->andar*(-SCREEN_H/NUM_ANDARES) + SCREEN_H - PISO_H) {
            cop->y = cop->andar*(-SCREEN_H/NUM_ANDARES) + SCREEN_H - PISO_H; // Ajusta a posição ao chão
            cop->y_vel = 0;             // Para o movimento vertical
            cop->pulo = 0;              // Desativa o estado de pulo
			cop->pulos_restantes = 2;
        }
    }


	if (cop->x >= obst[cop->sala][cop->andarMud].x && 
		cop->x < obst[cop->sala][cop->andarMud].w + obst[cop->sala][cop->andarMud].x &&
		cop->y == SCREEN_H - PISO_H - (cop->andar*SCREEN_H/NUM_ANDARES)){
		
			cop->vel = 0.5;
			cop->k = 0;
	}
	else{
		cop->vel = 3;
		cop->k = 1;
	}
	
	cop->sala = cop->x_mall / SCREEN_W;
	cop->x = (int)(cop->x_mall) % SCREEN_W;
}

int colisaoCopBurglar(Pessoa cop, Pessoa burglar){

	if (cop.andar == burglar.andar &&
		cop.sala == burglar.sala &&
		cop.y >= burglar.y - PESSOA_W &&
		cop.x >= burglar.x - PESSOA_H/2 && cop.x <= burglar.x + PESSOA_H/2 )
			return 1;
	return 0;

}


int landraoVence(Pessoa l){
		
	if(NUM_ANDARES%2 != 0){
		if (l.sala == 2 &&
			l.andar == NUM_ANDARES)
				return  1;
		return 0;
	}
	else if (NUM_ANDARES%2 == 0){
		if (l.sala == 0 &&
			l.andar == NUM_ANDARES)
				return  1;
		return 0;
	}

}
 

//----------------------------------------------------------------------------


int main(int argc, char **argv){
	
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;
   
	//----------------------- rotinas de inicializacao ---------------------------------------
    
	//inicializa o Allegro
	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}
	
    //inicializa o módulo de primitivas do Allegro
    if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }	
	
	//inicializa o modulo que permite carregar imagens no jogo
	if(!al_init_image_addon()){
		fprintf(stderr, "failed to initialize image module!\n");
		return -1;
	}
   
	//cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}
 
	//cria uma tela com dimensoes de SCREEN_W, SCREEN_H pixels
	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display) {
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}

	//instala o teclado
	if(!al_install_keyboard()) {
		fprintf(stderr, "failed to install keyboard!\n");
		return -1;
	}


	//inicializa o modulo allegro que carrega as fontes
	al_init_font_addon();

	//inicializa o modulo allegro que entende arquivos tff de fontes
	if(!al_init_ttf_addon()) {
		fprintf(stderr, "failed to load tff font module!\n");
		return -1;
	}
	
	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 32, 1);   
	if(size_32 == NULL) {
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}

 	//cria a fila de eventos
	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		return -1;
	}
   
   //Musica

	// Inicializa o módulo de áudio
	if (!al_install_audio()) {
		fprintf(stderr, "Erro: Falha ao inicializar o áudio!\n");
		return -1;
	}

	// Inicializa o módulo de codecs de áudio
	if (!al_init_acodec_addon()) {
		fprintf(stderr, "Erro: Falha ao inicializar o codec de áudio!\n");
		return -1;
	}

	// Reserva canais de áudio para reprodução de música e efeitos sonoros
	if (!al_reserve_samples(1)) {
		fprintf(stderr, "Erro: Falha ao reservar canais de áudio!\n");
		return -1;
	}

	// Carrega a música (substitua "musica.ogg" pelo nome do arquivo de áudio)
	ALLEGRO_SAMPLE *musica = al_load_sample("m1.ogg");
	if (!musica) {
		fprintf(stderr, "Erro ao carregar a música 'musica.ogg'. Certifique-se de que o arquivo está no diretório correto.\n");
		return -1;
	}

	// Reproduz a música em loop
	if (!al_play_sample(musica, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL)) {
		fprintf(stderr, "Erro ao tentar reproduzir a música!\n");
		return -1;
	}



	//registra na fila os eventos de tela (ex: clicar no X na janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
	//registra na fila os eventos de tempo: quando o tempo altera de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	//registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source()); 	

    //--------------------------------------------------------------------------------------
    

	

    initGlobais();

	Pessoa cop, burglar;
	initPolicial(&cop);
	initBurglar(&burglar);

	//----------------- Obstacles -------------	

	Mud obst[NUM_SALAS][NUM_ANDARES];

	for(int i=0; i<NUM_SALAS; i++) {
		for(int j=0; j<NUM_ANDARES; j++) {

			obst[i][j].x =	50 + rand()%500;
			obst[i][j].y = SCREEN_H + ANDAR_H - PISO_H; 
			obst[i][j].w = rand()%150 + 80;

		}
	}

	//-----------------------------------------


    //inicia o temporizador
	al_start_timer(timer);

    //----------------------- Começando o Jogo  ---------------------------------------
    
    int playing = 1, ladrao_venceu = 0;

	while(playing) {

        ALLEGRO_EVENT ev;
		//espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);
		

		//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if(ev.type == ALLEGRO_EVENT_TIMER) {

			desenhaCenario(cop, obst);

			atualizaPolicial(&cop, obst);
			atualizaBurglar(&burglar);

			desenhaPessoa(cop);
			desenhaBurglar(burglar, cop);

			
			ladrao_venceu = landraoVence(burglar);
			playing = !landraoVence(burglar);

			if(!ladrao_venceu)
				playing = !colisaoCopBurglar(cop, burglar);




			//atualiza a tela (quando houver algo para mostrar)
			al_flip_display();
			
			if(al_get_timer_count(timer)%(int)FPS == 0) {
			    printf("\n%d segundos se passaram...", (int)(al_get_timer_count(timer)/FPS));
				score += 1;
			}
		}
        //se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
		}
		//se o tipo de evento for um pressionar de uma tecla
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			//imprime qual tecla foi
			//printf("\ncodigo tecla: %d", ev.keyboard.keycode);
			
			switch(ev.keyboard.keycode) {
				
				case ALLEGRO_KEY_A:
					cop.x_esq = 1;
					break;
					
				case ALLEGRO_KEY_D:
					cop.x_dir = 1;
					break;
					
				case ALLEGRO_KEY_SPACE:
					if (!cop.pulo) {
                        cop.pulo = 1;
                		cop.y_vel = -JUMP_VEL;
                		cop.pulos_restantes--; 
                    } break;
			}			
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
			//imprime qual tecla foi
			//printf("\ncodigo tecla: %d", ev.keyboard.keycode);
			
			switch(ev.keyboard.keycode) {
				
				case ALLEGRO_KEY_A:
					cop.x_esq = 0;
					break;
					
				case ALLEGRO_KEY_D:
					cop.x_dir = 0;
					break;
			}			
		}
    } //fim do while
     
	//procedimentos de fim de jogo (fecha a tela, limpa a memoria, etc)
	

		al_rest(1);
	
	if (ladrao_venceu) {
		al_destroy_sample(musica);
		al_uninstall_audio();
		// Limpa a tela com a cor preta
		al_clear_to_color(al_map_rgb(0, 0, 0));

		// Carrega a imagem "ending2.png"
		ALLEGRO_BITMAP *ending = al_load_bitmap("ending2.png");
		
		
		if (!ending) {
			fprintf(stderr, "Erro ao carregar a imagem 'ending2.png'.\n");
			return -1; // Certifique-se de lidar com erros caso a imagem não seja carregada
		}

		// Desenha a imagem armazenada na variável 'ending' nas posições x=20, y=0
		al_draw_bitmap(ending, 0, 0, 0);

		// Atualiza a tela para exibir a imagem
		al_flip_display();

		// Pausa a tela por 8.0 segundos
		al_rest(3.0);

		// Libera a memória da imagem carregada
		al_destroy_bitmap(ending);
	}
	else {
		
		al_destroy_sample(musica);
		al_uninstall_audio();

		char my_text[20];
    	char my_text2[20];
    	int record;
    	int is_new_record = 0;

    // Colore toda a tela de preto
    	al_clear_to_color(al_map_rgb(230, 240, 250));
    	sprintf(my_text, "Score: %d", score);
    	al_draw_text(size_32, al_map_rgb(200, 0, 30), SCREEN_W/3, SCREEN_H/2, 0, my_text);

    // Sistema de recorde
    	FILE *arq = fopen("record.txt", "r+"); // Abre para leitura e escrita
        // Se o arquivo não existir, cria um novo com um valor inicial alto
		if (arq == NULL) {
        // Se o arquivo não existir, cria um novo com um valor inicial alto
        	arq = fopen("record.txt", "w+");
        	fprintf(arq, "%d\n", INT_MAX); // Inicializa com o maior valor possível
        	rewind(arq); // Volta ao início do arquivo
    	}


    	int menor_score;
    	fscanf(arq, "%d", &menor_score);

    // Atualiza o recorde se o score atual for menor
    	if (score < menor_score) {
        	menor_score = score;
        	is_new_record = 1; // Marca que um novo recorde foi alcançado

        // Reescreve o arquivo com o novo recorde
        	freopen("record.txt", "w", arq);
        	if (arq != NULL) {
            	fprintf(arq, "%d\n", menor_score);
        	}
    	}

    // Exibe o recorde
    	sprintf(my_text2, "Record: %d", menor_score);
    	al_draw_text(size_32, al_map_rgb(200, 50, 30), SCREEN_W/3 + 200, SCREEN_H/2, 0, my_text2);

    // Notifica o jogador se ele bateu o recorde
    	if (is_new_record) {
        	al_draw_text(size_32, al_map_rgb(0, 200, 0), SCREEN_W/3, SCREEN_H/2 + 50, 0, "New Record!");
    	}

    	fclose(arq);

    // Reinicializa a tela
    	al_flip_display();
    	al_rest(5);
	}
 
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
   	

 
	    return 0;
    }
