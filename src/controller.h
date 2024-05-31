#ifndef CONTROLLER_H
#define CONTROLLER_H

enum class ParseInputRet { None, Break, Redraw };

[[nodiscard]] ParseInputRet parse_input();

#endif  // CONTROLLER_H
