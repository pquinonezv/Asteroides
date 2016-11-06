#include "SFML/Graphics.hpp"
#include <time.h>
#include <list>
using namespace sf;

//tamaño de la pantalla
const int W = 1200;
const int H = 800;

//convierte de grados a radianes
float DEGTORAD = 0.017453f;

//se encarga de animar cual sprite, pasando 
//textura, posicion, tamaño, cantidad imagenes para animacion y velocidad de la animacion
class Animation
{
public:
	float Frame, speed;
	Sprite sprite;
	std::vector<IntRect> frames;

	Animation() {}

	//constructor que agrega al vector frames cada una de las imagenes de la animacion
	//ademas asigna la textura, el ancla del sprite y asigna la textura actual
	Animation(Texture &t, int x, int y, int w, int h, int count, float Speed)
	{
		Frame = 0;
		speed = Speed;

		for (int i = 0; i<count; i++)
			frames.push_back(IntRect(x + i*w, y, w, h));

		sprite.setTexture(t);
		sprite.setOrigin(w / 2, h / 2);
		sprite.setTextureRect(frames[0]);
	}

	//va cambiando la imagen cada determinado tiempo (Frame)
	void update()
	{
		Frame += speed;
		int n = frames.size();
		if (Frame >= n) Frame -= n;
		if (n>0) sprite.setTextureRect(frames[int(Frame)]);
	}

	//validar si ya termino la animacion
	bool isEnd()
	{
		return Frame + speed >= frames.size();
	}

};

//Clase que se encarga las propiedades basicas de cada sprite tal como:
//mover, animar(utilizando la clase de arriba), recibir opciones del teclado
class Entity
{
public:
	float x, y, dx, dy, R, angle;
	bool life;
	std::string name;
	Animation anim;

	//asigna vida a la entidad
	Entity()
	{
		life = 1;
	}

	//asigna las propiedades de la entidad tal como Clase animacion, posicion y angulo
	void settings(Animation &a, int X, int Y, float Angle = 0, int radius = 1)
	{
		anim = a;
		x = X; y = Y;
		angle = Angle;
		R = radius;
	}

	virtual void update() {};

	//dibuja la entidad
	void draw(RenderWindow &app)
	{
		anim.sprite.setPosition(x, y);
		anim.sprite.setRotation(angle + 90);
		app.draw(anim.sprite);

		CircleShape circle(R);
		circle.setFillColor(Color(255, 0, 0, 170));
		circle.setPosition(x, y);
		circle.setOrigin(R, R);
		//app.draw(circle);
	}

	virtual ~Entity() {};
};

//Clase Asteroide que se encarga de ir moviendo los asteroides y validando colisiones de pantalla
class asteroid : public Entity
{
public:
	asteroid()
	{
		//dx, dy se va a encargar del movimiento del asteroide de manera aleatoria
		dx = rand() % 8 - 4;
		dy = rand() % 8 - 4;
		//asigna nombre asteroid para el momento de las colisiones
		name = "asteroid";
	}

	//se encarga de refrescar la posicion del asteroide y colisiones con pantall
	void  update()
	{
		x += dx;
		y += dy;

		if (x>W) x = 0;  if (x<0) x = W;
		if (y>H) y = 0;  if (y<0) y = H;
	}

};


//Clase bala que utiliza la nave para disparar
class bullet : public Entity
{
public:
	//asigna nombre bullet para utilizarlo en las colisiones
	bullet()
	{
		name = "bullet";
	}

	//actualiza las propiedades de posicion y rotacion de la bala
	void  update()
	{
		//mediante coseno y seno determina el nuevo angulo que va a viajar la bala
		dx = cos(angle*DEGTORAD) * 6;
		dy = sin(angle*DEGTORAD) * 6;
		
		//mueve la bala de acuerdo al valor incremento dx,dy estimado con coseno y seno
		x += dx;
		y += dy;

		//estima que si sale de pantalla borre
		if (x>W || x<0 || y>H || y<0) life = 0;
	}

};

//Clase que se encarga de manejar a la nave jugadora
class player : public Entity
{
public:
	//es el encargado de saber si el usuario esta acelerando la navecita
	bool thrust;
	int muertes = 0;

	//asigna nombre player para utilizarlo en colisiones
	player()
	{
		name = "player";
	}

