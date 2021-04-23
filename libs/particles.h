#ifndef PARTICLES
#define PARTICLES 15
#endif

// Paint the particles
char particles[PARTICLES][2] ;

void initParticles() {
	for(unsigned char i = 0 ; i < PARTICLES ; ++i) {
		particles[i][0] = rand() % 128 ;
		particles[i][1] = rand() % 64 * -1 ;
	}
}

void drawParticles() {
	for(unsigned char i = 0 ; i < PARTICLES ; ++i) {
		particles[i][1] += (i + 1) % 3 ;

		if (particles[i][1] > 64) {
			particles[i][0] = rand() % 128 ;
			particles[i][1] = rand() % 64 * -1 ;
		}

		display.drawPixel(particles[i][0], particles[i][1], WHITE) ;
	}
}
