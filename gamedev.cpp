#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace sf;

const int   WIDTH    = 800;
const int   HEIGHT   = 600;
const float PI       = 3.14159265f;
const float FOV      = PI / 3.0f;
const int   MAP_SIZE = 48;

enum GameState { MENU, PLAYING };

int worldMap[MAP_SIZE][MAP_SIZE];

struct Door     { int x, y; bool opening = false; float offset = 0.0f; };
struct Particle { float x, y, vx, vy, life; Color color; };

std::vector<Door>     doors;
std::vector<Particle> particles;

void generateMap() {
    for (int y = 0; y < MAP_SIZE; y++)
        for (int x = 0; x < MAP_SIZE; x++)
            worldMap[y][x] = 0;

    for (int i = 0; i < MAP_SIZE; i++) {
        worldMap[0][i] = worldMap[MAP_SIZE-1][i] = 1;
        worldMap[i][0] = worldMap[i][MAP_SIZE-1] = 1;
    }
    for (int x = 1; x < MAP_SIZE-1; x++) { worldMap[16][x] = 1; worldMap[32][x] = 1; }
    for (int y = 1; y < MAP_SIZE-1; y++) { worldMap[y][16] = 2; worldMap[y][32] = 3; }
    worldMap[16][16]=1; worldMap[16][32]=1; worldMap[32][16]=1; worldMap[32][32]=1;
    for (int dx : {8,24,40}) { worldMap[16][dx]=4; worldMap[32][dx]=4; }
    for (int dy : {8,24,40}) { worldMap[dy][16]=4; worldMap[dy][32]=4; }

    int py[]={5,5,11,11,5,5,11,11,5,5,11,11,20,20,28,28,20,20,28,28,20,20,28,28,36,36,44,44,36,36,44,44,36,36,44,44};
    int px[]={5,11,5,11,20,26,20,26,35,43,35,43,5,11,5,11,20,26,20,26,35,43,35,43,5,11,5,11,20,26,20,26,35,43,35,43};
    int pt[]={1,1,1,1,2,2,2,2,3,3,3,3,2,2,2,2,3,3,3,3,1,1,1,1,3,3,3,3,1,1,1,1,2,2,2,2};
    for (int i=0;i<36;i++) worldMap[py[i]][px[i]]=pt[i];

    doors.clear();
    for (int y=0;y<MAP_SIZE;y++)
        for (int x=0;x<MAP_SIZE;x++)
            if(worldMap[y][x]==4) doors.push_back({x,y});
}

void spawnEnemy(float& ex, float& ey) {
    while(true){int cx=rand()%MAP_SIZE,cy=rand()%MAP_SIZE;if(worldMap[cy][cx]==0){ex=cx+0.5f;ey=cy+0.5f;return;}}
}
void spawnBurst(float x,float y,Color c,int count){
    for(int i=0;i<count;i++){float a=(rand()%360)*PI/180.f;float s=(rand()%60)/25.f+0.3f;particles.push_back({x,y,cosf(a)*s,sinf(a)*s,0.18f+(rand()%20)/100.f,c});}
}
bool doorPassable(int mx,int my){for(auto& d:doors)if(d.x==mx&&d.y==my)return d.offset>0.8f;return false;}
float getWallDist(float px,float py,float ra){
    float dx=cosf(ra),dy=sinf(ra),rx=px,ry=py;
    for(float d=0.02f;d<(float)MAP_SIZE;d+=0.02f){rx+=dx*0.02f;ry+=dy*0.02f;int mx=(int)rx,my=(int)ry;
    if(mx<0||mx>=MAP_SIZE||my<0||my>=MAP_SIZE)return d;int h=worldMap[my][mx];if(h>0){if(h==4&&doorPassable(mx,my))continue;return d;}}return(float)MAP_SIZE;}