	//actualiza movimiento, angulo de la nave
	void update()
	{
		//si esta presionando la tecla de arriba osea que esta acelerando
		if (thrust)
		{
			//calcula el nuevo incremento dx, dy con coseno,seno y angulo que tiene la nave
			dx += cos(angle*DEGTORAD)*0.2;
			dy += sin(angle*DEGTORAD)*0.2;
		}
		else
		{
			//si no esta presionada la tecla el incremento es el 1% por lo cual se va ir frenando
			dx *= 0.99;
			dy *= 0.99;
		}

		//la maxima velocidad de la nave es de 15
		int maxSpeed = 15;
		//formula para sacar la velocidad de la nave
		float speed = sqrt(dx*dx + dy*dy);

		//si velocidad es mayor a la velocidad maxima va multiplicarle a dx,dy la division de ambas
		//por lo cual va ser 1 o menor de modo que ya no acelere mas
		if (speed>maxSpeed)
		{
			dx *= maxSpeed / speed;
			dy *= maxSpeed / speed;
		}

		//suma a la posicion de la nave el incremento de movimiento
		x += dx;
		y += dy;

		//valida si sale de la pantalla que la ponga al otro lado
		if (x>W) x = 0; if (x<0) x = W;
		if (y>H) y = 0; if (y<0) y = H;
	}

};

//funcion que valida si dos entidades tienen colisicion por medio de una formula
bool isCollide(Entity *a, Entity *b)
{
	return (b->x - a->x)*(b->x - a->x) +
		(b->y - a->y)*(b->y - a->y)<
		(a->R + b->R)*(a->R + b->R);
}


