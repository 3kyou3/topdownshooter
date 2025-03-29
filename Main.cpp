#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <string>
#include <cmath>
#define _USE_MATH_DEFINES
#include <vector>
#include <random>

const double M_PI = 3.14159265358979323846;
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 900;

float playerPosX = 640.0f;
float playerPosY = 450.0f;
const float playerSizeX = 50.0f;
const float playerSizeY = 30.0f;
float deltaTime;
bool isGameOver = false;

float playerVelocityX = 0.0f;
float playerVelocityY = 0.0f;
const float PlayerAcceleration = 7000.0f;
const float friction = 600.0f; 
const float playerMaxSpeed = 250.0f;

const int playerRed = 0;
const int playerGreen = 100;
const int playerBlue = 157;
const int playerAplha = 255;

const float gunSizeX = 35.0;
const float gunSizeY = 15.0;
float gunPosX;
float gunPosY;

bool pauseButton = false;

//enemyobject setting
const float obSizeWidth = 50.0f;
const float obSizeHeight = 45.0f;
const float obfriction = 500.0f;
const float m_bullet = 1.0f;
const float m_object = 10.0f;
const float obFullAcceleration = 200.0f;
const float recoverTime = 0.1f;

//spawn enemy setting
const Uint32 spawnEnemyInterval = 4000;
Uint64 lastSpawnTime = 0;
const Uint32 enemyDuration = 45000;

//bullet setting
float bulletAcceleration = 2500.0f;
float bulletSizeWidth = 15.0f;
float bulletSizeHeight = 15.0f;
const Uint32 bulletLifeTime = 2000;
const int bulletRed = 255;
const int bulletGreen = 255;
const int bulletBlue = 0;
const int bulletAplha = 255;
struct Bullet
{
	float x, y;
	float dirX, dirY;
	float width, height;
	bool alreadyHitEnemy;
	Uint64 spawnTime;
	bool isExpired;
	Bullet(float startX, float startY, float targetX, float targetY) : alreadyHitEnemy(false), isExpired(false)
	{
		x = startX;
		y = startY;
		dirX = targetX - startX;
		dirY = targetY - startY;
		
		float length = sqrt(dirX * dirX + dirY * dirY);
		if (length != 0)
		{
			dirX /= length;
			dirY /= length;
		}
		width = bulletSizeWidth;
		height = bulletSizeHeight;
		spawnTime = SDL_GetTicks();
	}
	void Update(float deltaTime)
	{
		if (!isExpired && (SDL_GetTicks() - spawnTime) >= bulletLifeTime)
		{
			isExpired = true;
			return;
		}
		if (!isExpired)
		{
			x += dirX * bulletAcceleration * deltaTime;
			y += dirY * bulletAcceleration * deltaTime;
		}
	}
};
std::vector<Bullet> bullets;

//shot gun bullet setting
float shotbulletAcceleration = 2500.0f;
float shotbulletSizeWidth = 9.2f;
float shotbulletSizeHeight = 9.2f;
const Uint32 shotbulletLifeTime = 2000;
const int shotbulletRed = 255;
const int shotbulletGreen = 150;
const int shotbulletBlue = 0;
const int shotbulletAplha = 255;
struct shotBullet
{
	float x, y;
	float dirX, dirY;
	float width, height;
	bool alreadyHitEnemy;
	Uint64 spawnTime;
	bool isExpired;
	shotBullet(float startX, float startY, float targetX, float targetY) : alreadyHitEnemy(false), isExpired(false)
	{
		x = startX;
		y = startY;
		dirX = targetX - startX;
		dirY = targetY - startY;

		float length = sqrt(dirX * dirX + dirY * dirY);
		if (length != 0)
		{
			dirX /= length;
			dirY /= length;
		}
		width = shotbulletSizeWidth;
		height = shotbulletSizeHeight;
		spawnTime = SDL_GetTicks();
	}
	void Update(float deltaTime)
	{
		if (!isExpired && (SDL_GetTicks() - spawnTime) >= bulletLifeTime)
		{
			isExpired = true;
			return;
		}
		if (!isExpired)
		{
			x += dirX * shotbulletAcceleration * deltaTime;
			y += dirY * shotbulletAcceleration * deltaTime;
		}
	}
};
std::vector<shotBullet> shotbullets;

