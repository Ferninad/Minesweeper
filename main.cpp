#include "common.h"
#include "cmath"
#include "OpenSimplexNoise.h"
#include "string"

bool Init();
void CleanUp();
void Run();
void DrawGrid();
string image(int a);
void UpdateHover();
void RenderImage(string src, int posx, int posy, int w, int h);
bool InGrid();
void Generate();
int CheckMines(int indexx, int indexy);
bool IsSlated(int index, int indexy);
void Reveal(int index, int indexy, bool tree);
void GameOver();
void WinCheck();

SDL_Window *window;
SDL_GLContext glContext;
SDL_Surface *gScreenSurface = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Rect pos;
SDL_Texture *img = NULL;

int screenWidth = 500;
int screenHeight = 500;
int gridSize = 40;
int mx, my, pmx, pmy;
int offset;
int hoverIndexx, hoverIndexy, prevIndexx, prevIndexy;
bool reenter = false;
bool leave = false;
bool generate = true;
bool deflagged = false;
bool over = false;
bool win = false;
int numberOfMines = pow(floor(screenWidth/gridSize), 2) * .2;

vector<vector<vector<int>>> grid;
vector<vector<int>> slated;

bool Init()
{
    if (SDL_Init(SDL_INIT_NOPARACHUTE & SDL_INIT_EVERYTHING) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s\n", SDL_GetError());
        return false;
    }
    else
    {
        //Specify OpenGL Version (4.2)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_Log("SDL Initialised");
    }

    //Create Window Instance
    window = SDL_CreateWindow(
        "Game Engine",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        screenWidth,
        screenHeight,   
        SDL_WINDOW_OPENGL);

    //Check that the window was succesfully created
    if (window == NULL)
    {
        //Print error, if null
        printf("Could not create window: %s\n", SDL_GetError());
        return false;
    }
    else{
        gScreenSurface = SDL_GetWindowSurface(window);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_Log("Window Successful Generated");
    }
    //Map OpenGL Context to Window
    glContext = SDL_GL_CreateContext(window);

    return true;
}

int main()
{
    //Error Checking/Initialisation
    if (!Init())
    {
        printf("Failed to Initialize");
        return -1;
    }

    // Clear buffer with black background
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    //Swap Render Buffers
    SDL_GL_SwapWindow(window);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    Run();

    CleanUp();
    return 0;
}

