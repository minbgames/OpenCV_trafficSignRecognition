per:
	rm Perception_exe
	g++ -o Perception_exe perception.cpp `pkg-config opencv --cflags --libs` -std=c++11
	./Perception_exe

per_make:
	g++ -o Perception_exe perception.cpp `pkg-config opencv --cflags --libs` -std=c++11

tra:
	rm Traffic_exe
	g++ -o Traffic_exe traffic.cpp `pkg-config opencv --cflags --libs` -std=c++11
	./Traffic_exe

tra_make:
	g++ -o Traffic_exe traffic.cpp `pkg-config opencv --cflags --libs` -std=c++11

total:
	rm total_exe
	g++ -o total_exe total_traffic.cpp `pkg-config opencv --cflags --libs` -std=c++11
	./total_exe

total_make:
	g++ -o total_exe total_traffic.cpp `pkg-config opencv --cflags --libs` -std=c++11

color_search:
	rm color_search_exe
	g++ -o color_search_exe color_search.cpp `pkg-config opencv --cflags --libs` -std=c++11
	./color_search_exe

color_search_make:
	g++ -o color_search_exe color_search.cpp `pkg-config opencv --cflags --libs` -std=c++11

total2:
	rm total2_exe
	g++ -o total2_exe total_traffic3.cpp `pkg-config opencv --cflags --libs` -std=c++11
	./total2_exe

total2_make:
	g++ -o total2_exe total_traffic3.cpp `pkg-config opencv --cflags --libs` -std=c++11
