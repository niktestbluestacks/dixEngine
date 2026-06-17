#ifndef DIX_MACROES_HPP
#define DIX_MACROES_HPP

// libs
#include <boost/preprocessor.hpp>

#define DIX_EMPTY()
#define DIX_DEFFER(id) id DIX_EMPTY()
#define DIX_PARENS ()
#define DIX_UNWRAP(...) __VA_ARGS__
#define DIX_REMOVE_PARENS(macro, X) macro X
#define DIX_EVAL(x) x
#define DIX_EXPAND(x) DIX_EVAL(x)
#define DIX_RECURSIVE_REMOVE_PARENS(parented, ...) \
    DIX_REMOVE_PARENS(DIX_UNWRAP, parented)        \
    __VA_OPT__(, DIX_RECURSIVE_REMOVE_PARENS(__VA_ARGS__))
#define DIX_TO_TYPE_NAME(type, name) type name
#define DIX_TO_TYPE(type, name) type
#define DIX_TO_NAME(type, name) name
// #define DIX_RECURSIVE_TO_TYPE(pair, ...) \
//     DIX_REMOVE_PARENS(DIX_TO_TYPE, pair) \
//     __VA_OPT__(, DIX_RECURSIVE_TO_TYPE(__VA_ARGS__))

// #define DIX_RECURSIVE_TO_NAME(pair, ...) \
//     DIX_REMOVE_PARENS(DIX_TO_NAME, pair) \
//     __VA_OPT__(, DIX_RECURSIVE_TO_NAME(__VA_ARGS__))

// #define DIX_RECURSIVE_TO_TYPE_NAME(pair, ...) \
//     DIX_REMOVE_PARENS(DIX_TO_TYPE_NAME, pair) \
//     __VA_OPT__(, DIX_RECURSIVE_TO_TYPE_NAME(__VA_ARGS__))

#define DIX_BOOST_TO_TYPE_NAME(a, b, c, d) \
    BOOST_PP_COMMA_IF(c)                   \
    DIX_TO_TYPE_NAME(BOOST_PP_TUPLE_ELEM(0, d), BOOST_PP_TUPLE_ELEM(1, d))

#define DIX_BOOST_TO_TYPE(a, b, c, d) \
    BOOST_PP_COMMA_IF(c)              \
    DIX_TO_TYPE(BOOST_PP_TUPLE_ELEM(0, d), BOOST_PP_TUPLE_ELEM(1, d))

#define DIX_BOOST_TO_NAME(a, b, c, d) \
    BOOST_PP_COMMA_IF(c)              \
    DIX_TO_NAME(BOOST_PP_TUPLE_ELEM(0, d), BOOST_PP_TUPLE_ELEM(1, d))

#define DIX_RECURSIVE_TO_TYPE(elems)              \
    BOOST_PP_SEQ_FOR_EACH_I(DIX_BOOST_TO_TYPE, _, \
                            BOOST_PP_VARIADIC_TO_SEQ elems)

#define DIX_RECURSIVE_TO_NAME(elems)              \
    BOOST_PP_SEQ_FOR_EACH_I(DIX_BOOST_TO_NAME, _, \
                            BOOST_PP_VARIADIC_TO_SEQ elems)

#define DIX_RECURSIVE_TO_TYPE_NAME(elems)              \
    BOOST_PP_SEQ_FOR_EACH_I(DIX_BOOST_TO_TYPE_NAME, _, \
                            BOOST_PP_VARIADIC_TO_SEQ elems)

#define DIX_ASSERT(condition, message) \
    if (condition) {                   \
        throw message;                 \
    }

#endif  // DIX_MACROES_HPP