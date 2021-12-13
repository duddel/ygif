// Based on Array.h, taken from
//
// https://github.com/vinniefalco/LuaBridge
//
// by
//
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#pragma once

#include <LuaBridge/detail/Stack.h>

#include <glm/glm.hpp>

namespace luabridge
{
    template <>
    struct Stack<glm::quat>
    {
        static void push(lua_State *L, glm::quat const &quat)
        {
            lua_createtable(L, static_cast<int>(4), 0);
            for (std::size_t i = 0; i < 4; ++i)
            {
                lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
                Stack<float>::push(L, quat[i]);
                lua_settable(L, -3);
            }
        }

        static glm::quat get(lua_State *L, int index)
        {
            if (!lua_istable(L, index))
            {
                luaL_error(L, "#%d argments must be table", index);
            }

            std::size_t const tableSize = static_cast<std::size_t>(get_length(L, index));

            if (tableSize != 4)
            {
                luaL_error(L, "array size should be %d ", 4);
            }

            glm::quat quat;

            int const absindex = lua_absindex(L, index);
            lua_pushnil(L);
            int arrayIndex = 0;
            while (lua_next(L, absindex) != 0)
            {
                quat[arrayIndex] = Stack<float>::get(L, -1);
                lua_pop(L, 1);
                ++arrayIndex;
            }
            return quat;
        }
    };

} // namespace luabridge