struct Enemy
{
	float x, y;
	float dirX, dirY;
	float width, height;
	float obVelX = 0.0f;
	float obVelY = 0.0f;
	float obAcceleration = 200.0f;
	bool enemyGothit;
	bool enemyExpired;
	Uint32 hitDuration = 1500;
	Uint32 hitStartTime = 0;
	Uint64 enemyLiftTimeStart = 0;
	Enemy(float startX, float startY) : enemyGothit(false), enemyExpired(false), dirX(0.0f), dirY(0.0f)
	{
		x = startX;
		y = startY;
	
		width = obSizeWidth;
		height = obSizeHeight;
		enemyLiftTimeStart = SDL_GetTicks();
	}
	void Update(float deltaTimeForEnemy, float targetX, float targetY)
	{
		if (SDL_GetTicks() - enemyLiftTimeStart >= enemyDuration)
		{
			enemyExpired = true;
		}
		if (enemyGothit)
		{
			x += obVelX * deltaTimeForEnemy;
			y += obVelY * deltaTimeForEnemy;
		}
		else
		{
			dirX = targetX - x;
			dirY = targetY - y;

			float length = sqrt(dirX * dirX + dirY * dirY);
			if (length != 0)
			{
				dirX /= length;
				dirY /= length;
			}
			if (dirX != 0.0f || dirY != 0.0f)
			{
				float accelerationX = dirX * obAcceleration;
				float accelerationY = dirY * obAcceleration;

				obVelX += accelerationX * deltaTime;
				obVelY += accelerationY * deltaTime;
			}
			obVelX = dirX * obAcceleration;
			obVelY = dirY * obAcceleration;
			x += obVelX * deltaTimeForEnemy;
			y += obVelY * deltaTimeForEnemy;
		}
	}
};
std::vector<Enemy> enemy;

int playerControl()
{
	//playermovement
	const bool* keyboardState = SDL_GetKeyboardState(NULL);
	keyboardState = SDL_GetKeyboardState(NULL);

	float moveX = 0.0f;
	float moveY = 0.0f;
	if (keyboardState[SDL_SCANCODE_W]) moveY -= 1.0f; 
	if (keyboardState[SDL_SCANCODE_S]) moveY += 1.0f;
	if (keyboardState[SDL_SCANCODE_A]) moveX -= 1.0f;
	if (keyboardState[SDL_SCANCODE_D]) moveX += 1.0f;

	if (moveX != 0.0f || moveY != 0.0f)
	{
		float length = sqrt(moveX * moveX + moveY * moveY);
		moveX /= length;
		moveY /= length;
	}
	if (moveX != 0.0f || moveY != 0.0f)
	{
		float accelerationX = moveX * PlayerAcceleration;
		float accelerationY = moveY * PlayerAcceleration;

		playerVelocityX += accelerationX * deltaTime;
		playerVelocityY += accelerationY * deltaTime;
	}
	else
	{
		if (playerVelocityX > 0)
		{
			playerVelocityX -= friction * deltaTime;
			if (playerVelocityX < 0) playerVelocityX = 0;
		}
		else if (playerVelocityX < 0)
		{
			playerVelocityX += friction * deltaTime;
			if (playerVelocityX > 0) playerVelocityX = 0;
		}

		if (playerVelocityY > 0)
		{
			playerVelocityY -= friction * deltaTime;
			if (playerVelocityY < 0) playerVelocityY = 0;
		}
		else if (playerVelocityY < 0)
		{
			playerVelocityY += friction * deltaTime;
			if (playerVelocityY > 0) playerVelocityY = 0;
		}
	}

	float currentPlayerSpeed = sqrt(playerVelocityX * playerVelocityX + playerVelocityY * playerVelocityY);
	if (currentPlayerSpeed > playerMaxSpeed)
	{
		float scale = playerMaxSpeed / currentPlayerSpeed;
		playerVelocityX *= scale;
		playerVelocityY *= scale;
	}
	playerPosX += playerVelocityX * deltaTime;
	playerPosY += playerVelocityY * deltaTime;
	return 0;
}

void recoverEnemyMovement(float deltaTimeForEnemy, Enemy& e)
{
	if (e.enemyGothit && SDL_GetTicks() - e.hitStartTime >= e.hitDuration && e.obAcceleration < obFullAcceleration)
	{
		e.obAcceleration += (obFullAcceleration / recoverTime) * deltaTime;
		if (e.obAcceleration > obFullAcceleration)
		{
			e.obAcceleration = obFullAcceleration;
			e.enemyGothit = false;
		}
	}
}

void SpawnBullet(float mouseX, float mouseY)
{
	Bullet newBullet(playerPosX + playerSizeX / 2, playerPosY + playerSizeY / 2, mouseX, mouseY);
	bullets.push_back(newBullet);
}

