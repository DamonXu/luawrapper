/*	
    Copyright (C) <2012>  <huangweilook@21cn.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/	
#ifndef _POPVALUE_H
#define _POPVALUE_H

//��luaջ�е���ջ��Ԫ��
template<typename T>
inline T popvalue(lua_State *L)
{
	return realGet<pointerTraits<T>::PointeeType>::Get(L,-1,Int2Type<pointerTraits<T>::isPointer>());
}


template<>
inline luaObject popvalue(lua_State *L)
{
	int r = luaL_ref(L,LUA_REGISTRYINDEX);
	luaObject obj(L,r);
	return obj;
}

template<>
inline double popvalue(lua_State *L)
{
	double ret = lua_tonumber(L,-1);
	lua_pop(L,1);
	return ret;
}

template<>
inline int popvalue(lua_State *L)
{
	int ret = (int)lua_tonumber(L,-1);
	lua_pop(L,1);
	return ret;
}

template<>
inline std::string popvalue(lua_State *L)
{
	const char *str = lua_tostring(L,-1);
	std::string ret(str);
	lua_pop(L,1);
	return ret;
}

template<>
inline const void *popvalue(lua_State *L)
{
	const void *ret;
	ret = lua_touserdata(L,-1);
	if(((objUserData<void>*)ret)->m_flag == 0x1234AFEC)
		ret = ((objUserData<void>*)ret)->ptr;
	lua_pop(L,1);
	return ret;
}

template<>
inline bool popvalue(lua_State *L)
{
	bool ret = (bool)lua_toboolean(L,-1);
	lua_pop(L,1);
	return ret;
}

template<>
inline __int64 popvalue(lua_State *L)
{
	Integer64 *ret = (Integer64*)lua_touserdata(L,-1);
	return ret->GetValue();
}


template<>
inline luatable popvalue(lua_State *L)
{
	luatable ret;
	int len = lua_objlen(L, -1);
	for( int i = 1; i <= len; ++i)
	{
		lua_rawgeti(L,-1,i);
		
		if(lua_isnil(L,-1))
		{
			ret.push_back(any());
			lua_pop(L,1);
		}
		else if(lua_isuserdata(L,-1))
		{
			const void *r = lua_touserdata(L,-1);
			if(((Integer64*)r)->GetFlag() == 0XFEDC1234)
			{
				ret.push_back(((Integer64*)r)->GetValue());
			}
			else if(((objUserData<void>*)r)->m_flag == 0x1234AFEC)
				ret.push_back((const void*)((objUserData<void>*)r)->ptr);
			else
				ret.push_back(r);
			lua_pop(L,1);
		}		
		else if(lua_isnumber(L,-1))
		{
			ret.push_back(popvalue<int>(L));
		}
		else if(lua_isstring(L,-1))
		{
			ret.push_back(popvalue<std::string>(L));
		}

		else if(lua_isboolean(L,-1))
		{
			ret.push_back(popvalue<bool>(L));
		}
		else if(lua_istable(L,-1))
		{
			//��ȡԪ�����û��ԭ����Ǽ򵥵�table��������object
			if(0 == lua_getmetatable(L,-1))
			{
				ret.push_back(popvalue<luatable>(L));
			}
			else
			{
				lua_pop(L,1);
				ret.push_back(popvalue<luaObject>(L));
			}
		}
		else
		{
			throw std::string("lua���������˲�֧�ֵ�����");
		}
	}

	lua_pop(L,1);
	return ret;
}


#endif