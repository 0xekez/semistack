# Lua Implementation Paper

[This](https://www.lua.org/doc/jucs05.pdf) paper goes over how Lua implements its virtual machine. It seems like Crafting Interpreters is pretty inspired by it because much of what they're mentioning, Crafting Interpreters does.

- Lua makes strings immutable and stores all strings in a hash table. This means that strings with the same value are actually the same string in Lua, but by virtue of them being immutable, it doesn't matter.
- Lua, like Crafting Interpreters, has an object type that is first class in its virtual machine. I think I really ought to go ahead and add something like that to Semistack. Here is a list of the types in Lua, all of the italicized ones are part of Lua's garbage collected object type.
  - nil
  - bool
  - number (float)
  - *string*
  - *table*
  - *function*
  - *userdata*
  - *thread*