void SpawnShotBullet(float mouseX, float mouseY)
{
	int numberBulletOfSingShot = 10;
	float spreadAngle = 20.0f;
	
	float startX = playerPosX + playerSizeX / 2;
	float startY = playerPosY + playerSizeY / 2;

	float targetDirectionX = mouseX - startX;
	float targetDirectionY = mouseY - startY;
	float baseLength = sqrt(targetDirectionX * targetDirectionX + targetDirectionY * targetDirectionY);
	float baseDirX = 0.0f;
	float baseDirY = 0.0f;
	if (baseLength != 0)
	{
		baseDirX = targetDirectionX / baseLength;
		baseDirY = targetDirectionY / baseLength;
	}
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> randomAngle(-spreadAngle / 2.0f, spreadAngle / 2.0f);

	for (int i = 0; i < numberBulletOfSingShot; ++i)
	{
		float angle = randomAngle(gen);
		float radians = angle * M_PI / 180.0f;
		float newDirX = baseDirX * cos(radians) - baseDirY * sin(radians);
		float newDirY = baseDirX * sin(radians) + baseDirY * cos(radians);
		float targetX = startX + newDirX;
		float targetY = startY + newDirY;
		shotBullet newShotBullet(startX, startY, targetX, targetY);
		shotbullets.push_back(newShotBullet);
	}
}

void SpawnEnemy(std::uniform_real_distribution<float>* distX, std::uniform_real_distribution<float>* distY, std::default_random_engine* engine)
{
	float x = (*distX)(*engine);
	float y = (*distY)(*engine);
	Enemy newEnemy(x, y);
	enemy.push_back(newEnemy);
}

void handleBulletCollision(Bullet& a, Bullet& b)
{
	float bulletCenterX = a.x + a.width / 2;
	float bulletCenterY = a.y + a.height / 2;
	float otherbulletCenterX = b.x + b.width / 2;
	float otherbulletCenterY = b.y + b.height / 2;
	float normalX = otherbulletCenterX - bulletCenterX;
	float normalY = otherbulletCenterY - bulletCenterY;
	float length = sqrt(normalX * normalX + normalY * normalY);
	if (length == 0.0f) return;
	normalX /= length;
	normalY /= length;

	float relativeVelocityX = b.dirX - a.dirX;
	float relativeVelocityY = b.dirY - a.dirY;

	float velocityAlongNormal = relativeVelocityX * normalX + relativeVelocityY * normalY;
	if (velocityAlongNormal > 0) return;
	
	float restitution = 0.8f;
	float impulseScalar = -(1 + restitution) * velocityAlongNormal;
	impulseScalar /= 2;

	a.dirX -= impulseScalar * normalX;
	a.dirY -= impulseScalar * normalY;
	b.dirX += impulseScalar * normalX;
	b.dirY += impulseScalar * normalY;
}

void handleBulletPlayerCollision(Bullet& bullet)
{
	float bulletCenterX = bullet.x + bullet.width / 2;
	float bulletCenterY = bullet.y + bullet.height / 2;
	float playerCenterX = playerPosX + playerSizeX / 2;
	float playerCenterY = playerPosY + playerSizeY / 2;

	float normalX = bulletCenterX - playerCenterX;
	float normalY = bulletCenterY - playerCenterY;
	float length = sqrt(normalX * normalX + normalY * normalY);
	if (length == 0.0f) return;

	normalX /= length;
	normalY /= length;

	float bulletVelX = bullet.dirX * bulletAcceleration;
	float bulletVelY = bullet.dirY * bulletAcceleration;
	float relativeVelX = bulletVelX - playerVelocityX;
	float relativeVelY = bulletVelY - playerVelocityY;

	float velocityAlongNormal = relativeVelX * normalX + relativeVelY * normalY;
	if (velocityAlongNormal > 0) return;
	float restitution = 0.5f;
	float impulseScalar = -(1 + restitution) * velocityAlongNormal;
	bullet.dirX += (impulseScalar * normalX) / bulletAcceleration;
	bullet.dirY += (impulseScalar * normalY) / bulletAcceleration;
}

void handleBulletObjectCollision(Bullet& bullet, Enemy& e)
{
	float bulletCenterX = bullet.x + bullet.width / 2;
	float bulletCenterY = bullet.y + bullet.height / 2;
	float objectCenterX = e.x + obSizeWidth / 2;
	float objectCenterY = e.y + obSizeHeight / 2;

	float normalX = bulletCenterX - objectCenterX;
	float normalY = bulletCenterY - objectCenterY;
	float distance = sqrt(normalX * normalX + normalY * normalY);
	if (distance == 0.0f) return;

	normalX /= distance;
	normalY /= distance;

	float vb_x = bullet.dirX * bulletAcceleration;
	float vb_y = bullet.dirY * bulletAcceleration;
	float vo_x = e.obVelX;
	float vo_y = e.obVelY;

	float rel_vel_x = vb_x - vo_x;
	float rel_vel_y = vb_y - vo_y;
	float velocityAlongNormal = rel_vel_x * normalX + rel_vel_y * normalY;
	if (velocityAlongNormal > 0) return;
	float E = 0.8f;
	float J = -(1 + E) * velocityAlongNormal / (1 / m_bullet + 1 / m_object);
	float impulse_x = J * normalX;
	float impulse_y = J * normalY;

	float delta_vb_x = impulse_x / m_bullet;
	float delta_vb_y = impulse_y / m_bullet;
	float delta_vo_x = -impulse_x / m_object;
	float delta_vo_y = -impulse_y / m_object;

	bullet.dirX += delta_vb_x / bulletAcceleration;
	bullet.dirY += delta_vb_y / bulletAcceleration;
	e.obVelX += delta_vo_x * 5;
	e.obVelY += delta_vo_y * 5;
}

