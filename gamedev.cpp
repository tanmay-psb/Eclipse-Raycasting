#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <vector>

using namespace sf;

// --- CONFIGURATION ---
const int WIDTH = 800;
const int HEIGHT = 600;
const float PI = 3.14159265f;
const float FOV = PI / 3.0f;
const int MAP_SIZE = 16;

// 1=Blue Neon, 2=Orange Industrial, 3=Emerald Glass, 4=Sliding Door
int worldMap[MAP_SIZE][MAP_SIZE] =
{
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,4,1,0,0,0,0,0,0,1,1,0,1},
    {1,0,1,0,0,1,2,2,2,0,0,0,0,1,0,1},
    {1,0,0,0,0,0,2,0,2,0,0,0,0,0,0,1},
    {1,0,1,1,4,1,2,0,2,2,2,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,4,0,0,0,0,0,0,1},
    {1,0,1,0,0,0,3,0,3,0,0,0,0,1,0,1},
    {1,0,1,0,0,0,3,0,3,0,0,0,0,1,0,1},
    {1,0,1,0,0,0,2,4,2,0,0,0,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,2,0,0,0,0,0,2,0,0,0,0,1},
    {1,0,1,0,2,0,0,0,0,0,2,0,1,1,0,1},
    {1,4,1,0,2,2,4,2,2,2,2,0,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

struct Door {
    int x, y;
    bool opening = false;
    float offset = 0.0f; 
};

struct Particle {
    float x, y, vx, vy, life;
    Color color;
};

std::vector<Door> doors;
std::vector<Particle> particles;

void spawnEnemy(float &x, float &y) {
    while(true) {
        int cx = rand() % MAP_SIZE, cy = rand() % MAP_SIZE;
        if(worldMap[cy][cx] == 0) { x = cx + 0.5f; y = cy + 0.5f; return; }
    }
}

void spawnBurst(float x, float y, Color c, int count) {
    for(int i=0; i<count; i++) {
        float angle = (rand()%360) * PI / 180.0f;
        float speed = (rand()%100)/20.0f + 1.0f;
        particles.push_back({x, y, cos(angle)*speed, sin(angle)*speed, 0.5f + (rand()%100)/200.0f, c});
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0)));
    RenderWindow window(VideoMode(WIDTH, HEIGHT), "Neon Lab - Raycasting Engine");
    window.setFramerateLimit(60);

    for(int y=0; y<MAP_SIZE; y++)
        for(int x=0; x<MAP_SIZE; x++)
            if(worldMap[y][x] == 4) doors.push_back({x, y});

    float px = 1.5f, py = 1.5f, angle = 0.5f;
    float targetX, targetY; bool targetAlive = true; spawnEnemy(targetX, targetY);
    float shootTimer = 0, gameTime = 0, score = 0;
    Font font; font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    while(window.isOpen()) {
        float delta = 1.0f / 60.0f; gameTime += delta; shootTimer += delta;
        Event e;
        while(window.pollEvent(e)) {
            if(e.type == Event::Closed) window.close();
            if(e.type == Event::KeyPressed && e.key.code == Keyboard::E) {
                int tx = (int)(px + cos(angle)*1.2f), ty = (int)(py + sin(angle)*1.2f);
                for(auto &d : doors) if(d.x == tx && d.y == ty) d.opening = !d.opening;
            }
        }

        for(auto &d : doors) {
            if(d.opening && d.offset < 1.0f) d.offset += 2.0f * delta;
            else if(!d.opening && d.offset > 0.0f) d.offset -= 2.0f * delta;
            if(d.offset < 0) d.offset = 0; if(d.offset > 1) d.offset = 1;
        }
        for(int i=0; i<(int)particles.size(); i++) {
            particles[i].x += particles[i].vx * delta;
            particles[i].y += particles[i].vy * delta;
            particles[i].life -= delta;
            if(particles[i].life <= 0) { particles.erase(particles.begin()+i); i--; }
        }

        float rSpd = 3.0f * delta, spd = 3.5f * delta;
        if(Keyboard::isKeyPressed(Keyboard::A)) angle -= rSpd;
        if(Keyboard::isKeyPressed(Keyboard::D)) angle += rSpd;

        auto canMove = [&](float nx, float ny) {
            int mx = (int)nx, my = (int)ny; if(worldMap[my][mx] == 0) return true;
            if(worldMap[my][mx] == 4) { for(auto &d : doors) if(d.x == mx && d.y == my) return d.offset > 0.8f; }
            return false;
        };
        if(Keyboard::isKeyPressed(Keyboard::W)) {
            if(canMove(px + cos(angle)*spd, py)) px += cos(angle)*spd;
            if(canMove(px, py + sin(angle)*spd)) py += sin(angle)*spd;
        }
        if(Keyboard::isKeyPressed(Keyboard::S)) {
            if(canMove(px - cos(angle)*spd, py)) px -= cos(angle)*spd;
            if(canMove(px, py - sin(angle)*spd)) py -= sin(angle)*spd;
        }

        if(Mouse::isButtonPressed(Mouse::Left) && shootTimer > 0.2f) {
            shootTimer = 0;
            float rX = px, rY = py, rDx = cos(angle), rDy = sin(angle);
            for(float d=0; d<15; d+=0.02f) {
                rX += rDx*0.02f; rY += rDy*0.02f;
                if(targetAlive && (pow(rX-targetX,2)+pow(rY-targetY,2)) < 0.15f) {
                    spawnBurst(targetX, targetY, Color::Magenta, 30);
                    score++; spawnEnemy(targetX, targetY); break;
                }
                if(worldMap[(int)rY][(int)rX] > 0) {
                    int type = worldMap[(int)rY][(int)rX];
                    Color c = (type==1)?Color::Cyan:Color::Red;
                    spawnBurst(rX, rY, c, 15); break;
                }
            }
        }

        window.clear(Color(5, 5, 20));
        window.draw(RectangleShape(Vector2f(WIDTH, HEIGHT/2)));

        for(int x = 0; x < WIDTH; x++) {
            float rayA = angle - FOV/2 + FOV*x/(float)WIDTH;
            float rDx = cos(rayA), rDy = sin(rayA), rX = px, rY = py;
            for(float d = 0; d < 20; d += 0.015f) {
                rX += rDx * 0.015f; rY += rDy * 0.015f;
                int mx = (int)rX, my = (int)rY, hitType = worldMap[my][mx];
                if(hitType > 0) {
                    bool skip = false; if(hitType == 4) {
                        float hLoc = (fabs(rDx) > fabs(rDy)) ? rY-my : rX-mx;
                        for(auto &dr : doors) if(dr.x==mx && dr.y==my && hLoc<dr.offset) { skip=true; break; }
                    }
                    if(skip) continue;
                    float dist = d * cos(rayA - angle); int lH = (int)(HEIGHT / (dist + 0.0001f));
                    Color c = (hitType==1)?Color(0,180,255):(hitType==2)?Color(255,120,0):(hitType==4)?Color(100,105,115):Color(0,255,180);
                    float fog = 1.0f / (1.0f + dist*dist*0.06f); RectangleShape wall(Vector2f(1, lH));
                    wall.setPosition((float)x, HEIGHT/2 - lH/2); wall.setFillColor(Color(c.r*fog, c.g*fog, c.b*fog));
                    window.draw(wall); break;
                }
            }
        }

        if(targetAlive) {
            float dxE = targetX-px, dyE = targetY-py, dist = sqrt(dxE*dxE+dyE*dyE), aE = atan2(dyE, dxE)-angle;
            while(aE > PI) aE -= 2*PI; while(aE < -PI) aE += 2*PI;
            if(fabs(aE) < FOV/2) {
                float sz = HEIGHT/(dist+0.1f), sX = (aE+FOV/2)/FOV*WIDTH;
                CircleShape eDot(sz/3.5f); eDot.setOrigin(sz/3.5f, sz/3.5f); eDot.setPosition(sX, HEIGHT/2);
                int p = 160+90*sin(gameTime*8.0f); eDot.setFillColor(Color(p, 50, 255, 200)); window.draw(eDot);
            }
        }
        for(auto &p : particles) {
            float dxP = p.x-px, dyP = p.y-py, dist = sqrt(dxP*dxP+dyP*dyP), aP = atan2(dyP, dxP)-angle;
            while(aP > PI) aP -= 2*PI; while(aP < -PI) aP += 2*PI;
            if(fabs(aP) < FOV/2) {
                float sX = (aP+FOV/2)/FOV*WIDTH; float sz = 15.0f/dist;
                RectangleShape pk(Vector2f(sz, sz)); pk.setPosition(sX, HEIGHT/2);
                pk.setFillColor(Color(p.color.r, p.color.g, p.color.b, (int)(p.life*510))); window.draw(pk);
            }
        }

        int nx = (int)(px + cos(angle)*1.2f), ny = (int)(py + sin(angle)*1.2f);
        if(worldMap[ny][nx] == 4) {
            Text pr("PRESS [E] TO INTERACT", font, 18); pr.setOrigin(pr.getLocalBounds().width/2, 0);
            pr.setPosition(WIDTH/2, HEIGHT-80); pr.setFillColor(Color::Yellow); window.draw(pr);
        }
        float ms = 7.0f; for(int y=0; y<MAP_SIZE; y++) for(int x=0; x<MAP_SIZE; x++) {
            if(worldMap[y][x]>0) { RectangleShape w(Vector2f(ms-1,ms-1)); w.setPosition(WIDTH-MAP_SIZE*ms-25+x*ms, 25+y*ms);
                w.setFillColor(worldMap[y][x]==4?Color::Yellow:Color(80,80,95)); window.draw(w);
            }
        }
        CircleShape pD(3); pD.setFillColor(Color::Green); pD.setPosition(WIDTH-MAP_SIZE*ms-25+px*ms-1.5f, 25+py*ms-1.5f); window.draw(pD);
        std::stringstream ss; ss << "RECORDS: " << (int)score;
        Text txt(ss.str(), font, 24); txt.setPosition(30, 30); txt.setFillColor(Color::Cyan); window.draw(txt);
        window.display();
    }
    return 0;
}
