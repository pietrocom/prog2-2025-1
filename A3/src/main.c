//Compilação: gcc AggresiveSquares.c -o AS $(pkg-config allegro-5 allegro_main-5 allegro_font-5 --libs --cflags)

#include <allegro5/allegro5.h>											
#include <allegro5/allegro_font.h>	
#include <allegro5/allegro_image.h>									

int main() {
	al_init();														
	al_install_keyboard();											

	ALLEGRO_TIMER* timer = al_create_timer(1.0 / 30.0);				
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();			
	ALLEGRO_FONT* font = al_create_builtin_font();					
	ALLEGRO_DISPLAY* disp = al_create_display(320, 320);				

	al_register_event_source(queue, al_get_keyboard_event_source());		
	al_register_event_source(queue, al_get_display_event_source(disp));		
	al_register_event_source(queue, al_get_timer_event_source(timer));		

	if (!al_init_image_addon()) {
		printf("couldn't initialize image addon\n");
		return 1;
	}

	ALLEGRO_EVENT event;													
	al_start_timer(timer);
    
    int menu = 1;
	while (menu) {																
		al_wait_for_event(queue, &event);
		
		// Batida do clock
		if (event.type == 30){													
			al_clear_to_color(al_map_rgb(0, 0, 0));		
    		al_flip_display();											
		}

		else if (event.type == 42) break;	

        else if (event.type == ALLEGRO_KEY_DOWN) {
            switch (event.keyboard.keycode) {
                case ALLEGRO_KEY_W:
                    break;
                case ALLEGRO_KEY_S:
                    break;
                case ALLEGRO_KEY_ENTER:
                    break;
                default:
                    break;
            }
        }
	}

	al_destroy_font(font);	
	al_destroy_display(disp);			
	al_destroy_timer(timer);
	al_destroy_event_queue(queue);

	return 0;
}