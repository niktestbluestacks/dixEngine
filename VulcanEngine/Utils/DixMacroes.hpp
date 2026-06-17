#ifndef DIX_MACROES_HPP
#define DIX_MACROES_HPP

#define DIX_PARENS ()
#define DIX_UNWRAP(...) __VA_ARGS__
#define DIX_REMOVE_PARENS(macro, X) macro X
#define DIX_RECURSIVE_REMOVE_PARENS(parented, ...) \
    DIX_REMOVE_PARENS(DIX_UNWRAP, parented)        \
    __VA_OPT__(, DIX_RECURSIVE_REMOVE_PARENS(__VA_ARGS__))
#define DIX_TO_TYPE_NAME(type, name) type name
#define DIX_TO_TYPE(type, name) type
#define DIX_TO_NAME(type, name) name
#define DIX_RECURSIVE_TO_TYPE(pair, ...) \
    DIX_REMOVE_PARENS(DIX_TO_TYPE, pair) \
    __VA_OPT__(, DIX_RECURSIVE_TO_TYPE(__VA_ARGS__))

#define DIX_RECURSIVE_TO_NAME(pair, ...) \
    DIX_REMOVE_PARENS(DIX_TO_NAME, pair) \
    __VA_OPT__(, DIX_RECURSIVE_TO_NAME(__VA_ARGS__))

#define DIX_RECURSIVE_TO_TYPE_NAME(pair, ...) \
    DIX_REMOVE_PARENS(DIX_TO_TYPE_NAME, pair) \
    __VA_OPT__(, DIX_RECUTRIVE_TO_TYPE_NAME(__VA_ARGS__))
#endif  // DIX_MACROES_HPP