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
#ifndef _OBJPUSH_H
#define _OBJPUSH_H
//����lua����ʱ����ѹջ�ĳ���
template<typename T>
class objPush
{
public:
	objPush(lua_State *L,const T arg)
	{
		doPush(L,arg,Int2Type<pointerTraits<T>::isPointer>(),Int2Type<IndexOf<internalType,typename pointerTraits<T>::PointeeType>::value == -1>());
	}
private:
	 //����ָ������ָ��
	 void doPush(lua_State *L,T arg,Int2Type<true>,Int2Type<true>)
	 {
		if(!arg)
			lua_pushnil(L);
		else
		{
			if(!luaRegisterClass<typename pointerTraits<T>::PointeeType>::isRegister())
				lua_pushlightuserdata(L,(void*)arg);
			else
				objUserData<typename pointerTraits<T>::PointeeType>::NewObj(L,arg);
		}
	}

	//����ָ���ڲ����͵�ָ��
	void doPush(lua_State *L,T arg,Int2Type<true>,Int2Type<false>)
	{
		if(!arg)
			lua_pushnil(L);
		{
			typedef LOKI_TYPELIST_2(char,unsigned char) CharType;
			//ָ���ڲ����͵�ָ���п�����char *
			doPushUserData(L,arg,Int2Type<IndexOf<CharType,typename pointerTraits<T>::PointeeType>::value != -1>());
		}
	}

	//����ָ���ڲ�����
	void doPush(lua_State *L,T arg,Int2Type<false>,Int2Type<false>)
	{
		lua_pushnumber(L,(lua_Number)arg);
	}
	
	//���ڴ���ָ��char *��ָ��
	void doPushUserData(lua_State *L,T arg, Int2Type<true>)
	{
		lua_pushstring(L,arg);
	}

	//��������ָ���ڲ����͵�ָ��
	void doPushUserData(lua_State *L,T arg, Int2Type<false>)
	{
		lua_pushlightuserdata(L,(void*)arg);
	}

};

//��int64���ػ���
template<>
class objPush<int64_t>
{
public:
	objPush(lua_State *L,int64_t value)
	{
		pushI64(L,value);
	}
};

//��std::string���ػ�
template<>
class objPush<std::string>
{
public:
	objPush(lua_State *L,const std::string &arg)
	{
		lua_pushstring(L,arg.c_str());
	}
};

//���luaObject���ػ�
template<>
class objPush<luaObject>
{
public:
	objPush(lua_State *L,const luaObject arg)
	{
		lua_rawgeti(L,LUA_REGISTRYINDEX,arg.getIndex());
	}
};


//���luatable���ػ�
template<>
class objPush<luatable>
{
public:
	objPush(lua_State *L,const luatable &arg)
	{
		lua_newtable(L);
		for(int i = 0; i < (int)arg.size(); ++i)
		{	
			int v = any_cast<int>(arg[i]);
			if(arg[0].empty())
				lua_pushnil(L);
			else
				arg[i]._any_pusher->push(L,const_cast<any*>(&arg[i]));
			lua_rawseti(L,-2,i+1);
		}
	}
};

template<typename T>
void push_obj(lua_State *L,const T obj)
{
	objPush<T> _obj(L,obj);
}

template<typename T>
class concrete_any_pusher : public any_pusher
{
public:
	void push(lua_State *L,any *_any)
	{
		lua_pushnumber(L,any_cast<T>(*_any));
	}
};

template<typename T>
class concrete_any_pusher<T*> : public any_pusher
{
public:
	void push(lua_State *L,any *_any)
	{
		//�Ƿ�ע����û�����
		if(_any->luaRegisterClassName == "")
			objPush<void*> obj(L,any_cast<void*>(*_any));
		else
		{
			NewObj(L,any_cast<void*>(*_any),_any->luaRegisterClassName.c_str());
		}
	}
};

template<>
class concrete_any_pusher<bool> : public any_pusher
{
public:
	void push(lua_State *L,any *_any)
	{
		lua_pushboolean(L,any_cast<bool>(*_any));
	}
};

template<>
class concrete_any_pusher<std::string> : public any_pusher
{
public:
	void push(lua_State *L,any *_any)
	{
		lua_pushstring(L,any_cast<std::string>(*_any).c_str());
	}
};

template<>
class concrete_any_pusher<luaObject> : public any_pusher
{
public:
	void push(lua_State *L,any *_any)
	{
		objPush<luaObject> obj(L,any_cast<luaObject>(*_any));
	}
};

template<>
class concrete_any_pusher<luatable> : public any_pusher
{
public:
	void push(lua_State *L,any *_any)
	{
		objPush<luatable> obj(L,any_cast<luatable>(*_any));
	}
};

template<>
class concrete_any_pusher<int64_t> : public any_pusher
{
public:
	void push(lua_State *L,any *_any)
	{
		objPush<int64_t> obj(L,any_cast<int64_t>(*_any));
	}
};

template <typename T>
any_pusher *create_any_pusher()
{
	return new concrete_any_pusher<T>;
}


#endif