void CleanUp()
{
    //Free up resources
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Run()
{
    bool gameLoop = true;
    srand(time(NULL));
    
    for(int i = 0; i < screenWidth-gridSize; i+=gridSize){
        vector<vector<int>> temp;
        for(int j = 0; j < screenHeight-gridSize; j+=gridSize){
            temp.push_back({0, 0, 0}); //revealed, image, mine
        }
        grid.push_back(temp);
    }

    offset = (screenWidth - grid.size()*gridSize) / 2;
    
    pos.x = 0;
    pos.y = 0;
    pos.w = screenWidth;
    pos.h = screenHeight;
    SDL_SetRenderDrawColor(renderer, 189, 189, 189, 255);
    SDL_RenderFillRect(renderer, &pos);

    SDL_GetMouseState(&mx, &my);
    hoverIndexx = floor((mx-offset)/gridSize);
    hoverIndexy = floor((my-offset)/gridSize);
    prevIndexx = hoverIndexx;
    prevIndexy = hoverIndexy;
    DrawGrid();

    SDL_RenderPresent(renderer);
    
    while (gameLoop)
    {   
        pmx = mx;
        pmy = my;
        SDL_GetMouseState(&mx, &my);

        if(over || win){

        }
        else{
            if(InGrid()){
                leave = true;
                prevIndexx = hoverIndexx;
                prevIndexy = hoverIndexy;
                hoverIndexx = floor((mx-offset)/gridSize);
                hoverIndexy = floor((my-offset)/gridSize);
                if(prevIndexx != hoverIndexx || prevIndexy != hoverIndexy || reenter || deflagged){
                    if(reenter)
                        reenter = false;
                    if(deflagged)
                        deflagged = false;
                    UpdateHover();
                }
            }
            else{
                reenter = true;
                if(grid[hoverIndexx][hoverIndexy][0] == 0 && leave){
                    leave = false;
                    RenderImage("../Minesweeper/unrevealed.png", hoverIndexx*gridSize + offset, hoverIndexy*gridSize + offset, gridSize, gridSize);
                }
            }
        }

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                gameLoop = false;
            }
            if(event.type == SDL_MOUSEBUTTONDOWN){
                if(over || win){

                }
                else{
                    if(event.button.button == SDL_BUTTON_LEFT){
                        if(grid[hoverIndexx][hoverIndexy][0] == 0){
                            if(generate){
                                generate = false;
                                Generate();
                            }
                            if(grid[hoverIndexx][hoverIndexy][0] == 0){
                                if(grid[hoverIndexx][hoverIndexy][2] == 1){
                                    grid[hoverIndexx][hoverIndexy][0] = 1;
                                    RenderImage("../Minesweeper/hitmine.png", hoverIndexx*gridSize + offset, hoverIndexy*gridSize + offset, gridSize, gridSize);
                                    GameOver();
                                    cout << "GameOver" << endl;
                                }
                                if(grid[hoverIndexx][hoverIndexy][2] == 0){
                                    slated.clear();
                                    grid[hoverIndexx][hoverIndexy][0] = 1;
                                    grid[hoverIndexx][hoverIndexy][1] = CheckMines(hoverIndexx, hoverIndexy);
                                    Reveal(hoverIndexx, hoverIndexy, true);
                                    WinCheck();
                                }
                            }
                        }
                    }
                    else if(event.button.button == SDL_BUTTON_RIGHT){
                        if(grid[hoverIndexx][hoverIndexy][0] == 0){
                            grid[hoverIndexx][hoverIndexy][0] = 2;
                            RenderImage("../Minesweeper/flagged.png", hoverIndexx*gridSize + offset, hoverIndexy*gridSize + offset, gridSize, gridSize);
                        }
                        else if(grid[hoverIndexx][hoverIndexy][0] == 2){
                            deflagged = true;
                            grid[hoverIndexx][hoverIndexy][0] = 0;
                            RenderImage("../Minesweeper/unrevealed.png", hoverIndexx*gridSize + offset, hoverIndexy*gridSize + offset, gridSize, gridSize);
                        }
                    }
                }
            }
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym){
                    case SDLK_ESCAPE:
                        gameLoop = false;
                        break;
                    default:
                        break;
                }
            }

            if (event.type == SDL_KEYUP)
            {
                switch (event.key.keysym.sym){
                    default:
                        break;
                }
            }
        }
    }
}

void DrawGrid(){
    for(int x = 0; x < grid.size(); x++){
        for(int y = 0; y < grid[x].size(); y++){
            if(x == hoverIndexx && y == hoverIndexy && InGrid() && grid[x][y][0] == 0)
                RenderImage("../Minesweeper/hover.png", x*gridSize + offset, y*gridSize + offset, gridSize, gridSize);
            else
                RenderImage("../Minesweeper/unrevealed.png", x*gridSize + offset, y*gridSize + offset, gridSize, gridSize);
        }
    }
}

void Reveal(int indexx, int indexy, bool tree){
    RenderImage(image(grid[indexx][indexy][1]), indexx*gridSize + offset, indexy*gridSize + offset, gridSize, gridSize);
    if(grid[indexx][indexy][1] == 0 && tree){
        vector<vector<int>> empty;
        for(int i = -1; i < 2; i++){
            for(int j = -1; j < 2; j++){
                if(indexx+i >= 0 && indexx+i < grid.size() && indexy+j >= 0 && indexy+j < grid[0].size() && !(i == 0 && j == 0)){
                    grid[indexx+i][indexy+j][0] = 1;
                    int mines = CheckMines(indexx+i, indexy+j);
                    grid[indexx+i][indexy+j][1] = mines;
                    if(mines == 0){
                        if(!IsSlated(indexx+i, indexy+j)){
                            slated.push_back({indexx+i, indexy+j});
                            empty.push_back({indexx+i, indexy+j});
                        }
                    }
                    else{
                        Reveal(indexx+i, indexy+j, false);
                    }
                }
            }
        }
        for(int i = 0; i < empty.size(); i++){
            Reveal(empty[i][0], empty[i][1], true);
        }
    }
}

