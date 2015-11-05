#include <stdlib.h>
#include "cmaptel"
#include <string>
#include <unordered_map>

typedef std::unordered_map<std::string, std::string>
        Dictionary;
typedef std::unordered_map<int, Dictionary>
        DictionaryMap;

extern "C"{
	unsigned long jnp1::maptel_create(){
		return 0;
	}
}
/*
extern "C"{
	void jnp1::maptel_delete(unsigned long id){
	}
}

extern "C"{
	void jnp1::maptel_insert(unsigned long id, char const *tel_src, char const *tel_dst){
	}
}

extern "C"{
	void jnp1::maptel_erase(unsigned long id, char const *tel_src){
	}
}

extern "C"{
	void jnp1::maptel_transform(unsigned long id, char const *tel_src, char *tel_dst, size_t len){
	}
}
*/