void handleShotBulletPlayerCollision(shotBullet& bullet)
{
	float bulletCenterX = bullet.x + bullet.width / 2;
	float bulletCenterY = bullet.y + bullet.height / 2;
	float playerCenterX = playerPosX + playerSizeX / 2;
	float playerCenterY = playerPosY + playerSizeY / 2;

	float normalX = bulletCenterX - playerCenterX;
	float normalY = bulletCenterY - playerCenterY;
	float length = sqrt(normalX * normalX + normalY * normalY);
	if (length == 0.0f) return;

	normalX /= length;
	normalY /= length;

	float bulletVelX = bullet.dirX * bulletAcceleration;
	float bulletVelY = bullet.dirY * bulletAcceleration;
	float relativeVelX = bulletVelX - playerVelocityX;
	float relativeVelY = bulletVelY - playerVelocityY;

	float velocityAlongNormal = relativeVelX * normalX + relativeVelY * normalY;
	if (velocityAlongNormal > 0) return;
	float restitution = 0.5f;
	float impulseScalar = -(1 + restitution) * velocityAlongNormal;
	bullet.dirX += (impulseScalar * normalX) / bulletAcceleration;
	bullet.dirY += (impulseScalar * normalY) / bulletAcceleration;
}

void handleShotBulletEnemyCollision(shotBullet& bullet, Enemy& e)
{
	float bulletCenterX = bullet.x + bullet.width / 2;
	float bulletCenterY = bullet.y + bullet.height / 2;
	float objectCenterX = e.x + obSizeWidth / 2;
	float objectCenterY = e.y + obSizeHeight / 2;

	float normalX = bulletCenterX - objectCenterX;
	float normalY = bulletCenterY - objectCenterY;
	float distance = sqrt(normalX * normalX + normalY * normalY);
	if (distance == 0.0f) return;

	normalX /= distance;
	normalY /= distance;

	float vb_x = bullet.dirX * bulletAcceleration;
	float vb_y = bullet.dirY * bulletAcceleration;
	float vo_x = e.obVelX;
	float vo_y = e.obVelY;

	float rel_vel_x = vb_x - vo_x;
	float rel_vel_y = vb_y - vo_y;
	float velocityAlongNormal = rel_vel_x * normalX + rel_vel_y * normalY;
	if (velocityAlongNormal > 0) return;
	float E = 0.8f;
	float J = -(1 + E) * velocityAlongNormal / (1 / m_bullet + 1 / m_object);
	float impulse_x = J * normalX;
	float impulse_y = J * normalY;

	float delta_vb_x = impulse_x / m_bullet;
	float delta_vb_y = impulse_y / m_bullet;
	float delta_vo_x = -impulse_x / m_object;
	float delta_vo_y = -impulse_y / m_object;

	bullet.dirX += delta_vb_x / bulletAcceleration;
	bullet.dirY += delta_vb_y / bulletAcceleration;
	e.obVelX += delta_vo_x / 1;
	e.obVelY += delta_vo_y / 1;
}

void handleShotBulletBulletCollision(Bullet& a, shotBullet& b)
{
	float bulletCenterX = a.x + a.width / 2;
	float bulletCenterY = a.y + a.height / 2;
	float otherbulletCenterX = b.x + b.width / 2;
	float otherbulletCenterY = b.y + b.height / 2;
	float normalX = otherbulletCenterX - bulletCenterX;
	float normalY = otherbulletCenterY - bulletCenterY;
	float length = sqrt(normalX * normalX + normalY * normalY);
	if (length == 0.0f) return;
	normalX /= length;
	normalY /= length;

	float relativeVelocityX = b.dirX - a.dirX;
	float relativeVelocityY = b.dirY - a.dirY;

	float velocityAlongNormal = relativeVelocityX * normalX + relativeVelocityY * normalY;
	if (velocityAlongNormal > 0) return;

	float restitution = 0.8f;
	float impulseScalar = -(1 + restitution) * velocityAlongNormal;
	impulseScalar /= 2;

	a.dirX -= impulseScalar * normalX;
	a.dirY -= impulseScalar * normalY;
	b.dirX += impulseScalar * normalX;
	b.dirY += impulseScalar * normalY;
}