Color getBrickColor(float u,float v,float dist,int hitType){
    const float ROWS=3.5f,COLS=1.8f,MORTAR=0.07f;
    int bRow=(int)(v*ROWS);float bv=fmodf(v*ROWS,1.f),bu=fmodf(u*COLS+(bRow&1)*0.5f,1.f);
    bool grout=(bv<MORTAR||bu<MORTAR);
    float noise=fmodf((float)bRow*5.7f+floorf(u*COLS+(bRow&1)*0.5f)*11.3f,1.f);int n=(int)(noise*28);
    float fog=std::max(0.04f,1.f-dist*0.048f);Color c;
    switch(hitType){
        case 1:c=grout?Color(68,60,56):Color(168+n,50+n/5,32);break;
        case 2:c=grout?Color(50,42,32):Color(136+n,72+n/4,26);break;
        case 3:c=grout?Color(16,52,62):Color(26+n/3,106+n,126+n/2);break;
        default:{int pv=82+(int)(fmodf(u*3.f,1.f)*38);c=Color(pv,pv+3,pv+8);break;}
    }
    return Color((sf::Uint8)std::min(255.f,c.r*fog),(sf::Uint8)std::min(255.f,c.g*fog),(sf::Uint8)std::min(255.f,c.b*fog));
}

// ── Draw a menu button, returns true if mouse is hovering ────────
bool drawButton(RenderWindow& win, Font& font, float x, float y, float w, float h,
                const std::string& label, Vector2f mousePos, Color baseColor) {
    FloatRect bounds(x, y, w, h);
    bool hover = bounds.contains(mousePos);

    RectangleShape btn({w, h});
    btn.setPosition(x, y);
    btn.setFillColor(hover ? Color(baseColor.r+40, baseColor.g+40, baseColor.b+40, 220)
                           : Color(baseColor.r,     baseColor.g,     baseColor.b,     180));
    btn.setOutlineColor(Color(255,255,255,hover?200:80));
    btn.setOutlineThickness(1.5f);

    // Subtle glow on hover
    if (hover) {
        RectangleShape glow({w+10.f, h+10.f});
        glow.setOrigin(5.f, 5.f); glow.setPosition(x, y);
        glow.setFillColor(Color(baseColor.r, baseColor.g, baseColor.b, 30));
        win.draw(glow);
    }
    win.draw(btn);

    Text txt(label, font, 18);
    txt.setOrigin(txt.getLocalBounds().width*0.5f, txt.getLocalBounds().height*0.5f);
    txt.setPosition(x + w*0.5f, y + h*0.5f - 2.f);
    txt.setFillColor(Color(220, 220, 220));
    win.draw(txt);

    return hover;
}

