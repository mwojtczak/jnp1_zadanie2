#include <cstdlib>
#include <cassert>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <climits>
#include "cmaptel"

#ifndef NDEBUG
const bool debug = true;
#else
const bool debug = false;
#endif

/* *
 * dictionary to mapa zmian numerow telefonow w pojedynczym slowniku.
 * Pierwsze pole mapy to numer pierowtny.
 * Drugie pole mapy to numer na jaki zostal zmieniony.
 *
 * dictionary_map to mapa wszystkich slownikow.
 * Pierwsze pole mapy to identyfikator slownika.
 * Drugie pole mapy to slownik.
 *
 */
typedef std::unordered_map<std::string, std::string> dictionary;
typedef std::unordered_map<int, dictionary> dictionary_map;

/**
 * Funkcja do sprawdzania poprawnosc id slownika
 *
 */
static void assert_id(unsigned long id) {
	assert(id >= 0);
	assert(id < ULONG_MAX);
}

/**
 * Funkcja do sprawdzania poprawnosc numeru telefonu
 *
 */
static void assert_tel_number(std::string s_tel_src) {
	assert(s_tel_src.length() <= jnp1::TEL_NUM_MAX_LEN);
	assert(std::all_of(s_tel_src.begin(), s_tel_src.end(), isdigit));
}

/**
 * Tworzy statyczną mapę słowników database i ją zwraca.
 *
 * @return mapa słowników, w której są przechowywane
 */
static dictionary_map& database() {
	static dictionary_map database;
	return database;
}

/**
 * Funkcja pomocnicza dla maptel_transform, ktora szuka kolejnych zmian
 * w konkretnym slowniku.
 *
 * @info
 * Nowy numer zostaje dodany do zbioru tel_num_set. W petli jest robione
 * to samo dla kazdego nowego numeru.
 * Jezeli numer ktory probujemy dodac do slownika juz sie tam znajduje
 * dostajemy cykl.
 * wpp ostatnia znaleziona zmiana jest aktualnym numerem
 *
 * @param dict slownik w ktorym maja byc szukane zmiany
 * @param s_tel_src numer ktorego ostateczna zmiana ma zostac znaleziona
 *
 * @return numer na ktory zostal zmieniony tel_src
 *
 */
static std::string find_tel_dst(dictionary dict, std::string s_tel_src) {
	std::unordered_set<std::string> tel_num_set;
	bool found = false;
	std::string s_tel_dst = s_tel_src;
	while (!found) {
		auto dict_elem = dict.find(s_tel_dst);
		if (dict_elem != dict.end()) {
			std::string dummy = dict_elem->second;
			auto check = tel_num_set.insert(dummy);
			if (check.second) {
				s_tel_dst = dummy;
			} else {
				if (debug)
					std::cerr << "maptel: maptel_transform: cycle detected"
							<< std::endl;
				found = true;
				s_tel_dst = s_tel_src;
			}
		} else
			found = true;
	}
	tel_num_set.clear();
	return s_tel_dst;
}