bool CheckBulletCollision(const Bullet& a, const Bullet& b)
{
	return (a.x < b.x + b.width &&
		a.x + a.width > b.x &&
		a.y < b.y + b.height &&
		a.y + a.height > b.y);
}

bool CheckPlayerCollision(const Bullet& b)
{
	return (playerPosX < b.x + b.width &&
		playerPosX + playerSizeX > b.x &&
		playerPosY < b.y + b.height &&
		playerPosY + playerSizeY > b.y);
}

bool CheckobjectBulletCollision(Bullet& b, Enemy& e)
{
	bool collision = (e.x < b.x + b.width &&
		e.x + e.width > b.x &&
		e.y < b.y + b.height &&
		e.y + e.height > b.y);
	if (collision  && !e.enemyGothit &&!b.alreadyHitEnemy){
		e.enemyGothit = true;
		b.alreadyHitEnemy = true;
		e.obAcceleration = 0;
		e.hitStartTime = SDL_GetTicks();
	}
	return collision;
}

bool CheckShotBulletEnemyCollision(shotBullet& b, Enemy& e)
{
	bool collision = (e.x < b.x + b.width &&
		e.x + e.width > b.x &&
		e.y < b.y + b.height &&
		e.y + e.height > b.y);
	if (collision && !e.enemyGothit && !b.alreadyHitEnemy)
	{
		e.enemyGothit = true;
		b.alreadyHitEnemy = true;
		e.obAcceleration = 0;
		e.hitStartTime = SDL_GetTicks();
	}
	return collision;
}

bool CheckShotBulletPlayerCollision(const shotBullet& b)
{
	return (playerPosX < b.x + b.width &&
		playerPosX + playerSizeX > b.x &&
		playerPosY < b.y + b.height &&
		playerPosY + playerSizeY > b.y);
}

bool CheckEnemyTouchPlayer(Enemy& e)
{
	bool collision =  (playerPosX < e.x + e.width &&
		playerPosX + playerSizeX > e.x &&
		playerPosY < e.y + e.height &&
		playerPosY + playerSizeY > e.y);
	if (collision)
	{
		isGameOver = true;
	}
	return collision;
}

bool CheckShotBulletBulletCollision(Bullet& a, shotBullet& b)
{
	return (a.x < b.x + b.width &&
		a.x + a.width > b.x &&
		a.y < b.y + b.height &&
		a.y + a.height > b.y);
}

void gunFollowMouse(SDL_Renderer* render, float gunX, float gunY, float mouseX, float mouseY)
{
	float angle = atan2(mouseY - (gunY + gunSizeY / 2),
		mouseX - (gunX + gunSizeX / 6));
	const float halfWidth = gunSizeX / 2;
	const float halfHeight = gunSizeY / 2;
	const SDL_FPoint center =
	{
		gunX + 22,
		gunY + halfHeight
	};
	const float cos_angle = cos(angle);
	const float sin_angle = sin(angle);
	SDL_Vertex vertices[4] = {
		{{center.x + (-halfWidth * cos_angle - (-halfHeight) * sin_angle),
			center.y + (-halfWidth * sin_angle + (-halfHeight) * cos_angle)},
		{93, 93, 93, 255}, {0} },

	{{center.x + (halfWidth * cos_angle - (-halfHeight) * sin_angle),
			center.y + (halfWidth * sin_angle + (-halfHeight) * cos_angle)},
		{93, 93, 93, 255}, {0} },

		{{center.x + (halfWidth * cos_angle - halfHeight * sin_angle),
			center.y + (halfWidth * sin_angle + halfHeight * cos_angle)},
		{93, 93, 93, 255}, {0} },

		{{center.x + (-halfWidth * cos_angle - halfHeight * sin_angle),
			center.y + (-halfWidth * sin_angle + halfHeight * cos_angle)},
		{93, 93, 93, 255}, {0} },
	};
	const int indices[6] = { 0,1,2,0,2,3 };
	SDL_RenderGeometry(render, nullptr, vertices, 4, indices, 6);
}

