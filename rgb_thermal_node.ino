#include <ncurses.h>
#include <string>
#include <stdlib.h>
#include <unistd.h> 
#include <iostream>
#include "display.h"
using namespace std;

// Handle attack sequence
int pokeAttack(char player_type, pogemon *player, pogemon *enemy, double difficulity) {
    float opr = 1; // Damage Increasing Factor due to elemental weakness

    string p_atk_name, e_atk_name, spaces;

    switch(player->elem){ // Nerdy --> Athletic --> Creative --> Charismatic --> Nerdy (-->: strong against)
        case 'n':
            if(enemy->elem == 'a') opr = 2.0;
            p_atk_name = "LogicClash";
            break;
        case 'a':
            if(enemy->elem == 'c') opr = 2.0;
            p_atk_name = "PowerStrike";
            break;
        case 'c':
            if(enemy->elem == 'h') opr = 2.0;
            p_atk_name = "DreamForge";
            break;
        case 'h':
            if(enemy->elem == 'n') opr = 2.0;
            p_atk_name =  "CharmWave";
            break;
    }
    switch(enemy->elem){ // Nerdy --> Athletic --> Creative --> Charismatic --> Nerdy (-->: strong against)
        case 'n':
            if(player->elem == 'a') opr = 2.0;
            e_atk_name = "LogicClash";
            break;
        case 'a':
            if(player->elem == 'c') opr = 2.0;
            e_atk_name = "PowerStrike";
            break;
        case 'c':
            if(player->elem == 'h') opr = 2.0;
            e_atk_name = "DreamForge";
            break;
        case 'h':
            if(player->elem == 'n') opr = 2.0;
            e_atk_name = "CharmWave";
            break;
    }

    for(int i=0;i<13-p_atk_name.length();i++) spaces += " ";

    int x=0, y=0, dmg;
    if(player_type == 's'){
        while(1){
            printArena(player, enemy, 0, 0);
            mvprintw(VOFF + 17, HOFF + 0, "| %s TACKLE        %s -            | PP        %.0f/35 |",
                (x == 0 && y == 0) ? ">" : " ", (x == 0 && y == 1) ? ">" : " ", player->pp);
            mvprintw(VOFF + 18, HOFF + 0, "| %s %s%s %s -            | TYPE/NORMAL     |",
                (x == 1 && y == 0) ? ">" : " ", p_atk_name.c_str(), spaces.c_str(), (x == 1 && y == 1) ? ">" : " ");
    
            int ch = getch();
            switch (ch) {
                case 'b': return 0;
                case 'w': case 's': x ^= 1; break;
                case 'a': case 'd': y ^= 1; break;
                case '\n':
                    if(x == 0 && y == 0){ // Tackle
                        dmg = damage(5,15); // Tackle
                        enemy->hp -= dmg;
                        player->pp -= 5;
                        char temp[20] = "";
                        strcat(temp, "Tackle");
                        printAttack(player_type, temp, player, enemy);
                        return 1;
                    }
                    else if(x == 1 && y == 0){ // Elemental Attack
                        dmg = damage(5,15) * (opr);
                        enemy->hp -= dmg;
                        player->pp -= 10;
                        //char temp[20] = "Bull";
                        printAttack(player_type, p_atk_name, player, enemy);
                        return 1;
                    }
                    else{
                        gotoxy(2, 17);
                        printw(" This option isn't available!");
                        gotoxy(2,18);
                        printw("  Press any key to continue");
                        getch();
                        break;
                    }
            }
        }
    }
    else{
        if(enemy->pp < 10 && enemy->pp >= 5){
            dmg = damage(5,15);
            player->hp -= dmg*difficulity;
            char temp[20] = "";
            strcat(temp, "Tackle");
            printAttack('h', temp, player, enemy);
            enemy->pp -= 5;
        }
        else{
            dmg = damage(5,15) * (opr);
            player->hp -= dmg*difficulity;
            printAttack('h', e_atk_name, player, enemy);
            enemy->pp -= 10;
        }
        return 1;
    }

    return 1;
}

int change(vector<pogemon> &pogelist, pogemon* &cur_player, pogemon *enemy){
    // Initialize the variables
    char **namelist = new char*[4];
    for (int i = 0; i < 4; i++) namelist[i] = new char[30](); 

    for(int i=0;i<pogelist.size();i++) strcat(namelist[i], pogelist[i].name);
    if(pogelist.size() < 4) for(int i=pogelist.size();i<4;i++) strcat(namelist[i], "-");
    for(int i=0;i<4;i++) for(int j=0;j<13-strlen(pogelist[i].name);j++) strcat(namelist[i], " ");
    
    int *row = new int(0), *col = new int(0);
    while(1){
        printArena(cur_player, enemy, 0, 0);
        char *pad = new char[7]();
        if((int)pogelist[(*row)*2 + *col].hp == 100) strcpy(pad, "    ");
        else if((int)pogelist[(*row)*2 + *col].hp >= 10) strcpy(pad, "     ");
        else strcpy(pad, "      ");

        mvprintw(VOFF + 17, HOFF + 0, "| %s %s %s %s | HP: %.0f/100%s|",
            (*row == 0 && *col == 0) ? ">" : " ", namelist[0], (*row == 0 && *col == 1) ? ">" : " ", namelist[1], pogelist[(*row)*2 + *col].hp, pad);
        mvprintw(VOFF + 18, HOFF + 0, "| %s %s %s %s| TYPE: %c     ",
            (*row == 1 && *col == 0) ? ">" : " ", namelist[2], (*row == 1 && *col == 1) ? ">" : " ", namelist[3], pogelist[(*row)*2 + *col].elem);

        int ch = getch();
        switch (ch) {
            case 'b': return 0;
            case 'w': case 's': *row ^= 1; break;
            case 'a': case 'd': *col ^= 1; break;
            case '\n':
                if((*row)*2 + *col > pogelist.size() or pogelist[(*row)*2 + *col].hp == 0){
                    gotoxy(2, 17);
                    printw(" This option isn't available!");
                    gotoxy(2,18);
                    printw("  Press any key to continue");
                    getch();
                    break;
                }
                // Change the current player
                cur_player = &pogelist[(*row)*2 + *col];

                // Print the action
                printAttack('c', cur_player->name, cur_player, enemy);

                // Delete the variables
                delete[] pad;
                for (int i = 0; i < 4; i++) delete[] namelist[i];
                delete[] namelist;
                delete row; delete col;
                return 1;
        }
        delete[] pad;
    } 
}

