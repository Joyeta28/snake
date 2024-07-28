all:
	 g++ -I src/include -L src/lib -o Task_201 Task_201.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer

   	 g++ -I src/include -L src/lib -o modify modify.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer
	