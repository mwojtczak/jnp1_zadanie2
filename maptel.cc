#include <cstdlib>
#include <cassert>
#include "cmaptel"
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm> //std::all_of
#include <cctype> //isdigit
#include <cstring> //std::strcpy
#include <climits>
#include <iomanip> // cout << hex

#ifndef NDEBUG
    const bool debug = true;
#else
    const bool debug = false;
#endif

//@TODO
//3. czy w nagłówkach metod musi byc jnp1::?nie wystarczy namespace? patrz: jnp1::TEL_NUM_MAX_LEN
//4. extern "C" tylko na początku, 1 dla wszystkich funkcji



/* 
 * dictionary to mapa zmian numerow telefonow w pojedynczym slowniku.
 * Pierwsze pole mapy to numer pierowtny.
 * Drugie pole mapy to numer na jaki zostal zmieniony.
 *
 * dictionary_map to mapa wszystkich slownikow.
 * Pierwsze pole mapy to identyfikator slownika.
 * Drugie pole mapy to slownik.
 *
 */
typedef std::unordered_map<std::string, std::string>
        dictionary;
typedef std::unordered_map<int, dictionary>
        dictionary_map;

dictionary_map database;

extern "C"{

/*
 * Tworzy nowy słownik i dodaje go do mapy słowników database, nadając mu wygenerowany klucz
 *
 * @return numer stworzonego słownika
 */
	unsigned long maptel_create(){
		static unsigned long dict_id = 0;
		if(debug)
			std::cerr << "maptel: maptel_create()" << std::endl;
		dictionary new_dict;
		database.insert({dict_id, new_dict});
		if(debug)
			std::cerr << "maptel: maptel_create: new map id = " << dict_id << std::endl;
		return dict_id++;
	}

	bool maptel_found_dict(unsigned long id){
		if(database.find(id) == database.end()){
			std::cout << "nie ma takiego słownika\n";
			return false;
		}else{
			std::cout << "słownik znaleziono\n";
			return true;
		}
		return !(database.find(id) == database.end());
	}
}


extern "C"{


//@TODO sprawdzić czy jedyną opcją komunikatu jest potwierdzenie: czy może być notka o braku słownika do usunięcia?
/*
 * Usuwa słownik o numerze id z database
 *
 */
	void maptel_delete(unsigned long id){
		if(debug){
			std::cerr << "maptel: maptel_delete(" << id << ")" << std::endl;
			assert(id >= 0);
			assert (id < ULONG_MAX);
			assert(database.find(id) != database.end());
		}
		database.erase(id);
		if(debug){
			std::cerr << "maptel: maptel_delete: map " << id << " deleted" << std::endl;
		}
	}
}

extern "C"{
/*
 * Wstawia nową zmianę słowa do słownika o numerze id.
 * Jeśli istniała informacja o zmianie numeru tel_src, to ją nadpisuje.
 *
 */
	void maptel_insert(unsigned long id, char const *tel_src, char const *tel_dst){
		if(debug){
			std::cerr << "maptel: maptel_insert(" << id << ", " << tel_src << ", " << tel_dst << ")" << std::endl;
			assert(id >= 0);
			assert (id < ULONG_MAX);
			assert(database.find(id) != database.end());
		}
		std::string s_tel_src(tel_src);
		std::string s_tel_dst(tel_dst);

		if (debug){
			assert(s_tel_src.length() <= jnp1::TEL_NUM_MAX_LEN);
			assert(s_tel_dst.length() <= jnp1::TEL_NUM_MAX_LEN);
			assert(std::all_of(s_tel_src.begin(), s_tel_src.end(), isdigit));
			assert(std::all_of(s_tel_dst.begin(), s_tel_dst.end(), isdigit));
		}

		dictionary_map::const_iterator map_elem = database.find(id);
		if(debug)
			assert(map_elem != database.end());

		dictionary dict = database.at(id);
		database.erase(id);
		dict.insert({s_tel_src, s_tel_dst});
		database.insert({id, dict});

		if(debug)
			std::cerr << "maptel: maptel_insert: inserted" << std::endl;
	}


	bool maptel_found_hist(unsigned long id, char const *tel_src, char const *tel_dst){
		std::string s_tel_src(tel_src);
		std::string s_tel_dst(tel_dst);
		dictionary_map::const_iterator map_elem = database.find(id);
		if(map_elem == database.end()){
			std::cout << "nie ma takiego słownika\n";
			return false;
		}
		dictionary my_dict = map_elem->second;
		dictionary::const_iterator elem = my_dict.find(s_tel_src);
		if(elem == my_dict.end()){
			std::cout << "nie ma takiego numeru \n";
			return false;
		}
		std::string second = elem->second;
		if(second.compare(s_tel_dst) == 0){
			std::cout << "wszystko okay";
			return true;
		}else{
			std::cout << "nie zgadza się historia zmian";
			return false;
		}
		return !(elem == my_dict.end());
	}

	void print_all_dict(unsigned long id){
		dictionary d = (database.find(id))->second;
		for ( auto it = d.begin(); it != d.end(); ++it ){
		    std::cout << " " << it->first << ":" << it->second;
		    std::cout << std::endl;
		}
	}
}


