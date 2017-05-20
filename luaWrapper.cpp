#include "luawrapper/luaWrapper.h"
#include <cstdlib>

using namespace std;
namespace luacpp
{
	luaWrapper::luaWrapper()
		:lState(NULL)
	{
		init();
	}
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

	static void kennyluainit(lua_State *L)
	{			
		luaL_newmetatable(L, "kenny.lualib");
		lua_pop(L,1);
	}
	void luaWrapper::clear()
	{
		if (lState)
			lua_close(lState);
		lState = NULL;
	}

	void luaWrapper::init()
	{
		clear();

		lState = luaL_newstate();
		luaL_openlibs(lState);
		kennyluainit(lState);
		Integer64::Register2Lua(lState);
	}

	int NewObj(lua_State *L,const void *ptr,const char *classname) 
	{
		size_t nbytes = sizeof(objUserData<void>);
		objUserData<void> *obj = (objUserData<void> *)lua_newuserdata(L, nbytes);
		obj->ptr = const_cast<void*>(ptr);
		obj->m_flag = 0x1234AFEC;
		obj->construct_by_lua = false;
		luaL_getmetatable(L, "kenny.lualib");
		lua_pushstring(L,classname);
		lua_gettable(L,-2);
		lua_setmetatable(L, -3);
		lua_pop(L,1);//pop mt kenny.lualib
		return 1;
	}

	int newI64(lua_State *L)
	{
		int type = lua_type(L,1);
		if(type == LUA_TUSERDATA)
		{
			Integer64 *tmp = (Integer64*)lua_touserdata(L,1);
			int flag = tmp->GetFlag();
			if(static_cast<unsigned int>(flag) == 0XFEDC1234)
			{
				void *buf = lua_newuserdata(L, sizeof(Integer64));
				new(buf)Integer64(tmp->GetValue());
				Integer64::setmetatable(L);
				return 1;
			}
			else
			{
				return luaL_error(L,"invaild argument");
			}
		}
		else if(type == LUA_TNUMBER)
		{
			void *buf = lua_newuserdata(L, sizeof(Integer64));
			new(buf)Integer64((int64_t)lua_tonumber(L,1));
			Integer64::setmetatable(L);
			return 1;				
		}
		return luaL_error(L,"invaild argument");
	}


	static const struct luaL_Reg i64Lib[] = {
		{"new",newI64},
		{NULL,NULL},
	};

	void Integer64::Register2Lua(lua_State *L)
	{
		luaL_getmetatable(L, "kenny.lualib");
		lua_pushstring(L,"int64");
		lua_newtable(L);
	
		lua_pushstring(L, "__add");
		lua_pushcfunction(L, i64Add);
		lua_rawset(L, -3);

		lua_pushstring(L, "__sub");
		lua_pushcfunction(L, i64Sub);
		lua_rawset(L, -3);

		lua_pushstring(L, "__div");
		lua_pushcfunction(L, i64Div);
		lua_rawset(L, -3); 

		lua_pushstring(L, "__mul");
		lua_pushcfunction(L, i64Mul);
		lua_rawset(L, -3); 

		lua_pushstring(L, "__mod");
		lua_pushcfunction(L, i64Mod);
		lua_rawset(L, -3);

		lua_pushstring(L, "__eq");
		lua_pushcfunction(L, i64Eq);
		lua_rawset(L, -3);

		lua_pushstring(L, "__lt");
		lua_pushcfunction(L, i64Lt);
		lua_rawset(L, -3);

		lua_pushstring(L, "__le");
		lua_pushcfunction(L, i64Le);
		lua_rawset(L, -3);

		lua_pushstring(L, "__tostring");
		lua_pushcfunction(L, i64toString);
		lua_rawset(L, -3);

		//just for test
		lua_pushstring(L,"__gc");
		lua_pushcfunction(L, i64Destroy);
		lua_rawset(L, -3);

		lua_rawset(L,1);
		lua_pop(L,1);
	#if (LUA_VERSION_NUM<502)
		luaL_register(L,"i64",i64Lib);
		lua_pop(L,1);
	#else
		lua_getglobal(L,"_G");
		luaL_newlib(L, i64Lib);
		lua_setfield(L, -2, "i64");
		lua_pop(L, 1);
	#endif		
	}

	void get_luatable(luatable &lt,lua_State *L)
	{
		lt = popvalue<luatable>(L);
	}
}