int main()
{
	srand(time(0));

	//inicia la pantalla del juego
	RenderWindow app(VideoMode(W, H), "Asteroids!");
	//asigna un limite de imagenes por segundo a 60 imagenes por segundo
	app.setFramerateLimit(60);

	//carga las texturas que se usaran en el juego de la carpeta images
	Texture t1, t2, t3, t4, t5, t6, t7;
	t1.loadFromFile("images/spaceship.png");
	t2.loadFromFile("images/background.jpg");
	t3.loadFromFile("images/explosions/type_C.png");
	t4.loadFromFile("images/rock.png");
	t5.loadFromFile("images/fire_blue.png");
	t6.loadFromFile("images/rock_small.png");
	t7.loadFromFile("images/explosions/type_B.png");

	//hace que la nave y fondo tenga un suavizado
	t1.setSmooth(true);
	t2.setSmooth(true);

	//Se crea el sprite del fondo osea el cielo oscuro
	Sprite background(t2);

	//Se crean todos los objetos Animacion, el cual se les asignaron las texturas de arriba
	//pero cada textura tiene una lista de imagenes que juntas realizan una animacion
	//y la clase animacion va a generar esa animacion para cada sprite
	//por lo cual los parametros de la animacion son:
	//textura, posicion incial, tamaño, num imagenes, velocidad de la imagen
	Animation sExplosion(t3, 0, 0, 256, 256, 48, 0.5);
	Animation sRock(t4, 0, 0, 64, 64, 16, 0.2);
	Animation sRock_small(t6, 0, 0, 64, 64, 16, 0.2);
	Animation sBullet(t5, 0, 0, 32, 64, 16, 0.8);
	Animation sPlayer(t1, 40, 0, 40, 40, 1, 0);
	Animation sPlayer_go(t1, 40, 40, 40, 40, 1, 0);
	Animation sExplosion_ship(t7, 0, 0, 192, 192, 64, 0.5);

	//lista principal que contiene a todos los objetos en pantalla escepto el fondo
	//tiene a los asteroides, nave y balas ya que al final todos son hijos de la clase entities
	//El juego se va iterar y recorrer este arreglo para mover, animar y redibujar las entidades
	std::list<Entity*> entities;


	//Se llena arreglo con clases asteroides pasandole la textura correspondiente
	//las posiciones aleatorias, el angulo aletario y el radio del circulo que aparece en las explosiones
	for (int i = 0; i<1; i++)
	{
		asteroid *a = new asteroid();
		a->settings(sRock, rand() % W, rand() % H, rand() % 360, 25);
		entities.push_back(a);
	}

	//crea apuntador de la clase player para la navecita
	player *p = new player();
	//asigna la textura del jugador, posiciones 200,200 angulo 0 y radio del circulo 20
	p->settings(sPlayer, 200, 200, 0, 20);
	entities.push_back(p);



	//Creamos una fuente1
	Font font;
	//La cargamos de nuestra carpeta fonts
	font.loadFromFile("fonts/RuslanDisplay.ttf");
	//Creamos un texto
	Text texto;
	//Establecemos el tipo de fuente, sobre el que cargamos hace rato
	texto.setFont(font);
	//Establecemos el tamaño
	texto.setCharacterSize(30);
	//Establecemos el estilo
	texto.setStyle(Text::Regular);
	//defino la posicion del texto en y es -15 ya que en 0 lo toma muy abajo
	texto.setPosition(0, -10);
	texto.setColor(Color::Blue);

	
	//Ciclo hasta que se cierra la ventana
	while (app.isOpen())
	{
		//Encargado de los eventos del teclado y mouse
		Event event;

		while (app.pollEvent(event))
		{
			//en caso de que se cierre la ventana
			if (event.type == Event::Closed)
				app.close();

			//si se presiona escape genera las balas 
			if (event.type == Event::KeyPressed)
				if (event.key.code == Keyboard::Space)
				{
					//crea apuntador clase bala
					bullet *b = new bullet();
					//asgina las propiedades que tiene la nave
					b->settings(sBullet, p->x, p->y, p->angle, 10);
					//agrega a la lista de entidades para luego que se mueva, anime
					entities.push_back(b);
				}
		}

		//valida si se presiono una flecha del teclado y le modifica el angulo a la nave
		//la variable thrust se encarga de validar si esta acelerando o no la nave
		if (Keyboard::isKeyPressed(Keyboard::Right)) p->angle += 3;
		if (Keyboard::isKeyPressed(Keyboard::Left))  p->angle -= 3;
		if (Keyboard::isKeyPressed(Keyboard::Up)) p->thrust = true;
		else p->thrust = false;


		//se encarga de recorrer cada elemento del arreglo entities contra el mismo arreglo
		for (auto a : entities)
			for (auto b : entities)
			{
				//si el elemento es asteroid y el otro es bullet valida colision
				if (a->name == "asteroid" && b->name == "bullet")
					//funcion que con una formula valida la colision de ambos objetos
					if (isCollide(a, b))
					{
						//quita la vida de ambos objetos
						a->life = false;
						b->life = false;

						//crea nueva entidad para asignar la textura explosion en lugar del asteroide
						Entity *e = new Entity();
						//pone la textura explosion en la posicion x,y del primer elemento
						e->settings(sExplosion, a->x, a->y);
						e->name = "explosion";
						//lo agrega a la lista de entidades
						entities.push_back(e);

						//agrega dos meteoritos mas chiquitos a partir del mas grande
						for (int i = 0; i < 2; i++)
						{
							//valida si es el radio de un meteorito chiquito ignora lo de abajo
							if (a->R == 15) continue;

							//crea nueva entidad de tipo asteroide
							Entity *e = new asteroid();
							//asigna una textura de asteroide pequeño le pone las mismas posciones del asteroide con un angulo diferente
							//pero con un radio de 15 ya que el asteroide grande tiene un radio de 25
							e->settings(sRock_small, a->x, a->y, rand() % 360, 15);
							//lo agrega a la lista de animaciones
							entities.push_back(e);
						}

					}

				//valida si uno es el jugador y el otro asteroide
				if (a->name == "player" && b->name == "asteroid")
					//valida si hay colision por medio de una formula
					if (isCollide(a, b))
					{
						//le quita la vida al asteroide
						b->life = false;			

						//crea una entidad para la explosion en lugar del asteroide
						Entity *e = new Entity();
						//asigna la explosion de la nave con las posiciones de la nave
						e->settings(sExplosion_ship, a->x, a->y);
						e->name = "explosion";
						//lo agrega a la lista de animaciones
						entities.push_back(e);
												
						//el jugador tiene las posiciones centrales de la pantalla
						p->settings(sPlayer, W / 2, H / 2, 0, 20);
						//su velocidad es 0
						p->dx = 0; p->dy = 0;
						p->muertes++;
											
					}
			}

		//valida que si esta encendida la aceleracion la animacion va hacer sobre la textura con lumbre
		//sino sobre la normal
		if (p->thrust)  p->anim = sPlayer_go;
		else   p->anim = sPlayer;

		//recorre todas las entidades para validar si una se llama explosion
		//valida si ya termino la animacion explosion le quita la vida
		for (auto e : entities)
			if (e->name == "explosion")
				if (e->anim.isEnd()) e->life = 0;

		//de manera aleatoria si cae un numero cero agrega en el escenraio un nuevo asteroide
		if (rand() % 150 == 0)
		{
			asteroid *a = new asteroid();
			a->settings(sRock, 0, rand() % H, rand() % 360, 25);
			entities.push_back(a);
		}

		//recorre todas las entidades y les actualiza las texturas para simular las animaciones
		for (auto i = entities.begin(); i != entities.end();)
		{
			Entity *e = *i;

			//la entidad invocan la entidad update que se encarga de animar el objeto
			//puede ser un asteroide, nave, o bala
			e->update();
			e->anim.update();

			//si la entidad ya no tiene vida la elimina de la lista
			if (e->life == false) { i = entities.erase(i); delete e; }
			else i++;			
		}
		
		//Pinta el fondo de pantalla, el espacio exterior
		app.draw(background);


		// creo arreglo char para armar texto de puntuacion
		char salida[50] = { 0 };
		//asigno a salida el siguiente texto
		sprintf(salida, "Numero de muertes: %d", p->muertes);
		//asigno salida a nuestro texto que se mostrara por pantalla
		texto.setString(salida);
		//pinto texto a pantalla
		app.draw(texto);


		//recorre todas las entidades e invoca su metodo draw que se encarga de pintar cada entidad
		for (auto i : entities)
			i->draw(app);

		//refresca la pantalla y ahora si pinta todo lo que se mando a pintar
		app.display();
	}

	return 0;
}
