#include "cmaptel"

extern "C"{
	unsigned long jnp1::maptel_create(){
		return 0;
	}
}

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
