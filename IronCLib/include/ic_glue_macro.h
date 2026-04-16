#ifndef IC_GLUE_MACRO_H
#define IC_GLUE_MACRO_H

#define IC_INNER_GLUE_IMPL(a, b) a##b
#define IC_GLUE(a, b) IC_INNER_GLUE_IMPL(a, b)
#define IC_GLUE3(a, b, c) IC_GLUE(IC_GLUE(a, b), c)

#endif // IC_GLUE_MACRO_H