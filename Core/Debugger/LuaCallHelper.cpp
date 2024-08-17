#include "pch.h"
#include "LuaCallHelper.h"

LuaCallHelper::LuaCallHelper(lua_State *lua) : _lua(lua), _stackSize(lua_gettop(lua)), _paramCount(0), _returnCount(0) {}

void LuaCallHelper::ForceParamCount(int paramCount) {
    while (_lua_gettop(_lua) < paramCount) {
        lua_pushnil(_lua);
    }
}

bool LuaCallHelper::CheckParamCount(int minParamCount) {
    if (minParamCount >= 0 && _stackSize != _paramCount) {
        string message = string("Invalid parameter count. Expected ") + to_string(_paramCount) + " but got " + to_string(_stackSize);
        luaL_error(_lua, message.c_str());
        return false;
    }
    return true;
}

double LuaCallHelper::ReadDouble() {
    if (!lua_isnumber(_lua, -1)) {
        luaL_error(_lua, "Expected a number but got something else");
    }
    double value = lua_tonumber(_lua, -1);
    lua_pop(_lua, 1);
    _paramCount++;
    return value;
}

bool LuaCallHelper::ReadBool(bool defaultValue) {
    bool value = defaultValue;
    if (lua_isboolean(_lua, -1)) {
        value = lua_toboolean(_lua, -1) != 0;
    } else if (lua_isnumber(_lua, -1)) {
        value = lua_tonumber(_lua, -1) != 0;
    }
    lua_pop(_lua, 1);
    _paramCount++;
    return value;
}

Nullable<bool> LuaCallHelper::ReadOptionalBool() {
    Nullable<bool> result;
    if (lua_isboolean(_lua, -1)) {
        result.HasValue = true;
        result.Value = lua_toboolean(_lua, -1) != 0;
    } else if (lua_isnumber(_lua, -1)) {
        result.HasValue = true;
        result.Value = lua_tonumber(_lua, -1) != 0;
    }
    lua_pop(_lua, 1);
    _paramCount++;
    return result;
}

Nullable<int32_t> LuaCallHelper::ReadOptionalInteger() {
    Nullable<int32_t> result;
    if (lua_isinteger(_lua, -1)) {
        result.HasValue = true;
        result.Value = (int32_t)lua_tointeger(_lua, -1);
    } else if (lua_isnumber(_lua, -1)) {
        result.HasValue = true;
        result.Value = (int32_t)lua_tonumber(_lua, -1);
    }
    lua_pop(_lua, 1);
    _paramCount++;
    return result;
}

uint32_t LuaCallHelper::ReadInteger(uint32_t defaultValue) {
    uint32_t value = defaultValue;
    if (lua_isinteger(_lua, -1)) {
        value = (uint32_t)lua_tointeger(_lua, -1);
    } else if (lua_isnumber(_lua, -1)) {
        value = (uint32_t)lua_tonumber(_lua, -1);
    }
    lua_pop(_lua, 1);
    _paramCount++;
    return value;
}

string LuaCallHelper::ReadString() {
    size_t len;
    string str;
    if (lua_isstring(_lua, -1)) {
        const char* cstr = lua_tolstring(_lua, -1, &len);
        str = string(cstr, len);
    }
    lua_pop(_lua, 1);
    _paramCount++;
    return str;
}

int LuaCallHelper::GetReference() {
    if (lua_isfunction(_lua, -1)) {
        _paramCount++;
        return luaL_ref(_lua, LUA_REGISTRYINDEX);
    } else {
        lua_pop(_lua, 1);
        return LUA_NOREF;
    }
}

void LuaCallHelper::Return(bool value) {
    lua_pushboolean(_lua, value);
    _returnCount++;
}

void LuaCallHelper::Return(int value) {
    lua_pushinteger(_lua, value);
    _returnCount++;
}

void LuaCallHelper::Return(uint32_t value) {
    lua_pushinteger(_lua, value);
    _returnCount++;
}

void LuaCallHelper::Return(string value) {
    lua_pushlstring(_lua, value.c_str(), value.size());
    _returnCount++;
}

int LuaCallHelper::ReturnCount() {
    return _returnCount;
}