int runGame()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cerr << "sdl could not be initialize" << SDL_GetError() << std::endl;
		return -1;
	}

	if (TTF_Init() == -1)
	{
		std::cerr << "sdl ttf aint installed properly" << std::endl;
		SDL_Quit();
		return -1;
	}

	SDL_Window* window = SDL_CreateWindow("ue6", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (!window)
	{
		std::cout << "cant make the display" << SDL_GetError() << std::endl;
		SDL_Quit();
		TTF_Quit();
		return -1;
	}

	SDL_Renderer* render = SDL_CreateRenderer(window, 0);
	if (!render)
	{
		std::cerr << "cant make renderer" << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		TTF_Quit();
		return -1;
	}

	TTF_Font* font = TTF_OpenFont("K:\\c++project\\font\\LittleBoy-Regular.otf", 24);
	if (!font)
	{
		std::cerr << "cant load the font with the path" << std::endl;
		SDL_DestroyRenderer(render);
		SDL_DestroyWindow(window);
		SDL_Quit();
		TTF_Quit();
		return -1;
	}
	TTF_Font* Manuefont = TTF_OpenFont("K:\\c++project\\font\\LittleBoy-Regular.otf", 50);
	if (!Manuefont)
	{
		std::cerr << "cant load the font with the path" << std::endl;
		SDL_DestroyRenderer(render);
		SDL_DestroyWindow(window);
		SDL_Quit();
		TTF_Quit();
		return -1;
	}

	bool mainGameLoop = true;
	SDL_Event e;
	const bool* keyboardState = SDL_GetKeyboardState(NULL);

	Uint64 frameStartTime = SDL_GetTicks();
	int frameCount = 0;
	int fps = 0;

	const float targetFPS = 60.0f;
	const Uint64 targetFrameTime = 1000 / targetFPS;
	const float fixedTimeStep = 1.0f / 120.0f;

	float accumulatedTime = 0.0f;

	float prevPlayerPosx = playerPosX;
	float prevPlayerPosY = playerPosY;

	std::random_device rd;
	std::default_random_engine engine(rd());

	float objectWidth = 50.0f;
	float objectHeight = 45.0f;
	std::uniform_real_distribution<float> distX(0.0f, 1280.0f - objectWidth);
	std::uniform_real_distribution<float> distY(0.0f, 900.0f - objectHeight);

	while (mainGameLoop)
	{
		Uint64 currentTime = SDL_GetTicks();
		float time_deltaTime = (currentTime - frameStartTime) / 1000.0f;
		frameStartTime = currentTime;

		if (!isGameOver)
		{
			accumulatedTime += time_deltaTime;
		}

		static Uint64 lastFpsTime = SDL_GetTicks();
		frameCount++;
		if (currentTime - lastFpsTime >= 1000)
		{
			fps = frameCount;
			frameCount = 0;
			lastFpsTime = currentTime;
		}
		deltaTime = time_deltaTime;
		Uint64 frameEnd = SDL_GetTicks();
		Uint64 frameTime = frameEnd - frameStartTime;
		if (frameTime < targetFrameTime)
		{
			SDL_Delay(targetFrameTime - frameTime);
		}
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_EVENT_QUIT)
			{
				mainGameLoop = false;
			}
			else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
			{
				if (e.button.button == SDL_BUTTON_LEFT)
				{
					float mouseX, mouseY;
					SDL_GetMouseState(&mouseX, &mouseY);
					SpawnBullet(mouseX, mouseY);
				}
				if (e.button.button == SDL_BUTTON_RIGHT)
				{
					float mouseX, mouseY;
					SDL_GetMouseState(&mouseX, &mouseY);
					SpawnShotBullet(mouseX, mouseY);
				}
			}
			else if (e.type == SDL_EVENT_KEY_DOWN && e.key.repeat == 0)
			{
				if (e.key.scancode == SDL_SCANCODE_ESCAPE)
				{
					pauseButton = !pauseButton;
				}
			}
		}
		playerControl();
		gunPosX = playerPosX;
		gunPosY = playerPosY;

		while (accumulatedTime >= fixedTimeStep)
		{
			float bounceBulletForce = 5.0f;
			//bullet collsision
			for (auto& bullet : bullets)
			{
				bullet.Update(fixedTimeStep);
				for (auto& shotb : shotbullets)
				{
					if (CheckShotBulletBulletCollision(bullet, shotb))
					{
						handleShotBulletBulletCollision(bullet, shotb);
					}
				}
				if (bullet.y < 0)
				{
					bullet.y = 0;
					bullet.dirY = -bullet.dirY / bounceBulletForce;
				}
				if (bullet.y > SCREEN_HEIGHT - bulletSizeHeight)
				{
					bullet.y = SCREEN_HEIGHT - bulletSizeHeight;
					bullet.dirY = -bullet.dirY / bounceBulletForce;
				}

				if (bullet.x < 0)
				{
					bullet.x = 0;
					bullet.dirX = -bullet.dirX / bounceBulletForce;
				}
				if (bullet.x > SCREEN_WIDTH - bulletSizeWidth)
				{
					bullet.x = SCREEN_WIDTH - bulletSizeWidth;
					bullet.dirX = -bullet.dirX / bounceBulletForce;
				}
			}
			for (size_t i = 0; i < bullets.size(); i++)
			{
				for (size_t j = i + 1; j < bullets.size(); j++)
				{
					if (CheckBulletCollision(bullets[i], bullets[j]))
					{
						handleBulletCollision(bullets[i], bullets[j]);
					}
				}
			}
			//collision between player and bullets and object and bullet
			for (auto& bullet : bullets)
			{
				if (CheckPlayerCollision(bullet))
				{
					handleBulletPlayerCollision(bullet);
				}
				for (auto& e : enemy)
				if (CheckobjectBulletCollision(bullet, e))
				{
					handleBulletObjectCollision(bullet, e);
				}
			}
			for (auto& sb : shotbullets)
			{
				sb.Update(fixedTimeStep);
				if (CheckShotBulletPlayerCollision(sb))
				{
					handleShotBulletPlayerCollision(sb);
				}
				for (auto& e : enemy)
					if (CheckShotBulletEnemyCollision(sb, e))
					{
						handleShotBulletEnemyCollision(sb, e);
					}
				if (sb.y < 0)
				{
					sb.y = 0;
					sb.dirY = -sb.dirY / bounceBulletForce;
				}
				if (sb.y > SCREEN_HEIGHT - shotbulletSizeHeight)
				{
					sb.y = SCREEN_HEIGHT - shotbulletSizeHeight;
					sb.dirY = -sb.dirY / bounceBulletForce;
				}

				if (sb.x < 0)
				{
					sb.x = 0;
					sb.dirX = -sb.dirX / bounceBulletForce;
				}
				if (sb.x > SCREEN_WIDTH - shotbulletSizeWidth)
				{
					sb.x = SCREEN_WIDTH - shotbulletSizeWidth;
					sb.dirX = -sb.dirX / bounceBulletForce;
				}
			}
			for (auto& e : enemy)
			{
				e.Update(fixedTimeStep, playerPosX, playerPosY);
				recoverEnemyMovement(fixedTimeStep, e);
				if (CheckEnemyTouchPlayer)
				{
					//isGameOver = true;
				}
				//prevent object going though window
				if (e.x < 0)
				{
					e.x = 0;
					e.obVelX = -e.obVelX / 5.0f;
				}
				if (e.x > SCREEN_WIDTH - obSizeWidth)
				{
					e.x = SCREEN_WIDTH - obSizeWidth;
					e.obVelX = -e.obVelX / 5.0f;
				}
				if (e.y < 0)
				{
					e.y = 0;
					e.obVelY = -e.obVelY / 5.0f;
				}
				if (e.y > SCREEN_HEIGHT - obSizeHeight)
				{
					e.y = SCREEN_HEIGHT - obSizeHeight;
					e.obVelY = -e.obVelY / 5.0f;
				}

				if (e.obVelX > 0)
				{
					e.obVelX -= obfriction * fixedTimeStep;
					if (e.obVelX < 0) e.obVelX = 0;
				}
				else if (e.obVelX < 0)
				{
					e.obVelX += obfriction * fixedTimeStep;
					if (e.obVelX > 0) e.obVelX = 0;
				}

				if (e.obVelY > 0)
				{
					e.obVelY -= obfriction * fixedTimeStep;
					if (e.obVelY < 0) e.obVelY = 0;
				}
				else if (e.obVelY < 0)
				{
					e.obVelY += obfriction * fixedTimeStep;
					if (e.obVelY > 0) e.obVelY = 0;
				}
			}
			bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) {return b.isExpired;}),
				bullets.end());
			shotbullets.erase(std::remove_if(shotbullets.begin(), shotbullets.end(), [](const shotBullet& b) {return b.isExpired;}),
				shotbullets.end());
			enemy.erase(std::remove_if(enemy.begin(), enemy.end(), [](const Enemy& e) {return e.enemyExpired;}),
				enemy.end());
			accumulatedTime -= fixedTimeStep;
		}
		//player collsision
		if (playerPosX < 0) playerPosX = 0;
		if (playerPosX > SCREEN_WIDTH - playerSizeX) playerPosX = SCREEN_WIDTH - playerSizeX;

		if (playerPosY < 0) playerPosY = 0;
		if (playerPosY > SCREEN_HEIGHT - playerSizeY) playerPosY = SCREEN_HEIGHT - playerSizeY;
		if (currentTime - lastSpawnTime >= spawnEnemyInterval)
		{
			SpawnEnemy(&distX, &distY, &engine);
			lastSpawnTime = currentTime;
		}
		float dx = playerPosX - prevPlayerPosx;
		float dy = playerPosY - prevPlayerPosY;
		float movementSpeed = 0.0f;
		if (deltaTime > 0)
		{
			movementSpeed = std::sqrt(playerVelocityX * playerVelocityX + playerVelocityY * playerVelocityY);
		}

		//render screen
		SDL_SetRenderDrawColor(render, 100, 30, 0, 255);
		SDL_RenderClear(render);

		//render player
		SDL_FRect player = { playerPosX, playerPosY, playerSizeX, playerSizeY };
		SDL_SetRenderDrawColor(render, playerRed, playerGreen, playerBlue, playerAplha);
		SDL_RenderFillRect(render, &player);

		float gunmouseX, gunmouseY;
		SDL_GetMouseState(&gunmouseX, &gunmouseY);
		gunFollowMouse(render, gunPosX, gunPosY, gunmouseX, gunmouseY);
		//render bullet
		SDL_SetRenderDrawColor(render, bulletRed, bulletGreen, bulletBlue, bulletAplha);
		for (auto& bullet : bullets)
		{
			SDL_FRect bulletRender = { bullet.x, bullet.y, bullet.width, bullet.height };
			SDL_RenderFillRect(render, &bulletRender);
		}

		//render shot gun bullet
		SDL_SetRenderDrawColor(render, shotbulletRed, shotbulletGreen, shotbulletBlue, shotbulletAplha);
		for (auto& bullet : shotbullets)
		{
			SDL_FRect shotbulletRender = { bullet.x, bullet.y, bullet.width, bullet.height };
			SDL_RenderFillRect(render, &shotbulletRender);
		}

		//render enemy
		SDL_SetRenderDrawColor(render, 0, 255, 150, 255);
		for (auto& e : enemy)
		{
			SDL_FRect object = { e.x, e.y, e.width, e.height };
			SDL_RenderRect(render, &object);
		}

		SDL_Color textColor = { 0, 0 , 255, 255 };
		std::string fpsText = "FPS: " + std::to_string(fps);
		SDL_Surface* textSurface = TTF_RenderText_Solid(font, fpsText.c_str(), 0, textColor);
		if (textSurface != nullptr)
		{
			SDL_Texture* textTexture = SDL_CreateTextureFromSurface(render, textSurface);
			if (textTexture != nullptr)
			{
				SDL_FRect textRect = { 10.0f, 10.0f, static_cast<float>(textSurface->w), static_cast<float>(textSurface->h) };
				SDL_RenderTexture(render, textTexture, nullptr, &textRect);
				SDL_DestroyTexture(textTexture);
			}
			SDL_DestroySurface(textSurface);
		}
		else
		{
			std::cerr << "fail to render fps text" << std::endl;
		}

		//pause manue
		if (pauseButton)
		{
			SDL_Color manueColor = { 120, 120 ,120 ,255 };
			SDL_SetRenderDrawColor(render, 120, 120 ,120 ,255);
			SDL_RenderClear(render);
			std::string manueText = "pause";
			SDL_Surface* manueSurface = TTF_RenderText_Solid(Manuefont, manueText.c_str(), 0, manueColor);
			SDL_Texture* manueTexture = SDL_CreateTextureFromSurface(render, manueSurface);
			SDL_FRect manueRect = { 500.0f, 450.0f, static_cast<float>(manueSurface->w), static_cast<float>(manueSurface->h) };
			SDL_RenderTexture(render, manueTexture, nullptr, &manueRect);
			SDL_DestroyTexture(manueTexture);
			SDL_DestroySurface(manueSurface);
		}

		SDL_Color playerSpeedColor = { 0, 255 ,0 ,255 };
		std::string speedText = "speed: " + std::to_string(movementSpeed) + " px/s";
		SDL_Surface* speedSurface = TTF_RenderText_Solid(font, speedText.c_str(), 0, playerSpeedColor);
		if (speedSurface != nullptr)
		{
			SDL_Texture* speedTexture = SDL_CreateTextureFromSurface(render, speedSurface);
			if (speedTexture != nullptr)
			{
				SDL_FRect speedRect = { 500.0f, 10.0f, static_cast<float>(speedSurface->w), static_cast<float>(speedSurface->h) };
				SDL_RenderTexture(render, speedTexture, nullptr, &speedRect);
				SDL_DestroyTexture(speedTexture);
			}
			SDL_DestroySurface(speedSurface);
		}
		else
		{
			std::cerr << "fail to render playerspeed text" << std::endl;
		}
		SDL_RenderPresent(render);
	}
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

int main(int argc, char* argv[])
{
	return runGame();
}