extern "C" {

	/**
	 * Tworzy nowy słownik i dodaje go do mapy słowników database, nadając mu 
	 * wygenerowany klucz
	 *
	 * @return numer stworzonego słownika
	 */
	unsigned long maptel_create() {
		static unsigned long dict_id = 0;
		if (debug)
			std::cerr << "maptel: maptel_create()" << std::endl;
		dictionary new_dict;
		database().insert( { dict_id, new_dict });
		if (debug)
			std::cerr << "maptel: maptel_create: new map id = " << dict_id
					<< std::endl;
		return dict_id++;
	}

	/**
	 * Usuwa słownik o numerze id z database.
	 *
	 * @param id usuwanego słownika
	 */
	void maptel_delete(unsigned long id) {
		if (debug) {
			std::cerr << "maptel: maptel_delete(" << id << ")" << std::endl;
			assert_id(id)
			assert(database().find(id) != database().end());
		}
		database().erase(id);
		if (debug)
			std::cerr << "maptel: maptel_delete: map " << id << " deleted"
					<< std::endl;
	}

	/**
	 * Wstawia nową zmianę słowa do słownika o numerze id.
	 * Jeśli istniała informacja o zmianie numeru tel_src, to ją nadpisuje.
	 *
	 * @param id numer słownika, do którego wstawiane jest słowo
	 * @param tel_src zmieniany numer
	 * @param tel_dst numer, na który zmieniono tel_src
	 *
	 */
	void maptel_insert(unsigned long id, char const *tel_src, char const *tel_dst) {
		if (debug) {
			assert_id(id);
			std::cerr << "maptel: maptel_insert(" << id << ", " << tel_src
					<< ", " << tel_dst << ")" << std::endl;
			assert(database().find(id) != database().end());
		}
		std::string s_tel_src(tel_src ? tel_src : "");
		std::string s_tel_dst(tel_dst ? tel_dst : "");
		if (debug) {
			assert_tel_number(s_tel_src);
			assert_tel_number(s_tel_dst);
		}
		auto map_elem = database().find(id);
		if (debug)
			assert(map_elem != database().end());
		dictionary & dict = map_elem->second;
		dict[s_tel_src] = s_tel_dst;
		if (debug)
			std::cerr << "maptel: maptel_insert: inserted" << std::endl;
	}

	/**
	 * Usuwanie informacji o zmianie telefonu tel_src w slowniku o numerze id
	 * o ile taka zmiana istnieje, wpp nic sie nie dzieje
	 *
	 * @param id numer slownika, z ktorego usuwane jest slowo
	 * @param tel_src usuwany numer
	 * 
	 */
	void maptel_erase(unsigned long id, char const *tel_src) {
		if (debug)
			std::cerr << "maptel: maptel_erase(" << id << ", " << tel_src << ")"
					<< std::endl;
		std::string s_tel_src(tel_src ? tel_src : "");
		if (debug) {
			assert_tel_number(s_tel_src);
			assert_id(id);
		}
		auto map_elem = database().find(id);
		if (debug)
			assert(map_elem != database().end());
		if (map_elem != database().end()) {
			dictionary & dict = (map_elem->second);
			bool erased = dict.erase(s_tel_src);
			if (debug) {
				if (erased)
					std::cerr << "maptel: maptel_erase: erased" << std::endl;
				else
					std::cerr << "maptel: maptel_erase: nothing to erase"
							<< std::endl;
			}
		}
	}

	/** 
	 * Wypisywanie aktualnego numeru na jaki zostal zmieniony numer tel_src
	 *
	 * @param id numer slownika, w ktorym maja byc szukane zmiany
	 * @param tel_src numer, ktorego zmiana ma byc znaleziona
	 * @param tel_dst numer, na ktorym ma byc zapisane na jaki numer zostal 
	 * zmieniony tel_src
	 * @param len wielokosc przydzielonej pamieci tel_dst
	 *
	 * @info
	 * Korzysta z find_tel_dst
	 * Szuka numeru tel_src w slowniku o numerze id i sprawdza na co zostal
	 * zmieniony funkcja find_tel_dst.
	 *
	 * @result
	 * Na zmienna *tel_dst zostaje przypisany albo ostatni numer w ciagu zmian
	 * albo tel_src jezeli numer nie byl zmieniany lub otrzymalismy cykl
	 *
	 */
	void maptel_transform(unsigned long id, char const *tel_src, char *tel_dst,
			size_t len) {
		if (debug)
			std::cerr << "maptel: maptel_transform(" << id << ", " << tel_src
					<< ", " << std::addressof(tel_dst) << ", " << len << ")"
					<< std::endl;
		std::string s_tel_src(tel_src ? tel_src : "");
		if (debug) {
			assert_tel_number(s_tel_src);
			assert_id(id);
		}
		auto map_elem = database().find(id);
		if (debug)
			assert(map_elem != database().end());
		if (map_elem != database().end()) {
			dictionary dict = (map_elem->second);
			std::string s_tel_dst = find_tel_dst(dict, s_tel_src);
			std::strcpy(tel_dst, s_tel_dst.c_str());
			assert(strlen(tel_dst) < len);
			if (debug)
				std::cerr << "maptel: maptel_transform: " << tel_src << " -> "
						<< tel_dst << ", " << std::endl;
		}
	}
}
