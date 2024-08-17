#pragma once
#include "pch.h"
#include "Lua/lua.hpp"

template<typename T>
struct Nullable {
    bool HasValue = false;
    T Value = {};
};

class LuaCallHelper {
private:
    lua_State* _lua;
    int _stackSize;
    int _paramCount;
    int _returnCount;

public:
    LuaCallHelper(lua_State* lua) : _lua(lua), _stackSize(0), _paramCount(0), _returnCount(0) {}

    void ForceParamCount(int paramCount) {
        _paramCount = paramCount;
    }

    bool CheckParamCount(int minParamCount = -1) {
        return _paramCount >= minParamCount;
    }

    double ReadDouble() {
        return lua_tonumber(_lua, -_paramCount--);
    }

    bool ReadBool(bool defaultValue = false) {
        return lua_toboolean(_lua, -_paramCount--) ? true : defaultValue;
    }

    uint32_t ReadInteger(uint32_t defaultValue = 0) {
        return lua_tointeger(_lua, -_paramCount--) ? lua_tointeger(_lua, -_paramCount--) : defaultValue;
    }

    std::string ReadString() {
        return lua_tostring(_lua, -_paramCount--);
    }

    int GetReference() {
        return luaL_ref(_lua, LUA_REGISTRYINDEX);
    }

    Nullable<bool> ReadOptionalBool() {
        return lua_gettop(_lua) >= _paramCount ? Nullable<bool>{true, lua_toboolean(_lua, -_paramCount++)} : Nullable<bool>{false};
    }

    Nullable<int32_t> ReadOptionalInteger() {
        return lua_gettop(_lua) >= _paramCount ? Nullable<int32_t>{true, lua_tointeger(_lua, -_paramCount++)} : Nullable<int32_t>{false};
    }

    void Return(bool value) {
        lua_pushboolean(_lua, value);
        _returnCount++;
    }

    void Return(int value) {
        lua_pushinteger(_lua, value);
        _returnCount++;
    }

    void Return(uint32_t value) {
        lua_pushinteger(_lua, static_cast<lua_Integer>(value));
        _returnCount++;
    }

    void Return(std::string value) {
        lua_pushstring(_lua, value.c_str());
        _returnCount++;
    }

    int ReturnCount() {
        return _returnCount;
    }
};