// Main game loop
int poge(pogemon *player, pogemon *enemy, vector<pogemon> &pogelist, double difficulity) {

    // Initial animation
    animate(player, enemy, 'i');
    
    usleep(200000);  // 200ms delay
    
    int x = 0, y = 0, turn = 0, cnt = 0;
    while (1){
        bool turnover = false, printed = false;
        while (!turnover){
            if (turn == 0){
                printArena(player, enemy, 0, 0);
                
                // Menu
                mvprintw(VOFF + 17, HOFF + 0, "|                        ||    %s FIGHT     %s BAG   |",
                         (x == 0 && y == 0) ? ">" : " ", (x == 0 && y == 1) ? ">" : " ");
                mvprintw(VOFF + 18, HOFF + 0, "|                        ||    %s POGEMON   %s RUN  ",
                         (x == 1 && y == 0) ? ">" : " ", (x == 1 && y == 1) ? ">" : " ");
                
                gotoxy(2, 17);
                string message = " What will ";
                message += player->name;
                message += " do?";
                if(!printed){
                    printTo(message.c_str());
                    printed = true;
                }
                else mvprintw(VOFF + 17, HOFF + 2, "%s", message.c_str());

                // Print the current turn
                mvprintw(VOFF + 18, HOFF + 3, "P HP: %.0f E HP: %.0f", player->hp, enemy->hp);
                // Input handling
                int ch = getch();
                switch (ch) {
                    case 'w': case 's': x ^= 1; break;
                    case 'a': case 'd': y ^= 1; break;
                    case '\n':  // Enter key
                        if (x == 0 && y == 0){ // Fight
                            if(pokeAttack('s', player, enemy, difficulity)) turnover = true;
                        }
                        else if(x == 0 && y == 1){ // Bag
                            enemy->hp = 0;
                            turnover = true;
                        }
                        else if(x == 1 && y == 0){ // Pogemon
                            if(change(pogelist, player, enemy)) turnover = true;
                        }
                        else{
                            // Handle other menu options
                            gotoxy(2, 17);
                            printw(" This option isn't available!");
                            gotoxy(2,18);
                            printw("  Press any key to continue");
                            getch();
                        }
                        break;
                }
                cnt++;
            }
            else{
                // Opponent's turn
                pokeAttack('h', player, enemy, difficulity);
                break;
            }
            
        }
        turn ^= 1;
        cnt = 0;
        
        // Check for winner
        if (enemy->hp <= 0){
            char temp[50];
            printArena(player, enemy, 0, 0);
            gotoxy(2, 17);
            snprintf(temp, sizeof(temp), "%s FAINTED!", enemy->name);
            printTo(temp);

            usleep(500000);
            printArena(player, enemy, 0, 0);
            gotoxy(2, 17);
            snprintf(temp, sizeof(temp), "You get %s as a prize!", enemy->name);
            printTo(temp);
            enemy->hp = enemy->max_hp;
            enemy->pp = enemy->max_pp;
            if(pogelist.size() < 4) pogelist.push_back(*enemy); // The player gets the evemy as a prize!
            getch();
            return 1;
        }
        else if (player->hp <= 0){
            char temp[50];
            printArena(player, enemy, 0, 0);
            gotoxy(2, 17);
            snprintf(temp, sizeof(temp), "%s FAINTED!", player->name);
            printTo(temp);
            getch();
            return 0;
        }
    }
}

void poge_init(pogemon *poge, char elem, const char *name, float hp){
    poge->elem = elem;
    poge->hp = hp;
    strcat(poge->name, name);
}

int main() {
    pogemon enemy;
    poge_init(&enemy, 'n', "Ing.mov", 100);
    double difficulity = 5;

    vector<pogemon> pogelist;
    pogemon player1;
    poge_init(&player1, 'n', "Chim", 99);
    pogelist.push_back(player1);
    pogemon player2;
    poge_init(&player2, 'a', "Tat", 98);
    pogelist.push_back(player2);
    pogemon player3;
    poge_init(&player3, 'c', "Wing", 97);
    pogelist.push_back(player3);
    pogemon player4;
    //poge_init(&player4, 'h', "Sucks", 96);
    //pogelist[3] = player4;


    // Initialize NCurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide cursor
    
    // Initialize colors
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);    // Green for full HP
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);   // Yellow for medium HP
    init_pair(3, COLOR_RED, COLOR_BLACK);      // Red for low HP
    init_pair(4, COLOR_WHITE, COLOR_BLACK);    // Default white
    
    // Start the game
    poge(&player1, &enemy, pogelist, difficulity);  // Start with type 1

    //pogemon enemy2;
    //poge_init(&enemy2, 'n', "Fern", 99);
    //poge(&player1, &enemy2, pogelist);
    
    // Clean up NCurses
    endwin();
    return 0;
}