int main() {
    srand((unsigned)time(nullptr));
    generateMap();

    bool isFullscreen = false;
    GameState state   = MENU;

    // Lambda to (re)create the window with correct mode
    RenderWindow window;
    auto applyWindowMode = [&]() {
        if (isFullscreen)
            window.create(VideoMode::getDesktopMode(), "Eclipse - Raycasting Engine", Style::Fullscreen);
        else
            window.create(VideoMode(WIDTH, HEIGHT), "Eclipse - Raycasting Engine", Style::Default | Style::Close);
        window.setFramerateLimit(60);
        // Keep a fixed 800x600 game view regardless of actual window size
        View v(FloatRect(0,0,(float)WIDTH,(float)HEIGHT));
        window.setView(v);
        if (state == PLAYING) { window.setMouseCursorVisible(false); window.setMouseCursorGrabbed(true); }
        else                  { window.setMouseCursorVisible(true);  window.setMouseCursorGrabbed(false); }
    };
    applyWindowMode();

    const Vector2i WIN_CENTER(WIDTH/2, HEIGHT/2);

    float playerX=2.5f,playerY=2.5f,viewAngle=0.f;
    float targetX,targetY; bool targetAlive=true;
    spawnEnemy(targetX,targetY);
    float shootTimer=0.f,gameTime=0.f,score=0.f;
    Font font; font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    const float MS=3.5f, MMAP_X=WIDTH-MAP_SIZE*MS-8.f, MMAP_Y=8.f;
    VertexArray wallVerts(Quads);
    float zBuffer[WIDTH];

    while (window.isOpen()) {
        const float delta=1.f/60.f;
        gameTime+=delta; shootTimer+=delta;

        // Get mouse position mapped to our fixed 800x600 view
        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

        Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type==Event::Closed) window.close();

            if (ev.type==Event::KeyPressed && ev.key.code==Keyboard::Escape) {
                if (state==PLAYING) {
                    // Pause → go to menu
                    state=MENU;
                    window.setMouseCursorVisible(true);
                    window.setMouseCursorGrabbed(false);
                } else {
                    window.close();
                }
            }

            // Menu button clicks
            if (state==MENU && ev.type==Event::MouseButtonPressed && ev.mouseButton.button==Mouse::Left) {
                // START button: y=250, height=55
                if (mousePos.y>=250&&mousePos.y<=305&&mousePos.x>=250&&mousePos.x<=550) {
                    state=PLAYING;
                    window.setMouseCursorVisible(false);
                    window.setMouseCursorGrabbed(true);
                    Mouse::setPosition(WIN_CENTER, window);
                }
                // FULLSCREEN button: y=330
                if (mousePos.y>=330&&mousePos.y<=385&&mousePos.x>=250&&mousePos.x<=550) {
                    isFullscreen=!isFullscreen;
                    applyWindowMode();
                }
                // QUIT button: y=410
                if (mousePos.y>=410&&mousePos.y<=465&&mousePos.x>=250&&mousePos.x<=550) {
                    window.close();
                }
            }

            // In-game door interaction
            if (state==PLAYING && ev.type==Event::KeyPressed && ev.key.code==Keyboard::E) {
                int tx=(int)(playerX+cosf(viewAngle)*1.2f),ty=(int)(playerY+sinf(viewAngle)*1.2f);
                for(auto& d:doors)if(d.x==tx&&d.y==ty)d.opening=!d.opening;
            }
        }

        // ── MENU STATE ──────────────────────────────────────────
        if (state==MENU) {
            window.clear(Color(4,4,16));

            // Animated background grid
            for (int gx=0;gx<WIDTH;gx+=40) {
                RectangleShape line({1.f,(float)HEIGHT});
                line.setPosition((float)gx,0);
                line.setFillColor(Color(0,80,120,30+(int)(20*sinf(gameTime+gx*0.01f))));
                window.draw(line);
            }
            for (int gy=0;gy<HEIGHT;gy+=40) {
                RectangleShape line({(float)WIDTH,1.f});
                line.setPosition(0,(float)gy);
                line.setFillColor(Color(0,80,120,30+(int)(20*sinf(gameTime+gy*0.01f))));
                window.draw(line);
            }

            // Panel background
            RectangleShape panel({320.f,280.f});
            panel.setOrigin(160.f,140.f); panel.setPosition(WIDTH*0.5f,HEIGHT*0.5f+20.f);
            panel.setFillColor(Color(10,10,30,180));
            panel.setOutlineColor(Color(0,160,200,120)); panel.setOutlineThickness(1.5f);
            window.draw(panel);

            // Title
            Text title("ECLIPSE",font,72);
            title.setOrigin(title.getLocalBounds().width*0.5f,0);
            title.setPosition(WIDTH*0.5f,80.f);
            int tw=180+(int)(60*sinf(gameTime*1.5f));
            title.setFillColor(Color(tw,230,255));
            window.draw(title);

            Text sub("RAYCASTING ENGINE",font,18);
            sub.setOrigin(sub.getLocalBounds().width*0.5f,0);
            sub.setPosition(WIDTH*0.5f,165.f);
            sub.setFillColor(Color(100,180,200,200));
            window.draw(sub);

            // Separator line
            RectangleShape sep({240.f,1.f}); sep.setOrigin(120.f,0);
            sep.setPosition(WIDTH*0.5f,205.f); sep.setFillColor(Color(0,160,200,150));
            window.draw(sep);

            // Controls hint
            Text hint("WASD  Move     Mouse  Look\n E  Interact     ESC  Menu",font,13);
            hint.setOrigin(hint.getLocalBounds().width*0.5f,0);
            hint.setPosition(WIDTH*0.5f,218.f);
            hint.setFillColor(Color(120,140,150,180));
            window.draw(hint);

            // Buttons
            std::string fsLabel = isFullscreen ? "FULLSCREEN  :  ON" : "FULLSCREEN  :  OFF";
            drawButton(window,font,250,270,300,50,"  PLAY",         mousePos,Color(20,90,40));
            drawButton(window,font,250,338,300,50,fsLabel,          mousePos,Color(20,50,100));
            drawButton(window,font,250,406,300,50,"  QUIT",         mousePos,Color(100,20,20));

            // Version tag
            Text ver("v2.0",font,12); ver.setPosition(WIDTH-50.f,HEIGHT-25.f);
            ver.setFillColor(Color(60,60,80)); window.draw(ver);

            window.display();
            continue; // skip game loop below
        }

        // ── PLAYING STATE ────────────────────────────────────────

        // Door animation
        for(auto& d:doors){
            if(d.opening&&d.offset<1.f)d.offset=std::min(1.f,d.offset+2.f*delta);
            if(!d.opening&&d.offset>0.f)d.offset=std::max(0.f,d.offset-2.f*delta);
        }
        // Particle update
        for(int i=0;i<(int)particles.size();){auto& p=particles[i];p.x+=p.vx*delta;p.y+=p.vy*delta;p.life-=delta;if(p.life<=0.f)particles.erase(particles.begin()+i);else ++i;}

        // Mouse look
        { Vector2i mp=Mouse::getPosition(window); viewAngle+=(mp.x-WIN_CENTER.x)*0.0025f; Mouse::setPosition(WIN_CENTER,window); }

        // Movement
        const float spd=3.5f*delta;
        auto canMove=[&](float nx,float ny)->bool{
            int mx=(int)nx,my=(int)ny;if(mx<0||mx>=MAP_SIZE||my<0||my>=MAP_SIZE)return false;
            int h=worldMap[my][mx];if(h==0)return true;if(h==4)return doorPassable(mx,my);return false;};
        if(Keyboard::isKeyPressed(Keyboard::W)){if(canMove(playerX+cosf(viewAngle)*spd,playerY))playerX+=cosf(viewAngle)*spd;if(canMove(playerX,playerY+sinf(viewAngle)*spd))playerY+=sinf(viewAngle)*spd;}
        if(Keyboard::isKeyPressed(Keyboard::S)){if(canMove(playerX-cosf(viewAngle)*spd,playerY))playerX-=cosf(viewAngle)*spd;if(canMove(playerX,playerY-sinf(viewAngle)*spd))playerY-=sinf(viewAngle)*spd;}
        float saL=viewAngle-PI*0.5f,saR=viewAngle+PI*0.5f;
        if(Keyboard::isKeyPressed(Keyboard::A)){if(canMove(playerX+cosf(saL)*spd,playerY))playerX+=cosf(saL)*spd;if(canMove(playerX,playerY+sinf(saL)*spd))playerY+=sinf(saL)*spd;}
        if(Keyboard::isKeyPressed(Keyboard::D)){if(canMove(playerX+cosf(saR)*spd,playerY))playerX+=cosf(saR)*spd;if(canMove(playerX,playerY+sinf(saR)*spd))playerY+=sinf(saR)*spd;}

        // Shooting
        if(Mouse::isButtonPressed(Mouse::Left)&&shootTimer>0.25f){
            shootTimer=0.f;float wallD=getWallDist(playerX,playerY,viewAngle);
            if(targetAlive){
                float dxE=targetX-playerX,dyE=targetY-playerY,eDist=sqrtf(dxE*dxE+dyE*dyE);
                float eAng=atan2f(dyE,dxE)-viewAngle;
                while(eAng>PI)eAng-=2.f*PI;while(eAng<-PI)eAng+=2.f*PI;
                if(fabsf(eAng)<0.06f&&eDist<wallD+0.4f){spawnBurst(targetX,targetY,Color(210,20,210),3);score++;spawnEnemy(targetX,targetY);}
                else{spawnBurst(playerX+cosf(viewAngle)*wallD,playerY+sinf(viewAngle)*wallD,Color(195,170,85),2);}
            }
        }

        // ── Render game ──────────────────────────────────────────
        window.clear(Color(4,4,16));
        {RectangleShape c({(float)WIDTH,HEIGHT/2.f});c.setFillColor(Color(8,8,24));window.draw(c);}
        {RectangleShape f({(float)WIDTH,HEIGHT/2.f});f.setFillColor(Color(14,11,9));f.setPosition(0,HEIGHT/2.f);window.draw(f);}

        // Walls
        wallVerts.clear();
        for(int c=0;c<WIDTH;c++)zBuffer[c]=(float)MAP_SIZE;
        for(int col=0;col<WIDTH;col++){
            float rayA=viewAngle-FOV*0.5f+FOV*col/(float)WIDTH;
            float rDx=cosf(rayA),rDy=sinf(rayA),rX=playerX,rY=playerY;
            int hitT=0;float dist=0.f,wallU=0.f;
            for(float d=0.015f;d<(float)MAP_SIZE;d+=0.015f){
                rX+=rDx*0.015f;rY+=rDy*0.015f;int mx=(int)rX,my=(int)rY;
                if(mx<0||mx>=MAP_SIZE||my<0||my>=MAP_SIZE){hitT=0;break;}
                hitT=worldMap[my][mx];
                if(hitT>0){if(hitT==4){float hLoc=fabsf(rDx)>fabsf(rDy)?rY-my:rX-mx;bool skip=false;for(auto& dr:doors)if(dr.x==mx&&dr.y==my&&hLoc<dr.offset){skip=true;break;}if(skip){hitT=0;continue;}}
                    dist=d*cosf(rayA-viewAngle);wallU=fabsf(rDx)>fabsf(rDy)?rY-floorf(rY):rX-floorf(rX);break;}
            }
            zBuffer[col]=(hitT>0)?dist:(float)MAP_SIZE;if(hitT==0)continue;
            int lH=std::min(HEIGHT,(int)(HEIGHT/(dist+0.001f)));
            int top=std::max(0,HEIGHT/2-lH/2),bot=std::min(HEIGHT,HEIGHT/2+lH/2);
            for(int row=top;row<bot;row+=3){
                int rowEnd=std::min(row+3,bot);float v=(float)(row-(HEIGHT/2-lH/2))/(float)lH;
                v=std::max(0.f,std::min(0.999f,v));Color c=getBrickColor(wallU,v,dist,hitT);
                wallVerts.append(Vertex(Vector2f((float)col,(float)row),c));
                wallVerts.append(Vertex(Vector2f((float)col+1.f,(float)row),c));
                wallVerts.append(Vertex(Vector2f((float)col+1.f,(float)rowEnd),c));
                wallVerts.append(Vertex(Vector2f((float)col,(float)rowEnd),c));
            }
        }
        window.draw(wallVerts);

        // Enemy
        if(targetAlive){
            float dxE=targetX-playerX,dyE=targetY-playerY,eDist=sqrtf(dxE*dxE+dyE*dyE);
            float eAng=atan2f(dyE,dxE)-viewAngle;
            while(eAng>PI)eAng-=2.f*PI;while(eAng<-PI)eAng+=2.f*PI;
            if(fabsf(eAng)<FOV*0.5f){
                float sX=(eAng+FOV*0.5f)/FOV*WIDTH,perpD=eDist*cosf(eAng);
                int sCol=std::max(0,std::min(WIDTH-1,(int)sX));
                if(perpD<zBuffer[sCol]+0.3f){float sz=HEIGHT/(perpD+0.1f);CircleShape dot(sz/3.5f);dot.setOrigin(sz/3.5f,sz/3.5f);
                    dot.setPosition(sX,HEIGHT*0.5f);int pv=155+(int)(95*sinf(gameTime*8.f));dot.setFillColor(Color(pv,45,255,205));window.draw(dot);}
            }
        }
        // Particles
        for(auto& p:particles){float dpx=p.x-playerX,dpy=p.y-playerY,pDist=sqrtf(dpx*dpx+dpy*dpy);
            float pAng=atan2f(dpy,dpx)-viewAngle;while(pAng>PI)pAng-=2.f*PI;while(pAng<-PI)pAng+=2.f*PI;
            if(fabsf(pAng)<FOV*0.5f){float sX=(pAng+FOV*0.5f)/FOV*WIDTH,sz=std::max(1.5f,6.f/pDist);
                RectangleShape pk({sz,sz});pk.setPosition(sX,HEIGHT*0.5f);pk.setFillColor(Color(p.color.r,p.color.g,p.color.b,(sf::Uint8)(p.life*500.f)));window.draw(pk);}
        }
        // Interact prompt
        {int tx=(int)(playerX+cosf(viewAngle)*1.2f),ty=(int)(playerY+sinf(viewAngle)*1.2f);
         if(tx>=0&&tx<MAP_SIZE&&ty>=0&&ty<MAP_SIZE&&worldMap[ty][tx]==4){Text t("[ E ]  OPEN / CLOSE",font,16);t.setOrigin(t.getLocalBounds().width*0.5f,0);t.setPosition(WIDTH*0.5f,HEIGHT-72.f);t.setFillColor(Color::Yellow);window.draw(t);}}
        // Crosshair
        {const float CX=WIDTH*0.5f,CY=HEIGHT*0.5f,HL=9.f;Color cc(235,235,235,185);
         RectangleShape hL({HL*2.f+1.f,2.f});hL.setOrigin(HL,1.f);hL.setPosition(CX,CY);hL.setFillColor(cc);window.draw(hL);
         RectangleShape vL({2.f,HL*2.f+1.f});vL.setOrigin(1.f,HL);vL.setPosition(CX,CY);vL.setFillColor(cc);window.draw(vL);}
        // Gun sprite
        {float bobY=0.f;if(Keyboard::isKeyPressed(Keyboard::W)||Keyboard::isKeyPressed(Keyboard::S)||Keyboard::isKeyPressed(Keyboard::A)||Keyboard::isKeyPressed(Keyboard::D))bobY=sinf(gameTime*10.f)*4.5f;
         float gx=WIDTH*0.47f,gy=HEIGHT*0.63f+bobY;
         RectangleShape barrel({158.f,11.f});barrel.setPosition(gx,gy);barrel.setFillColor(Color(36,38,42));window.draw(barrel);
         RectangleShape slide({122.f,20.f});slide.setPosition(gx+8.f,gy-7.f);slide.setFillColor(Color(50,54,58));window.draw(slide);
         RectangleShape port({26.f,9.f});port.setPosition(gx+52.f,gy-4.f);port.setFillColor(Color(26,26,30));window.draw(port);
         RectangleShape grip({40.f,62.f});grip.setPosition(gx+88.f,gy+13.f);grip.setFillColor(Color(28,24,22));window.draw(grip);
         for(int i=0;i<5;i++){RectangleShape ln({36.f,1.f});ln.setPosition(gx+90.f,gy+18.f+i*8.f);ln.setFillColor(Color(42,38,36));window.draw(ln);}
         RectangleShape tg({30.f,4.f});tg.setPosition(gx+78.f,gy+13.f);tg.setFillColor(Color(40,38,36));window.draw(tg);
         RectangleShape sight({3.f,6.f});sight.setPosition(gx+149.f,gy-6.f);sight.setFillColor(Color(185,185,190));window.draw(sight);
         if(shootTimer<0.055f){float fade=1.f-shootTimer/0.055f;CircleShape flash(12.f);flash.setOrigin(12.f,12.f);flash.setPosition(gx+157.f,gy+5.f);flash.setFillColor(Color(255,195,65,(sf::Uint8)(fade*160.f)));window.draw(flash);}}
        // Minimap
        {RectangleShape bg({MAP_SIZE*MS+4.f,MAP_SIZE*MS+4.f});bg.setPosition(MMAP_X-2.f,MMAP_Y-2.f);bg.setFillColor(Color(0,0,0,115));window.draw(bg);
         for(int my=0;my<MAP_SIZE;my++)for(int mx=0;mx<MAP_SIZE;mx++){int wt=worldMap[my][mx];if(!wt)continue;
             RectangleShape cell({MS-0.5f,MS-0.5f});cell.setPosition(MMAP_X+mx*MS,MMAP_Y+my*MS);
             Color wc=wt==4?Color::Yellow:wt==2?Color(138,68,26):wt==3?Color(26,108,126):Color(76,64,60);cell.setFillColor(wc);window.draw(cell);}
         CircleShape pd(MS*0.9f);pd.setOrigin(MS*0.9f,MS*0.9f);pd.setPosition(MMAP_X+playerX*MS,MMAP_Y+playerY*MS);pd.setFillColor(Color::Green);window.draw(pd);
         RectangleShape dir({MS*2.5f,MS*0.4f});dir.setOrigin(0,MS*0.2f);dir.setPosition(MMAP_X+playerX*MS,MMAP_Y+playerY*MS);dir.setRotation(viewAngle*180.f/PI);dir.setFillColor(Color::Green);window.draw(dir);}
        // Score + ESC hint
        {std::stringstream ss;ss<<"SCORE: "<<(int)score;Text txt(ss.str(),font,21);txt.setPosition(18.f,18.f);txt.setFillColor(Color::Cyan);window.draw(txt);
         Text hint("ESC = Menu",font,13);hint.setPosition(18.f,46.f);hint.setFillColor(Color(100,100,120));window.draw(hint);}

        window.display();
    }
    return 0;
}
