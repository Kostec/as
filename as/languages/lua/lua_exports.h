//
// Created by Anton A. Vasilev on 28.02.2025
//
// Exports for Lua run-time (see core_exports.h)
// 

#ifndef AS_LUA_EXPORTS_H
#define AS_LUA_EXPORTS_H

#ifdef _MSC_VER

#define EXPORT_FUNC(name) __pragma(comment(linker, "/EXPORT:"#name"="#name))

//EXPORT_FUNC(vm_OP_SETGLOBAL)
//EXPORT_FUNC(luaD_precall)
//EXPORT_FUNC(lua_settop)
//EXPORT_FUNC(lua_tointeger)
//EXPORT_FUNC(module_entry_point)
//EXPORT_FUNC(push_global_closure)
//EXPORT_FUNC(vm_OP_CLOSURE)
//EXPORT_FUNC(vm_OP_RETURN)
//EXPORT_FUNC(lua_pushinteger)
//EXPORT_FUNC(luaV_arith)
//EXPORT_FUNC(lua_tonumber)
//EXPORT_FUNC(lua_pushnumber)
//EXPORT_FUNC(vm_OP_TAILCALL)
//EXPORT_FUNC(luaV_gettable)
//EXPORT_FUNC(luaL_checkudata)
//EXPORT_FUNC(luaC_barrierf)

#undef EXPORT_FUNC

#endif // _MSC_VER

#endif // AS_LUA_EXPORTS_H