bool IsSlated(int index, int indexy){
    for(int i = 0; i < slated.size(); i++){
        if(slated[i][0] == index && slated[i][1] == indexy)
            return true;
    }
    return false;
}

int CheckMines(int indexx, int indexy){
    int mines = 0;
    for(int i = -1; i < 2; i++){
        for(int j = -1; j < 2; j++){
            if(indexx+i >= 0 && indexx+i < grid.size() && indexy+j >= 0 && indexy+j < grid[0].size() && !(i == 0 && j == 0)){
                if(grid[indexx+i][indexy+j][2] == 1)
                    mines++;
            }
        }
    }

    return mines;
}

string image(int a){
    if(a == 0)
        return "../Minesweeper/revealedEmpty.png";
    else if(a == 1)
        return "../Minesweeper/revealed1.png";
    else if(a == 2)
        return "../Minesweeper/revealed2.png";
    else if(a == 3)
        return "../Minesweeper/revealed3.png";
    else if(a == 4)
        return "../Minesweeper/revealed4.png";
    else if(a == 5)
        return "../Minesweeper/revealed5.png";
    else if(a == 6)
        return "../Minesweeper/revealed6.png";
    else if(a == 7)
        return "../Minesweeper/revealed7.png";
    else if(a == 8)
        return "../Minesweeper/revealed8.png";

    return "../Minesweeper/unrevealed.png";
}

void UpdateHover(){
    if(grid[prevIndexx][prevIndexy][0] == 0){
        RenderImage("../Minesweeper/unrevealed.png", prevIndexx*gridSize + offset, prevIndexy*gridSize + offset, gridSize, gridSize);
    }
    if(grid[hoverIndexx][hoverIndexy][0] == 0){
        RenderImage("../Minesweeper/hover.png", hoverIndexx*gridSize + offset, hoverIndexy*gridSize + offset, gridSize, gridSize);
    }
}

void RenderImage(string src, int posx, int posy, int width, int height){
    img = IMG_LoadTexture(renderer, src.c_str());
    int w, h; //image width and height
    SDL_QueryTexture(img, NULL, NULL, &w, &h);
    pos.x = posx; pos.y = posy; pos.w = width; pos.h = height;
    SDL_RenderCopy(renderer, img, NULL, &pos);
    SDL_RenderPresent(renderer);
}

bool InGrid(){
    return mx >= offset && mx < screenWidth-offset && my >= offset && my < screenHeight-offset;
}

void Generate(){
    for(int i = 0; i < numberOfMines; i++){
        int indexx = rand() % grid.size();
        int indexy = rand() % grid.size();
        if(grid[indexx][indexy][2] != 1 && (indexx != hoverIndexx && indexy != hoverIndexy))
            grid[indexx][indexy][2] = 1;
        else
            i--;
    }
}

void GameOver(){
    over = true;
    for(int i = 0; i < grid.size(); i++){
        for(int j = 0; j < grid[0].size(); j++){
            if(!(i == hoverIndexx && j == hoverIndexy) && grid[i][j][2] == 1)
                RenderImage("../Minesweeper/mine.png", i*gridSize + offset, j*gridSize + offset, gridSize, gridSize);
        }
    }
}

void WinCheck(){
    for(int i = 0; i < grid.size(); i++){
        for(int j = 0; j < grid[0].size(); j++){
            if(grid[i][j][2] == 0 && grid[i][j][0] == 0){
                return;
            }
        }
    }
    win = true;
    cout << "WIN" << endl;
    for(int i = 0; i < grid.size(); i++){
        for(int j = 0; j < grid[0].size(); j++){
            if(grid[i][j][2] == 1)
                RenderImage("../Minesweeper/flagged.png", i*gridSize + offset, j*gridSize + offset, gridSize, gridSize);
        }
    }
}