/** Usuwanie informacji o zmianie telefonu tel_src w slowniku o numerze id
 * o ile taka zmiana istnieje, wpp nic sie nie dzieje
 *
 * @info
 * Szuka w bazie danych slownika o numerze id
 * Jezeli taki slownik istnieje usuwa informacje o zmianie numeru tel_src
 *
 * @result
 * Zostaje usunieta ze slownika zmiana numeru tel_src o ile istniala
 */
extern "C"{
	void maptel_erase(unsigned long id, char const *tel_src){

	    if (debug)
	        std::cerr << "maptel: maptel_erase(" << id << ", " << tel_src << ")" << std::endl;

		std::string s_tel_src(tel_src);

		//Sprawdzanie poprawnosci numeru telefonu
		if (debug){
		    assert(s_tel_src.length() <= jnp1::TEL_NUM_MAX_LEN);
		    assert(std::all_of(s_tel_src.begin(), s_tel_src.end(), isdigit));
		}

		//Poszukiwania slownika o numerze id
		auto map_elem = database.find(id);

		//Sprawdzenie czy istnieje slownik o numerze id
		if (debug)
		    assert(map_elem != database.end());

        //Poszukiwania numeru tel_src
		if ( map_elem != database.end() ) {
		    dictionary &dict = (map_elem->second);
		    bool erased = dict.erase(s_tel_src);
		    if (debug) {
		        if (erased)
		          std::cerr << "maptel: maptel_erase: erased" << std::endl;
		      else
		    	  std::cerr << "maptel: maptel_erase: nothing to erase" << std::endl;
		    }
		}
	}
}

/** Wypisywanie aktualnego numeru na jaki zostal zmieniony numer tel_src
 *
 * @info
 * Szuka numeru tel_src w slowniku o numerze id i sprawdza na co zostal
 * zmieniony.
 * Nowy numer zostaje dodany do zbioru tel_num_set. W petli jest robione
 * to samo dla kazdego nowego numeru.
 * Jezeli numer ktory probujemy dodac do slownika juz sie tam znajduje dostajemy
 * cykl.
 * wpp ostatnia znaleziona zmiana jest aktualnym numerem
 *
 * @result
 * Na zmienna *tel_dst zostaje przypisany albo ostatni numer w ciagu zmian
 * albo tel_src jezeli numer nie byl zmieniany lub otrzymalismy cykl
 *
 */
extern "C"{
	void maptel_transform(unsigned long id, char const *tel_src, char *tel_dst, size_t len){

	    if (debug)
	    	std::cerr << "maptel: maptel_transform(" << id << ", " << tel_src << ", " << std::addressof(tel_dst) << ", " << len << ")" << std::endl;

	    std::unordered_set<std::string> tel_num_set;
	    std::string s_tel_src(tel_src);

	    //Sprawdzanie poprawnosci numeru telefonu
		if (debug){
		    assert(s_tel_src.length() <= jnp1::TEL_NUM_MAX_LEN);
		    assert(std::all_of(s_tel_src.begin(), s_tel_src.end(), isdigit));
		}

		//Poszukiwania slownika o numerze id
		auto map_elem = database.find(id);

		//Sprawdzenie czy istnieje slownik o numerze id
		if (debug)
		    assert(map_elem != database.end());

        //Poszukiwania zmian numeru
		if ( map_elem != database.end() ) {
            dictionary dict = (map_elem->second);
            bool found = false;
            std::string s_tel_dst = s_tel_src;

            //Petla szukajaca kolejnych zmian numeru tel_src
            while (!found) {
                //Sprawdzenie czy istnieje zmiana numeru tel_dst
                auto dict_elem = dict.find(s_tel_dst);
                if (dict_elem != dict.end()){
                    //Sprawdzenie czy nie dostalismy zapetlenia
                    std::string dummy = dict_elem->second;
                    auto check = tel_num_set.insert(dummy);
                    if (check.second){
                        //Jezeli nie ma zapetlenia, ustawiamy zmienne
                        // i dochodzi do kolejnego obrotu petli while
                        s_tel_dst = dummy;
                    }
                    else{
                        //Jezeli doszlo do zaptelenia konczymy petle
                        //i ustawiamy tel_dsc na tel_src
                    	std::cerr << "maptel: maptel_transform: cycle detected" << std::endl;
                        found = true;
                        s_tel_dst = s_tel_src;
                    }
                }
                //Jezeli nie ma zmiany aktualnie sprawdzanego numeru
                //to koniec petli, a tel_dst jest aktualnie sprawdzanym numerem
                else
                    found = true;
            }

            std::strcpy(tel_dst, s_tel_dst.c_str());
            //Sprawdzenie czy wielkość pamięci wskazywanej przez tel_dst
            //nie przekracza len
            assert(strlen(tel_dst) < len);
            if (debug)
            	std::cerr << "maptel: maptel_transform: " << tel_src << " -> " << tel_dst << std::endl;
		}

        //Czyszczenie zbioru numerow
		tel_num_set.clear();
	}

}

//	int main(){
//		unsigned long test1 = maptel_create();
//		return 0;
//	}
