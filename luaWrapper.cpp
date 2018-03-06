#include "luaWrapper.h"
#include <stdlib.h>
namespace luacpp{

std::map<void*,void*> ptrToUserData;
std::set<void*> userdataSet;;

bool luaWrapper::dofile(const char *filename)
{
    if (luaL_dofile(lState, filename)) {
        const char * error = lua_tostring(lState, -1);
		lua_pop(lState,1);
		printf("%s\n",error);
        return false;
    }
	return true;
}

void luaWrapper::init()
{
	lState = luaL_newstate();
	luaL_openlibs(lState);

	lua_newtable(lState);
	lua_newtable(lState);
	lua_pushstring(lState,"__mode");
	lua_pushstring(lState,"v");
	lua_rawset(lState,-3);
	lua_setmetatable(lState,-2);
	lua_setglobal(lState,"kenny.lualib.userdata_info");

	luaL_newmetatable(lState, "kenny.lualib");
	lua_pop(lState,1);
}

void set_userdata(lua_State *L,void *ptr,int index) {
	printf("set_userdata:%p\n",ptr);
	lua_pushvalue(L,index);
	lua_getglobal(L,"kenny.lualib.userdata_info");
	lua_pushlightuserdata(L,ptr);  //push key
	lua_rotate(L,lua_gettop(L)-3,3);
	lua_rawset(L,-3);
	lua_pop(L,1);
}

void get_userdata(lua_State *L,void *ptr) {
	lua_getglobal(L,"kenny.lualib.userdata_info");
	lua_pushlightuserdata(L,ptr);  //push key
	lua_rawget(L,-2);
	lua_replace(L,-2);
}

int pushObj(lua_State *L,const void *ptr,const char *classname)
{
	objUserData<void> *userdata = (objUserData<void>*)ptrToUserData[(void*)ptr];
	if(!userdata) {
	    size_t nbytes = sizeof(objUserData<void>);
	    userdata = (objUserData<void> *)lua_newuserdata(L, nbytes);
	    userdata->ptr = const_cast<void*>(ptr);	
	    userdata->construct_by_lua = false;
	    ptrToUserData[(void*)ptr] = userdata;
	    userdataSet.insert((void*)userdata);
	    set_userdata(L,(void*)userdata,-1);
	} else {
		get_userdata(L,(void*)userdata);
	} 

    luaL_getmetatable(L, "kenny.lualib");
    lua_pushstring(L,classname);
    lua_gettable(L,-2);
    lua_setmetatable(L, -3);
    lua_pop(L,1);//pop mt kenny.lualib

    return 1;

} 

void get_luatable(luatable &lt,lua_State *L)
{
	lt = popvalue<luatable>(L);
}
}