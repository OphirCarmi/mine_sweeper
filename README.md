# mine_sweeper
mine_sweeper implementation in c

```
sudo apt install libncurses-dev gcc libgtk-3-dev
gcc `pkg-config gtk+-3.0 --cflags` game/game_main.c -o my_game `pkg-config gtk+-3.0 --libs` -lncurses -I.
gcc user/user_main.c -o my_user -I.  
```