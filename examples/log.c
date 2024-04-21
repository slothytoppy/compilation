#include "../nom.h"

int main(void) {
  // nom_logs second argument takes fmt style arguments
  // nom_log is a wrapper around nom_gen_log which takes a level, fmt, and a va_list as args
  nom_log(NOM_PANIC, "hello %s", "goodbye");
}
